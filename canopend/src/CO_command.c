/*
 * Socket command interface for CANopenSocket.
 *
 * @file        CO_command.c
 * @author      Janez Paternoster
 * @copyright   2015 - 2020 Janez Paternoster
 *
 * This file is part of CANopenSocket, a Linux implementation of CANopen
 * stack with master functionality. Project home page is
 * <https://github.com/CANopenNode/CANopenSocket>. CANopenSocket is based
 * on CANopenNode: <https://github.com/CANopenNode/CANopenNode>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "CANopen.h"
#include "CO_command.h"
#include "CO_comm_helpers.h"
#include "CO_master.h"
#include "CO_LSS_master.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <endian.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>


/* Maximum size of Object Dictionary variable transmitted via SDO. */
#ifndef CO_COMMAND_SDO_BUFFER_SIZE
#define CO_COMMAND_SDO_BUFFER_SIZE     1000000
#endif

#define STRING_BUFFER_SIZE  (CO_COMMAND_SDO_BUFFER_SIZE * 4 + 100)
#define LISTEN_BACKLOG      50


/* Globals */
char                       *CO_command_socketPath = "/tmp/CO_command_socket";  /* Name of the local domain socket */


/* Variables */
static void*                command_thread(void* arg);
static pthread_t            command_thread_id;
static void                 command_process(int fd, char* command, size_t commandLength);
static int                  fdSocket;
static uint16_t             comm_net = 1;   /* default CAN net number */
static uint8_t              comm_node_default = 0xFF;  /* CANopen Node ID number is undefined at startup. */
static uint16_t             SDOtimeoutTime = 500; /* Timeout time for SDO transfer in milliseconds, if no response */
static uint8_t              blockTransferEnable = 0; /* SDO block transfer enabled? */
static bool_t               tcpMode = false;
static volatile int         endProgram = 0;


/******************************************************************************/
int CO_command_init(void) {
    struct sockaddr_un addr;

    if(CO == NULL || CO->SDOclient[0] == NULL){
        CO_errExit("CO_command_init - Wrong arguments");
    }

    /* Create, bind and listen socket */
    fdSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fdSocket == -1) {
        CO_errExit("CO_command_init - socket failed");
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, CO_command_socketPath, sizeof(addr.sun_path) - 1);

    if(bind(fdSocket, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        fprintf(stderr, "Can't bind Socket to path '%s'\n", CO_command_socketPath);
        CO_errExit("CO_command_init");
    }

    if(listen(fdSocket, LISTEN_BACKLOG) == -1) {
        CO_errExit("CO_command_init - listen failed");
    }

    /* Create thread */
    endProgram = 0;
    if(pthread_create(&command_thread_id, NULL, command_thread, NULL) != 0) {
        CO_errExit("CO_command_init - thread creation failed");
    }

    return 0;
}


/******************************************************************************/
int CO_command_init_tcp(in_port_t port) {
    struct sockaddr_in addr;

    if(CO == NULL || CO->SDOclient[0] == NULL){
        CO_errExit("CO_command_init - Wrong arguments");
    }

    /* Create, bind and listen socket */
    fdSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(fdSocket == -1) {
        CO_errExit("CO_command_init - socket failed");
    }

    const int yes = 1;
    setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    //strncpy(addr.sin_port, CO_command_socketPath, sizeof(addr.sin_port) - 1);

    if(bind(fdSocket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "Can't bind Socket to path '%s'\n", CO_command_socketPath);
        CO_errExit("CO_command_init");
    }

    if(listen(fdSocket, LISTEN_BACKLOG) == -1) {
        CO_errExit("CO_command_init - listen failed");
    }

    /* Create thread */
    endProgram = 0;
    if(pthread_create(&command_thread_id, NULL, command_thread, NULL) != 0) {
        CO_errExit("CO_command_init - thread creation failed");
    }

    tcpMode = true;
    return 0;
}


/******************************************************************************/
int CO_command_clear(void) {

    static struct sockaddr_un addr;
    int fd;

    endProgram = 1;

    /* Establish a client socket connection to finish the command_thread. */
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd == -1) {
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, CO_command_socketPath, sizeof(addr.sun_path) - 1);

    if(connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        return -1;
    }

    close(fd);

    /* Wait for thread to finish. */
    if(pthread_join(command_thread_id, NULL) != 0) {
        return -1;
    }

    close(fdSocket);

    /* Remove socket from filesystem. */
    if(remove(CO_command_socketPath) == -1) {
        return -1;
    }

    return 0;
}

/******************************************************************************/
int CO_command_clear_tcp(in_port_t port) {

    static struct sockaddr_in addr;
    int fd;

    endProgram = 1;

    /* Establish a client socket connection to finish the command_thread. */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    //addr.sin_addr.s_addr = INADDR_ANY;

    if(connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        return -1;
    }

    close(fd);

    /* Wait for thread to finish. */
    if(pthread_join(command_thread_id, NULL) != 0) {
        return -1;
    }

    close(fdSocket);

    return 0;
}


/******************************************************************************/
static void* command_thread(void* arg) {
    int fd;
    ssize_t n;
    char* cmdBuf;
    size_t cmdBufSize = sizeof *cmdBuf * STRING_BUFFER_SIZE;
    cmdBuf = malloc(cmdBufSize);

    /* Almost endless loop */
    while(endProgram == 0) {

        /* wait for new command */
        fd = accept(fdSocket, NULL, NULL);
        if(fd == -1) {
            CO_error(0x15100000L);
        }

        /* Read command and send answer. */
        while((n = read(fd, cmdBuf, cmdBufSize-1)) > 0) {
            *(cmdBuf + n) = 0; /* terminate input string */
            n++;
            command_process(fd, cmdBuf, n);
        }

        if(n == -1){
            CO_error(0x15800000L + errno);
        }

        /* close current communication */
        if(close(fd) == -1) {
            CO_error(0x15900000L);
        }
    }

    free(cmdBuf);
    return NULL;
}


/******************************************************************************/
static void command_process(int fd, char* command, size_t commandLength) {
    int err = 0; /* syntax or other error, true or false */
    int emptyLine = 0;
    char *token;
    int i = 0;
    uint32_t ui[3];
    uint8_t comm_node = 0xFF; /* undefined */

    char* resp;
    size_t respSize = sizeof *resp * STRING_BUFFER_SIZE;
    resp = malloc(respSize);
    int respLen = 0;
    respErrorCode_t respErrorCode = respErrorNone;

    uint32_t sequence = 0;


    /* parse mandatory token '"["<sequence>"]"' */
    if((token = getTok(command, spaceDelim, &err)) == NULL) {
        /* If empty line, respond with empty line. */
        emptyLine = 1;
    }
    if(err == 0) {
        if(token[0] != '[' || token[strlen(token)-1] != ']') {
            err = 1;
            if(token[0] == '#') {
                /* If comment, respond with empty line. */
                emptyLine = 1;
            }
        }
        else {
            token[strlen(token)-1] = '\0';
            sequence = getU32(token+1, 0, 0xFFFFFFFF, &err);
        }
    }


    /* parse optional tokens '[[<net>] <node>]', both numerical. Then follows
     *  mandatory token <command>, which is not numerical. */
    if(err == 0) {
        for(i=0; i<3; i++) {
            if((token = getTok(NULL, spaceDelim, &err)) == NULL) {
                break;
            }
            if(isdigit(token[0]) == 0) {
                break;
            }
            ui[i] = getU32(token, 0, 0xFFFFFFFF, &err);
        }
    }
    if(err == 0) {
        switch(i) {
        case 0: /* only <command> (pointed by token) */
            comm_node = comm_node_default; /* may be undefined */
            break;
        case 1: /* <node> and <command> tokens */
            if(ui[0] < 0 || ui[0] > 127) {
                err = 1;
                respErrorCode = respErrorUnsupportedNode;
            }
            else {
                comm_node = (uint8_t) ui[0];
            }
            break;
        case 2: /* <net>, <node> and <command> tokens */
            if(ui[0] < 1 || ui[0] > 1) {
                err = 1;
                respErrorCode = respErrorUnsupportedNet;
            }
            else if(ui[1] < 0 || ui[1] > 127) {
                err = 1;
                respErrorCode = respErrorUnsupportedNode;
            }
            else {
                comm_net = (uint16_t) ui[0];
                comm_node = (uint8_t) ui[1];
            }
            break;
        case 3: /* <command> token contains digit */
            err = 1;
            break;
        }
    }

    /* Execute command */
    if(err == 0) {

        /* Upload SDO command - 'r[ead] <index> <subindex> <datatype>' */
        if(strcmp(token, "r") == 0 || strcmp(token, "read") == 0) {
            uint16_t idx;
            uint8_t subidx;
            const dataType_t *datatype; /* optional token */
            int errTokDt = 0;
            int errDt = 0;
            uint32_t SDOabortCode = 1;

            uint8_t* dataRx; /* SDO receive buffer */
            size_t dataRxSize = sizeof *dataRx * CO_COMMAND_SDO_BUFFER_SIZE;
            dataRx = malloc(dataRxSize);
            uint32_t dataRxLen;  /* Length of received data */

            token = getTok(NULL, spaceDelim, &err);
            idx = (uint16_t)getU32(token, 0, 0xFFFF, &err);

            token = getTok(NULL, spaceDelim, &err);
            subidx = (uint8_t)getU32(token, 0, 0xFF, &err);

            token = getTok(NULL, spaceDelim, &errTokDt);
            datatype = getDataType(token, &errDt);

            /* Datatype must be correct, if present. */
            if(errTokDt == 0 && errDt != 0) {
                err = 1;
            }

            lastTok(NULL, spaceDelim, &err);

            if(err == 0 && (comm_node < 1 || comm_node > 127)) {
                err = 1;
                if(comm_node == 0xFF) {
                    respErrorCode = respErrorNoDefaultNodeSet;
                } else {
                    respErrorCode = respErrorUnsupportedNode;
                }
            }

            /* Make CANopen SDO transfer */
            if(err == 0) {
                err = sdoClientUpload(
                        CO->SDOclient[0],
                        comm_node,
                        idx,
                        subidx,
                        dataRx,
                        dataRxSize,
                        &dataRxLen,
                        &SDOabortCode,
                        SDOtimeoutTime,
                        blockTransferEnable);

                if(err != 0){
                    respErrorCode = respErrorInternalState;
                }
            }

            /* output result */
            if(err == 0){
                if(SDOabortCode == 0) {
                    respLen = sprintf(resp, "[%d] ", sequence);

                    if(datatype == NULL || (datatype->length != 0 && datatype->length != dataRxLen)) {
                        respLen += dtpHex(resp+respLen, respSize-respLen, (char*)dataRx, dataRxLen);
                    }
                    else {
                        respLen += datatype->dataTypePrint(
                                resp+respLen, respSize-respLen, (char*)dataRx, dataRxLen);
                    }
                }
                else{
                    respLen = sprintf(resp, "[%d] ERROR: 0x%08X", sequence, SDOabortCode);
                }
            }

            free(dataRx);
        }

        /* Download SDO command - w[rite] <index> <subindex> <datatype> <value> */
        else if(strcmp(token, "w") == 0 || strcmp(token, "write") == 0) {
            uint16_t idx;
            uint8_t subidx;
            const dataType_t *datatype;
            uint32_t SDOabortCode = 1;

            uint8_t* dataTx; /* SDO transmit buffer */
            size_t dataTxSize = sizeof *dataTx * CO_COMMAND_SDO_BUFFER_SIZE;
            dataTx = malloc(dataTxSize);
            uint32_t dataTxLen = 0;  /* Length of data to transmit. */

            token = getTok(NULL, spaceDelim, &err);
            idx = (uint16_t)getU32(token, 0, 0xFFFF, &err);

            token = getTok(NULL, spaceDelim, &err);
            subidx = (uint8_t)getU32(token, 0, 0xFF, &err);

            token = getTok(NULL, spaceDelim, &err);
            datatype = getDataType(token, &err);

            if(err == 0) {
                /* take whole string or single token, depending on datatype. Comment may follow. */
                token = getTok(NULL, (datatype->length == 0) ? "\n\r\f" : spaceDelim, &err);
            }

            if(err == 0) {
                dataTxLen = datatype->dataTypeScan((char*)dataTx, dataTxSize, token);

                /* Length must match and must not be zero. */
                if((datatype->length != 0 && datatype->length != dataTxLen) || dataTxLen == 0) {
                    err = 1;
                }
            }

            lastTok(NULL, spaceDelim, &err);

            if(err == 0 && (comm_node < 1 || comm_node > 127)) {
                err = 1;
                if(comm_node == 0xFF) {
                    respErrorCode = respErrorNoDefaultNodeSet;
                } else {
                    respErrorCode = respErrorUnsupportedNode;
                }
            }

            /* Make CANopen SDO transfer */
            if(err == 0) {
                err = sdoClientDownload(
                        CO->SDOclient[0],
                        comm_node,
                        idx,
                        subidx,
                        dataTx,
                        dataTxLen,
                        &SDOabortCode,
                        SDOtimeoutTime,
                        blockTransferEnable);

                if(err != 0){
                    respErrorCode = respErrorInternalState;
                }
            }

            /* output result */
            if(err == 0){
                if(SDOabortCode == 0) {
                    respLen = sprintf(resp, "[%d] OK", sequence);
                }
                else{
                    respLen = sprintf(resp, "[%d] ERROR: 0x%08X", sequence, SDOabortCode);
                }
            }

            free(dataTx);
        }

        /* NMT start node */
        else if(strcmp(token, "start") == 0) {
            lastTok(NULL, spaceDelim, &err);
            if(err == 0 && comm_node > 127) {
                err = 1;
                respErrorCode = respErrorNoDefaultNodeSet;
            }
            if(err == 0) {
                err = CO_sendNMTcommand(CO, CO_NMT_ENTER_OPERATIONAL, comm_node) ? 1:0;
                if(err == 0) respLen = sprintf(resp, "[%d] OK", sequence);
            }
        }

        /* NMT stop node */
        else if(strcmp(token, "stop") == 0) {
            lastTok(NULL, spaceDelim, &err);
            if(err == 0 && comm_node > 127) {
                err = 1;
                respErrorCode = respErrorNoDefaultNodeSet;
            }
            if(err == 0) {
                err = CO_sendNMTcommand(CO, CO_NMT_ENTER_STOPPED, comm_node) ? 1:0;
                if(err == 0) respLen = sprintf(resp, "[%d] OK", sequence);
            }
        }

        /* NMT Set node to pre-operational */
        else if(strcmp(token, "preop") == 0 || strcmp(token, "preoperational") == 0) {
            lastTok(NULL, spaceDelim, &err);
            if(err == 0 && comm_node > 127) {
                err = 1;
                respErrorCode = respErrorNoDefaultNodeSet;
            }
            if(err == 0) {
                err = CO_sendNMTcommand(CO, CO_NMT_ENTER_PRE_OPERATIONAL, comm_node) ? 1:0;
                if(err == 0) respLen = sprintf(resp, "[%d] OK", sequence);
            }
        }

        /* NMT reset (node or communication) */
        else if(strcmp(token, "reset") == 0) {

            token = getTok(NULL, spaceDelim, &err);
            if(err == 0 && comm_node > 127) {
                err = 1;
                respErrorCode = respErrorNoDefaultNodeSet;
            }
            if(err == 0) {
                if(strcmp(token, "node") == 0) {
                    lastTok(NULL, spaceDelim, &err);
                    if(err == 0) {
                        err = CO_sendNMTcommand(CO, CO_NMT_RESET_NODE, comm_node) ? 1:0;
                        if(err == 0) respLen = sprintf(resp, "[%d] OK", sequence);
                    }
                }
                else if(strcmp(token, "comm") == 0 || strcmp(token, "communication") == 0) {
                    lastTok(NULL, spaceDelim, &err);
                    if(err == 0) {
                        err = CO_sendNMTcommand(CO, CO_NMT_RESET_COMMUNICATION, comm_node) ? 1:0;
                        if(err == 0) respLen = sprintf(resp, "[%d] OK", sequence);
                    }
                }

                else {
                    err = 1;
                }
            }
        }

        /* set command - multiple settings */
        else if(strcmp(token, "set") == 0) {

            token = getTok(NULL, spaceDelim, &err);
            if(err == 0) {
                /* sdo_timeout <value> */
                if(strcmp(token, "sdo_timeout") == 0) {
                    uint16_t tmout;

                    token = getTok(NULL, spaceDelim, &err);
                    tmout = (uint16_t)getU32(token, 0, 10000, &err);

                    lastTok(NULL, spaceDelim, &err);

                    /* Write to variable */
                    if(err == 0) {
                        SDOtimeoutTime = tmout;
                        respLen = sprintf(resp, "[%d] OK", sequence);
                    }
                }

                /* sdo_block <value> */
                else if(strcmp(token, "sdo_block") == 0) {
                    uint8_t blk;

                    token = getTok(NULL, spaceDelim, &err);
                    blk = (uint8_t)getU32(token, 0, 1, &err);

                    lastTok(NULL, spaceDelim, &err);

                    /* Write to variable */
                    if(err == 0) {
                        blockTransferEnable = blk;
                        respLen = sprintf(resp, "[%d] OK", sequence);
                    }
                }

                /* node <value> */
                else if(strcmp(token, "node") == 0) {
                    uint16_t node;

                    token = getTok(NULL, spaceDelim, &err);
                    node = (uint16_t)getU32(token, 1, 127, &err);

                    lastTok(NULL, spaceDelim, &err);

                    /* Write to variable */
                    if(err == 0) {
                        comm_node_default = node;
                        respLen = sprintf(resp, "[%d] OK", sequence);
                    }
                }

                /* Unknown command */
                else {
                    err = 1;
                }
            }
        }

        /* LSS command */
        else if (strstr(token, "lss_") != NULL) {
            err = 0;
            respLen = 0;

            /* Switch state global command */
            if(strcmp(token, "lss_switch_glob") == 0) {
                uint8_t select;

                token = getTok(NULL, spaceDelim, &err);
                select = (uint8_t)getU32(token, 0, 1, &err);

                lastTok(NULL, spaceDelim, &err);

                if(err == 0) {
                    if (select == 0) {
                        err = lssSwitchStateDeselect(CO->LSSmaster);
                    }
                    else {
                        err = lssSwitchStateSelect(CO->LSSmaster, true, 0, 0, 0, 0);
                    }
                }
            }

            /* Switch state selective command */
            else if(strcmp(token, "lss_switch_sel") == 0) {
                uint32_t vendorId;
                uint32_t productCode;
                uint32_t revisionNo;
                uint32_t serialNo;

                token = getTok(NULL, spaceDelim, &err);
                vendorId = getU32(token, 0, 0xFFFFFFFF, &err);

                token = getTok(NULL, spaceDelim, &err);
                productCode = getU32(token, 0, 0xFFFFFFFF, &err);

                token = getTok(NULL, spaceDelim, &err);
                revisionNo = getU32(token, 0, 0xFFFFFFFF, &err);

                token = getTok(NULL, spaceDelim, &err);
                serialNo = getU32(token, 0, 0xFFFFFFFF, &err);

                lastTok(NULL, spaceDelim, &err);

                if(err == 0) {
                    err = lssSwitchStateSelect(CO->LSSmaster, false,
                              vendorId, productCode, revisionNo, serialNo);
                }
            }

            /* LSS configure node-ID command */
            else if(strcmp(token, "lss_set_node") == 0) {
                uint8_t nid;

                token = getTok(NULL, spaceDelim, &err);
                nid = (uint8_t)getU32(token, 0, 0xFF, &err);

                lastTok(NULL, spaceDelim, &err);

                if(err == 0) {
                    err = lssConfigureNodeId(CO->LSSmaster, nid);
                    if(err == CO_LSSmaster_OK_ILLEGAL_ARGUMENT){
                        respErrorCode = respErrorLSSnodeIdNotSupported;
                    }
                }
            }

            /* LSS configure bit-rate command */
            else if(strcmp(token, "lss_conf_bitrate") == 0) {
                uint8_t tableIndex;

                /* First parameter is table selector. We only support the CiA
                 * bit timing table from CiA301 ("0") */
                token = getTok(NULL, spaceDelim, &err);
                (void)getU32(token, 0, 0, &err);

                /* the table has entries from 0..9 */
                token = getTok(NULL, spaceDelim, &err);
                tableIndex = (uint8_t)getU32(token, 0, 9, &err);

                lastTok(NULL, spaceDelim, &err);

                if(err == 0) {
                    err = lssConfigureBitTiming(CO->LSSmaster, tableIndex);
                    if(err == CO_LSSmaster_OK_ILLEGAL_ARGUMENT){
                        respErrorCode = respErrorLSSbitRateNotSupported;
                    }
                }
            }

            /* LSS activate new bit-rate command */
            else if(strcmp(token, "lss_activate_bitrate") == 0) {
                uint16_t switchDelay;

                token = getTok(NULL, spaceDelim, &err);
                switchDelay = (uint16_t)getU32(token, 0, 0xFFFF, &err);

                lastTok(NULL, spaceDelim, &err);

                if(err == 0) {
                    err = lssActivateBit(CO->LSSmaster, switchDelay);
                }
            }

            /* LSS store configuration command */
            else if(strcmp(token, "lss_store") == 0) {

                lastTok(NULL, spaceDelim, &err);

                if(err == 0) {
                    err = lssConfigureStore(CO->LSSmaster);
                    if(err == CO_LSSmaster_OK_ILLEGAL_ARGUMENT){
                        respErrorCode = respErrorLSSparameterStoringFailed;
                    }
                }
            }

            /* Inquire LSS address command */
            else if(strcmp(token, "lss_inquire_addr") == 0) {
                uint32_t vendorId;
                uint32_t productCode;
                uint32_t revisionNo;
                uint32_t serialNo;
                const dataType_t *datatype;

                datatype = getDataType("u32", &err);

                lastTok(NULL, spaceDelim, &err);

                if(err == 0) {
                    err = lssInquireLssAddress(CO->LSSmaster,
                              &vendorId, &productCode, &revisionNo, &serialNo);
                }
                if (err == 0) {
                    respLen = sprintf(resp, "[%d] ", sequence);

                    respLen += datatype->dataTypePrint(resp+respLen, respSize-respLen,
                                   (char*)&vendorId, sizeof(vendorId));
                    respLen += sprintf(resp+respLen, " ");
                    respLen += datatype->dataTypePrint(resp+respLen, respSize-respLen,
                                   (char*)&productCode, sizeof(productCode));
                    respLen += sprintf(resp+respLen, " ");
                    respLen += datatype->dataTypePrint(resp+respLen, respSize-respLen,
                                   (char*)&revisionNo, sizeof(revisionNo));
                    respLen += sprintf(resp+respLen, " ");
                    respLen += datatype->dataTypePrint(resp+respLen, respSize-respLen,
                                   (char*)&serialNo, sizeof(serialNo));
                }
            }

            /* LSS inquire node-ID command */
            else if(strcmp(token, "lss_get_node") == 0) {
                uint8_t nid;

                lastTok(NULL, spaceDelim, &err);

                if(err == 0) {
                    err = lssInquireNodeId(CO->LSSmaster, &nid);
                }
                if (err == 0) {
                    respLen = sprintf(resp, "[%d] %d", sequence, nid);
                }
            }
            /* LSS identify fastscan. This is a manufacturer specific command as
             * the one in DSP309 is quite useless */
            else if(strcmp(token, "_lss_fastscan") == 0) {
                uint32_t vendorId = 0;
                uint32_t productCode = 0;
                uint32_t revisionNo = 0;
                uint32_t serialNo = 0;
                uint16_t timeout;
                const dataType_t *datatype;

                token = getTok(NULL, spaceDelim, &err);
                timeout = (uint16_t)getU32(token, 0, 0xFFFF, &err);
                if (timeout==0 || err!=0) {
                    /* if no timeout was given, use 100ms. Should work in most
                     * cases */
                    err = 0;
                    timeout = 100;
                }

                lastTok(NULL, spaceDelim, &err);
                datatype = getDataType("u32", &err);

                if(err == 0) {
                    err = lssIdentifyFastscan(CO->LSSmaster, timeout,
                              0, &vendorId, 0, &productCode, 0, &revisionNo, 0, &serialNo);
                    if (err == CO_LSSmaster_SCAN_FINISHED) {
                        err = 0;
                    }
                }
                if (err == 0) {
                    respLen = sprintf(resp, "[%d] ", sequence);

                    respLen += datatype->dataTypePrint(resp+respLen, respSize-respLen,
                                   (char*)&vendorId, sizeof(vendorId));
                    respLen += sprintf(resp+respLen, " ");
                    respLen += datatype->dataTypePrint(resp+respLen, respSize-respLen,
                                   (char*)&productCode, sizeof(productCode));
                    respLen += sprintf(resp+respLen, " ");
                    respLen += datatype->dataTypePrint(resp+respLen, respSize-respLen,
                                   (char*)&revisionNo, sizeof(revisionNo));
                    respLen += sprintf(resp+respLen, " ");
                    respLen += datatype->dataTypePrint(resp+respLen, respSize-respLen,
                                   (char*)&serialNo, sizeof(serialNo));
                }
            }
            /*  LSS complete node-ID configuration command */
            else if(strcmp(token, "lss_allnodes") == 0) {
                uint32_t vendorId;
                uint32_t productCode;
                uint32_t revisionNo;
                uint32_t serialNo;
                uint8_t scanType[4];
                uint16_t timeout;
                uint8_t nodeStart;
                uint8_t nodeCount;
                uint8_t store;

                token = getTok(NULL, spaceDelim, &err);
                timeout = (uint16_t)getU32(token, 0, 0xFFFF, &err);
                if (timeout==0 || err!=0) {
                    /* if no timeout was given, use 100ms. Should work in most
                     * cases */
                    err = 0;
                    timeout = 100;
                }

                token = getTok(NULL, spaceDelim, &err);
                if (err != 0) {
                    /* CiA specification for this command takes no arguments. Do
                     * full scan. */
                    err = 0;
                    vendorId = 0;
                    productCode = 0;
                    revisionNo = 0;
                    serialNo = 0;
                    /* use start node ID 2. Should work in most cases */
                    nodeStart = 2;
                    /* store node ID in node's NVM */
                    store = 1;
                    memset(scanType, 0, sizeof(scanType));
                } else {

                    nodeStart = (uint8_t)getU32(token, 1, 127, &err);

                    token = getTok(NULL, spaceDelim, &err);
                    store = (uint8_t)getU32(token, 0, 1, &err);

                    token = getTok(NULL, spaceDelim, &err);
                    scanType[0] = (uint8_t)getU32(token, 0, 2, &err);
                    token = getTok(NULL, spaceDelim, &err);
                    vendorId = (uint32_t)getU32(token, 0, 0xFFFFFFFF, &err);

                    token = getTok(NULL, spaceDelim, &err);
                    scanType[1] = (uint8_t)getU32(token, 0, 2, &err);
                    token = getTok(NULL, spaceDelim, &err);
                    productCode = (uint32_t)getU32(token, 0, 0xFFFFFFFF, &err);

                    token = getTok(NULL, spaceDelim, &err);
                    scanType[2] = (uint8_t)getU32(token, 0, 2, &err);
                    token = getTok(NULL, spaceDelim, &err);
                    revisionNo = (uint32_t)getU32(token, 0, 0xFFFFFFFF, &err);

                    token = getTok(NULL, spaceDelim, &err);
                    scanType[3] = (uint8_t)getU32(token, 0, 2, &err);
                    token = getTok(NULL, spaceDelim, &err);
                    serialNo = (uint32_t)getU32(token, 0, 0xFFFFFFFF, &err);
                }
                lastTok(NULL, spaceDelim, &err);

                if(err == 0) {
                    err = lssEnumerateFastscan(CO->LSSmaster, timeout,
                              nodeStart, &nodeCount, store,
                              scanType[0], vendorId,
                              scanType[1], productCode,
                              scanType[2], revisionNo,
                              scanType[3], serialNo);
                }
                if (err == 0) {
                  respLen = sprintf(resp, "[%d] OK, found %d nodes starting at node ID %d. ",
                                sequence, nodeCount, nodeStart);
                  if ((nodeStart + nodeCount - 1) == 127) {
                      respLen += sprintf(resp+respLen, "Assignment process was stopped"
                                     " at ID 127. There still might be unconfigured"
                                     " nodes remaining. Re-run with lower start ID to"
                                     " find them.");
                  }
                }
            }
            else {
                /* unknown LSS command */
                respErrorCode = respErrorReqNotSupported;
                err = 1;
            }

            /* token / LSS default command result. This info is displayed when
             * the processed LSS command did not print a specific output. */
            if (respLen==0 && respErrorCode==0) {
                if(err==CO_LSSmaster_TIMEOUT || err==CO_LSSmaster_SCAN_NOACK){
                    respErrorCode = respErrorTimeOut;
                }
                else if (err == CO_LSSmaster_OK_MANUFACTURER) {
                    respErrorCode = respErrorLSSmanufacturer;
                }
                else if (err < 0) {
                    respErrorCode = respErrorInternalState;
                } else {
                    respLen = sprintf(resp, "[%d] OK", sequence);
                }
            }
        }

        /* Unknown command */
        else {
            respErrorCode = respErrorReqNotSupported;
            err = 1;
        }
    }


    /* Generate error response (or leave empty line response) */
    if(err != 0 && emptyLine == 0) {
        if(respErrorCode == respErrorNone) {
            respErrorCode = respErrorSyntax;
        }
        respLen = sprintf(resp, "[%d] ERROR: %d", sequence, respErrorCode);
    }


    /* Terminate string and send response */
    *(resp + respLen) = '\r';
    respLen++;
    *(resp + respLen) = '\n';
    respLen++;
    if(!tcpMode) {
        *(resp + respLen) = '\0';
        respLen++;
    }

    if(write(fd, resp, respLen) != respLen) {
        CO_error(0x15200000L);
    }

    free(resp);
}

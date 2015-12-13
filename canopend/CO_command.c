/*
 * Socket command interface for CANopenSocket.
 *
 * @file        CO_command.c
 * @author      Janez Paternoster
 * @copyright   2015 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <http://canopennode.sourceforge.net>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * CANopenNode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "CANopen.h"
#include "CO_command.h"
#include "CO_comm_helpers.h"
#include "CO_master.h"
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


#define SDO_BUFFER_SIZE     1000
#define STRING_BUFFER_SIZE  (SDO_BUFFER_SIZE * 4 + 100)
#define LISTEN_BACKLOG      50


/* Globals */
char                       *CO_command_socketPath = "/tmp/CO_command_socket";  /* Name of the local domain socket */


/* Variables */
static void*                command_thread(void* arg);
static pthread_t            command_thread_id;
static void                 command_process(int fd, char* command, size_t commandLength);
static int                  fdSocket;
static uint16_t             comm_net = 1;   /* default CAN net number */
static uint8_t              comm_node = 0;  /* CANopen Node ID number is undefined by default */
static uint16_t             SDOtimeoutTime = 500; /* Timeout time for SDO transfer in milliseconds, if no response */
static uint8_t              blockTransferEnable = 0; /* SDO block transfer enabled? */
static volatile int         endProgram = 0;


/******************************************************************************/
int CO_command_init(void) {
    struct sockaddr_un addr;

    if(CO == NULL || CO->SDOclient == NULL){
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
static void* command_thread(void* arg) {
    int fd;
    ssize_t n;
    char buf[STRING_BUFFER_SIZE];

    /* Almost endless loop */
    while(endProgram == 0) {

        /* wait for new command */
        fd = accept(fdSocket, NULL, NULL);
        if(fd == -1) {
            CO_error(0x15100000L);
        }

        /* Read command and send answer. */
        while((n = read(fd, buf, sizeof(buf)-1)) > 0) {
            buf[n++] = 0; /* terminate input string */
            command_process(fd, buf, n);
        }

        if(n == -1){
            CO_error(0x15800000L);
        }

        /* close current communication */
        if(close(fd) == -1) {
            CO_error(0x15900000L);
        }
    }

    return NULL;
}


/******************************************************************************/
static void command_process(int fd, char* command, size_t commandLength) {
    int err = 0; /* syntax or other error, true or false */
    int emptyLine = 0;
    char *token;
    int i;
    uint32_t ui[3];

    char resp[STRING_BUFFER_SIZE];
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
        /* case 0: only <command> (pointed by token) */
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

            uint8_t dataRx[SDO_BUFFER_SIZE]; /* SDO receive buffer */
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

            if(err == 0 && comm_node == 0) {
                err = 1;
                respErrorCode = respErrorNoDefaultNodeSet;
            }

            /* Make CANopen SDO transfer */
            if(err == 0) {
                err = sdoClientUpload(
                        CO->SDOclient,
                        comm_node,
                        idx,
                        subidx,
                        dataRx,
                        sizeof(dataRx),
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
                        respLen += dtpHex(resp+respLen, sizeof(resp)-respLen, (char*)dataRx, dataRxLen);
                    }
                    else {
                        respLen += datatype->dataTypePrint(
                                resp+respLen, sizeof(resp)-respLen, (char*)dataRx, dataRxLen);
                    }
                }
                else{
                    respLen = sprintf(resp, "[%d] ERROR: 0x%08X", sequence, SDOabortCode);
                }
            }
        }

        /* Download SDO command - w[rite] <index> <subindex> <datatype> <value> */
        else if(strcmp(token, "w") == 0 || strcmp(token, "write") == 0) {
            uint16_t idx;
            uint8_t subidx;
            const dataType_t *datatype;
            uint32_t SDOabortCode = 1;

            uint8_t dataTx[SDO_BUFFER_SIZE]; /* SDO transmit buffer */
            uint32_t dataTxLen = 0;  /* Length of data to transmit. */

            token = getTok(NULL, spaceDelim, &err);
            idx = (uint16_t)getU32(token, 0, 0xFFFF, &err);

            token = getTok(NULL, spaceDelim, &err);
            subidx = (uint8_t)getU32(token, 0, 0xFF, &err);

            token = getTok(NULL, spaceDelim, &err);
            datatype = getDataType(token, &err);

            token = getTok(NULL, "\n\r\f", &err); /* whole string */
            if(err == 0){
                dataTxLen = datatype->dataTypeScan((char*)dataTx, sizeof(dataTx), token);
            }

            lastTok(NULL, spaceDelim, &err);

            /* Length must match and must not be zero. */
            if((datatype->length != 0 && datatype->length != dataTxLen) || dataTxLen == 0) {
                err = 1;
            }

            if(err == 0 && comm_node == 0) {
                err = 1;
                respErrorCode = respErrorNoDefaultNodeSet;
            }

            /* Make CANopen SDO transfer */
            if(err == 0) {
                err = sdoClientDownload(
                        CO->SDOclient,
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
        }

        /* NMT start node */
        else if(strcmp(token, "start") == 0) {
            lastTok(NULL, spaceDelim, &err);
            if(err == 0) {
                err = CO_sendNMTcommand(CO, CO_NMT_ENTER_OPERATIONAL, comm_node) ? 1:0;
                if(err == 0) respLen = sprintf(resp, "[%d] OK", sequence);
            }
        }

        /* NMT stop node */
        else if(strcmp(token, "stop") == 0) {
            lastTok(NULL, spaceDelim, &err);
            if(err == 0) {
                err = CO_sendNMTcommand(CO, CO_NMT_ENTER_STOPPED, comm_node) ? 1:0;
                if(err == 0) respLen = sprintf(resp, "[%d] OK", sequence);
            }
        }

        /* NMT Set node to pre-operational */
        else if(strcmp(token, "preop") == 0 || strcmp(token, "preoperational") == 0) {
            lastTok(NULL, spaceDelim, &err);
            if(err == 0) {
                err = CO_sendNMTcommand(CO, CO_NMT_ENTER_PRE_OPERATIONAL, comm_node) ? 1:0;
                if(err == 0) respLen = sprintf(resp, "[%d] OK", sequence);
            }
        }

        /* NMT reset (node or communication) */
        else if(strcmp(token, "reset") == 0) {

            token = getTok(NULL, spaceDelim, &err);
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

                    tmout = (uint16_t)getU32(token, 0, 10000, &err);

                    lastTok(NULL, spaceDelim, &err);

                    /* Write to variable */
                    if(err == 0) {
                        SDOtimeoutTime = tmout;
                        respLen = sprintf(resp, "[%d] OK", sequence);
                    }
                }

                /* Unknown command */
                else {
                    err = 1;
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
    resp[respLen++] = '\r';
    resp[respLen++] = '\n';
    resp[respLen++] = '\0';

    if(write(fd, resp, respLen) != respLen) {
        CO_error(0x15200000L);
    }
}

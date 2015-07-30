/*
 * Object for CGI function, which acts as SDO client. It can access CANopen
 * Object Dictionary on any device.
 *
 * @file        CgiOD.c
 * @ingroup     CgiOD
 * @version     SVN: \$Id$
 * @author      Janez Paternoster
 * @copyright   2004 - 2013 Janez Paternoster
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


#include "CgiOD.h"
#include "CANopen.h"

#include <string.h> /*  for memcpy */


/*
 * Decodes hex string and returns numerical value.
 *
 * @param str Pointer to string.
 * @param len Length of string.
 * @param val Value (return).
 *
 * @return 0: Operation completed successfully.
 * @return -1: Error in string.
 */
static int32_t hex2dec(char_t *str, int32_t len, uint32_t *val){
    *val = 0;
    int32_t i;
    for(i=0; i<len; i++){
        char_t c = str[i];
        if     (c>='A' && c<='F') c = c - ('A' - 10);
        else if(c>='a' && c<='f') c = c - ('a' - 10);
        else if(c>='0' && c<='9') c = c - '0';
        else return 1;
        *val = *val<<4 | c;
    }
    return 0;
}


/******************************************************************************/
static void CgiCli_signal(uint32_t arg){
    RTX_Wakeup(arg);
}

/******************************************************************************/
static void huge _pascal CgiCliFunction(rpCgiPtr CgiRequest){
    /* Buffers for building the page */
    extern CgiCli_t *CgiCli;
    extern CO_t *CO;
    CO_SDOclient_t *SDO_C = CO->SDOclient;
    uint32_t bufLen = 0;   /* current length of the buffer */
    uint32_t timeouts = 0;
    const uint32_t maxTimeouts = 5;

    /* Prepare function, which will wake this task after CAN SDO response is */
    /* received (inside CAN receive interrupt). */
    SDO_C->pFunctSignal = CgiCli_signal; /* will wake from RTX_Sleep_Time() */
    SDO_C->functArg = RTX_Get_TaskID();  /* id of this task */

    /*  Construct HTML page */
    if(strlen(CgiRequest->fArgumentBufferPtr)){
        char_t *buf = CgiCli->buf;   /* buffer for CGI message */
        char_t far *name = NULL;
        char_t far *val = NULL;
        while(CGI_GetArgument(&name, &val, CgiRequest) == 0){
            /* decode and verify name */
            char_t rw = name[0];
            uint32_t nodeId, idx, sidx, len;
            char_t err = 0;
            uint8_t *data = CgiCli->SDOBuf; /* data sent or received on CANopen */
            uint32_t dataLen = 0;

            if(!name) err++;
            len = strlen(name);
            if (rw == 'r' || rw == 'R'){
                rw = 'r';
                if(len < 9 || len > 17) err++;
            }
            else if(rw == 'w' || rw == 'W'){
                rw = 'w';
                if(len < 10 || len > 17) err++;
            }
            else{
                err++;
            }
            err += hex2dec(&name[1],  2, &nodeId);
            if(nodeId > 127) err++;
            err += hex2dec(&name[3],  4, &idx);
            err += hex2dec(&name[7],  2, &sidx);
            if(rw == 'w'){
                err += hex2dec(&name[9], len-9, &len);

                /* decode value */
                int32_t i;
                int32_t valLen;
                if(val) valLen = strlen(val);
                else    valLen = 0;
                for(i=0; i<valLen; i=i+2){
                    uint32_t dc;
                    err += hex2dec(&val[i], 2, &dc);
                    data[dataLen] = dc;
                    if(++dataLen >= CgiCli->SDOBufSize) break;
                }

                /* verify value length */
                if(dataLen != len || dataLen == 0) err++;
            }

            /* SDO clinet access */
            uint32_t SDOabortCode = 0;
            if(err){
                buf += sprintf(buf, "Error in argument: %s=%s\n", name, val);
            }
            /* read object dictionary */
            else if(rw == 'r'){
                CO_SDOclient_return_t ret;
                CO_SDOclient_setup(SDO_C, 0, 0, nodeId);
                CO_SDOclientUploadInitiate(SDO_C, idx, sidx, data, CgiCli->SDOBufSize, 1);
                do{
                    uint16_t dt = 50;
                    ret = CO_SDOclientUpload(SDO_C, dt, 500, &dataLen, &SDOabortCode);
                    RTX_Sleep_Time(dt);
                }while(ret > 0);
                CO_SDOclientClose(SDO_C);

                if(ret == CO_SDOcli_endedWithTimeout) {
                    timeouts++;
                }
                else {
                    timeouts = 0;
                }

                if(SDOabortCode){
                    buf += sprintf(buf, "R %02X%04X%02X%X AB: %08X\n",
                                    (unsigned int)nodeId,
                                    (unsigned int)idx,
                                    (unsigned int)sidx,
                                    (unsigned int)dataLen,
                                    (unsigned int)SDOabortCode);
                }
                else{
                    uint32_t i;
                    buf += sprintf(buf, "R %02X%04X%02X%X OK:",
                                    (unsigned int)nodeId,
                                    (unsigned int)idx,
                                    (unsigned int)sidx,
                                    (unsigned int)dataLen);
                    for(i=0; i<dataLen; i++) buf += sprintf(buf, " %02X", data[i]);
                    buf += sprintf(buf, "\n");
                }
            }
            /* write into object dictionary */
            else if(rw == 'w'){
                CO_SDOclient_return_t ret;
                CO_SDOclient_setup(SDO_C, 0, 0, nodeId);
                CO_SDOclientDownloadInitiate(SDO_C, idx, sidx, data, dataLen, 1);
                do{
                    uint16_t dt = 2;
                    ret = CO_SDOclientDownload(SDO_C, dt, 500, &SDOabortCode);
                    if(ret == CO_SDOcli_waitingServerResponse) {
                        dt = 50;
                    }
                    RTX_Sleep_Time(dt);
                }while(ret > 0);
                CO_SDOclientClose(SDO_C);

                if(ret == CO_SDOcli_endedWithTimeout) {
                    timeouts++;
                }
                else {
                    timeouts = 0;
                }

                if(SDOabortCode){
                    buf += sprintf(buf, "W %02X%04X%02X%X AB: %08X\n",
                                    (unsigned int)nodeId,
                                    (unsigned int)idx,
                                    (unsigned int)sidx,
                                    (unsigned int)dataLen,
                                    (unsigned int)SDOabortCode);
                }
                else{
                    uint32_t i;
                    buf += sprintf(buf, "W %02X%04X%02X%X OK:",
                                    (unsigned int)nodeId,
                                    (unsigned int)idx,
                                    (unsigned int)sidx,
                                    (unsigned int)dataLen);
                    for(i=0; i<dataLen; i++) buf += sprintf(buf, " %02X", data[i]);
                    buf += sprintf(buf, "\n");
                }
            }

            /* calculate buffer length, if not enough space for next object, break */
            bufLen = (uint32_t)(buf - CgiCli->buf);
            if((bufLen+1000) > CgiCli->bufSize){
                break;
            }

            /* if no response from remote node for multiple times, break the loop */
            if(timeouts > maxTimeouts){
                break;
            }
        }
    }
    else{
        sprintf(CgiCli->buf,
            "CGI function provides access to object dictionary on any device on the CANopen network.\n\n"
            "Usage with POST method:\n"
            "  odcli?wnniiiissll=xxxx[&rnniiiissll=]\n"
            "  w    - 'w'rite or 'r'ead.\n"
            "  nn   - node ID in hex format.\n"
            "  iiii - Object dictionary index in hex format.\n"
            "  ss   - Object dictionary subindex in hex format.\n"
            "  ll   - length of variable (1 to FFFFFFFF) in hex format. If reading, this value is ignored.\n"
            "  xxxx - Value to be written in hex and little endian format. If reading, this value is ignored.\n");
        bufLen = strlen(CgiCli->buf);
    }

    /* disable SDO client */
    CO_SDOclient_setup(SDO_C, 0, 0, 0);

    /*  Give page to the web server */
    CgiRequest->fHttpResponse = CgiHttpOk;
    CgiRequest->fDataType = CGIDataTypeText;
    CgiRequest->fResponseBufferPtr = CgiCli->buf;
    CgiRequest->fResponseBufferLength = bufLen;
}


/******************************************************************************/
int16_t CgiCli_init_1(
        CgiCli_t               *CgiCli,
        char_t                 *buf,
        uint32_t                bufSize,
        uint8_t                *SDOBuf,
        uint32_t                SDOBufSize)
{
    if(CgiCli == NULL || buf == NULL || SDOBuf == NULL) return CO_ERROR_ILLEGAL_ARGUMENT;

    /* setup variables */
    CgiCli->buf = buf;
    CgiCli->bufSize = bufSize;
    CgiCli->SDOBuf = SDOBuf;
    CgiCli->SDOBufSize = SDOBufSize;

    /* Install CGI function */
    CgiCli->cgiEntry.PathPtr = "odcli";          /*  Name of the page */
    CgiCli->cgiEntry.method = CgiHttpPost;       /*  HTTP method */
    CgiCli->cgiEntry.CgiFuncPtr = CgiCliFunction;/*  Function called on browser request */

    if(CGI_Install(&CgiCli->cgiEntry) != 0)
        return CO_ERROR_PARAMETERS;

    return CO_ERROR_NO;
}


/******************************************************************************/
void CgiCli_delete(CgiCli_t *CgiCli){
    CGI_Delete(CgiCli->cgiEntry.PathPtr);
}

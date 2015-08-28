/*
 * Object for CGI function, which logs CAN messages as two separate logs. One
 * for Emergency messages (saving to file) and one temporary all messages log.
 *
 * @file        CgiLog.c
 * @author      Janez Paternoster
 * @copyright   2012 - 2015 Janez Paternoster
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


#include "CgiLog.h"
#include "CANopen.h"

#include <string.h> /*  for memcpy */
#include <stdio.h>


/******************************************************************************/
static void CgiLogTimestamp(uint8_t *buf, uint8_t disi){
    uint32_t tickCount;
    TimeDateFineS timeDate;

    if(disi){
        CO_DISABLE_INTERRUPTS();
        tickCount = RTX_GetTickCount();
        RTX_Get_TimeDate_us(&timeDate);
        CO_ENABLE_INTERRUPTS();
    }
    else{
        tickCount = RTX_GetTickCount();
        RTX_Get_TimeDate_us(&timeDate);
    }

    CO_memcpySwap4(buf, (uint8_t*) &tickCount); buf += 4;
    *buf++ = 16;
    *buf++ = 0;
    *buf++ = 0;
    *buf++ = 0;
    *buf++ = timeDate.yr;
    *buf++ = timeDate.mn;
    *buf++ = timeDate.dy;
    *buf++ = timeDate.hr;
    *buf++ = timeDate.min;
    *buf++ = timeDate.sec;
    CO_memcpySwap2(buf, (uint8_t*) &timeDate.msec);
}


/******************************************************************************/
static void huge _pascal CgiLogCANFunction(rpCgiPtr CgiRequest){
    extern CgiLog_t *CgiLog;

    int32_t clearOnly = 0;

    /* decode optional arguments */
    if(strlen(CgiRequest->fArgumentBufferPtr)){
        char_t far *name = NULL;
        char_t far *val = NULL;
        if(CGI_GetArgument(&name, &val, CgiRequest) == 0){
            if(strcasecmp(name, "clear")==0){
                clearOnly = 1;
            }
            if(strcasecmp(name, "dump")==0){
                CgiLogSaveBuffer(CgiLog);
                CgiRequest->fHttpResponse = CgiHttpOKNoDoc;
                return;
            }
        }
    }

    /* find buffer */
    uint8_t bufIdx = CgiLog->CANBufIdx;
    uint8_t *buf = CgiLog->CANBuf[bufIdx];
    uint32_t bufLen;

    CO_DISABLE_INTERRUPTS();
    /* calculate size and write pointer of the oldest message */
    if(CgiLog->CANBufOvf){
        bufLen = CgiLog->CANBufSize;
        CO_memcpySwap4(buf+4, (uint8_t*) &CgiLog->CANBufOfs);
    }
    else{
        bufLen = CgiLog->CANBufOfs;
    }
    /* swap buffers */
    CgiLog->CANBufIdx = 1 - bufIdx;
    CgiLog->CANBufOvf = 0;
    CgiLog->CANBufOfs = 16;
    CO_ENABLE_INTERRUPTS();

    /* write timestamp in the new buffer */
    CgiLogTimestamp(CgiLog->CANBuf[CgiLog->CANBufIdx], 1);

    /* Give page to the web server */
    if(clearOnly){
        CgiRequest->fHttpResponse = CgiHttpOKNoDoc;
    }
    else{
        CgiRequest->fHttpResponse = CgiHttpOk;
        CgiRequest->fDataType = CGIDataTypeOctet;
        CgiRequest->fResponseBufferPtr = (char_t*)buf;
        CgiRequest->fResponseBufferLength = bufLen;
    }
}


/******************************************************************************/
static void huge _pascal CgiLogEmcyFunction(rpCgiPtr CgiRequest){
    extern CgiLog_t *CgiLog;

    /* decode optional arguments */
    if(strlen(CgiRequest->fArgumentBufferPtr)){
        char_t far *name = NULL;
        char_t far *val = NULL;
        if(CGI_GetArgument(&name, &val, CgiRequest) == 0){
            if(strcasecmp(name, "flush")==0){
                /* move emergencies from SRAM to file */
                if(*CgiLog->emcyBufOfs > 4){
                    CgiLog->emcyTempBufHold = 40; /* Suppress <CgiLogEmcyProcess> function for 2 seconds */
                    CgiLogEmcySave(CgiLog);
                    CgiRequest->fHttpResponse =  CgiHttpOKNoDoc;
                    return;
                }
            }
        }
    }

    /* Give page to the web server - last messages */
    CgiRequest->fHttpResponse = CgiHttpOk;
    CgiRequest->fDataType = CGIDataTypeText;
    CgiRequest->fResponseBufferPtr = CgiLog->emcyBuf + 4;
    CgiRequest->fResponseBufferLength = *CgiLog->emcyBufOfs - 4;
}


/******************************************************************************/
int16_t CgiLog_init_1(
        CgiLog_t               *CgiLog,
        uint8_t                *emcyBuf,
        uint32_t                emcyBufSize,
        uint8_t                *CANBuf0,
        uint8_t                *CANBuf1,
        uint32_t                CANBufSize,
        uint32_t                maxDumpFiles)
{
    if(CgiLog == NULL || emcyBuf == NULL ||
       CANBuf0 == NULL || CANBuf0 == NULL || CANBufSize < 0x10000)
    {
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }


    /* setup variables */
    CgiLog->emcyBuf          = (char_t*) emcyBuf;
    CgiLog->emcyBufSize      = emcyBufSize;
    CgiLog->CANBuf[0]        = CANBuf0;
    CgiLog->CANBuf[1]        = CANBuf1;
    CgiLog->CANBufSize       = CANBufSize;
    CgiLog->maxDumpFiles     = maxDumpFiles;
    CgiLog->emcyBufOfs       = (uint32_t*) emcyBuf;/* offset is stored in SRAM */
    CgiLog->emcyTempBufStart = 0;
    CgiLog->emcyTempBufStop  = 0;
    CgiLog->emcyTempBufHold  = 0;
    CgiLog->emcyLastSavedDay = 0;
    CgiLog->CANBufIdx        = 0;
    CgiLog->CANBufOvf        = 0;
    CgiLog->CANBufOfs        = 16;

    CgiLogTimestamp(CgiLog->CANBuf[0], 1);

    /* if error in SRAM make new buffer */
    if(*CgiLog->emcyBufOfs < 4 || *CgiLog->emcyBufOfs >= CgiLog->emcyBufSize)
        *CgiLog->emcyBufOfs = 4;

    /* emergency buffer will be saved in CgiLogEmcyProcess automatically */

    /* Install CGI functions */
    CgiLog->cgiEntryEmcy.PathPtr    = "emcy";            /*  Name of the page */
    CgiLog->cgiEntryEmcy.method     = CgiHttpGet;        /*  HTTP method */
    CgiLog->cgiEntryEmcy.CgiFuncPtr = CgiLogEmcyFunction;/*  Function called on browser request */
    if(CGI_Install(&CgiLog->cgiEntryEmcy) != 0)
        return CO_ERROR_PARAMETERS;

    CgiLog->cgiEntryCAN.PathPtr     = "CANlog";          /*  Name of the page */
    CgiLog->cgiEntryCAN.method      = CgiHttpGet;        /*  HTTP method */
    CgiLog->cgiEntryCAN.CgiFuncPtr  = CgiLogCANFunction; /*  Function called on browser request */
    if(CGI_Install(&CgiLog->cgiEntryCAN) != 0)
        return CO_ERROR_PARAMETERS;

    return CO_ERROR_NO;
}


/******************************************************************************/
void CgiLog_delete(CgiLog_t *CgiLog){
    CGI_Delete(CgiLog->cgiEntryEmcy.PathPtr);
    CGI_Delete(CgiLog->cgiEntryCAN.PathPtr);
}


/******************************************************************************/
void CO_logMessage(const CanMsg *msg){
    extern CgiLog_t *CgiLog;
make it reentrant or use mutex
    /* variables */
    uint32_t   timeStamp = RTX_GetTickCount();
    uint8_t    len       = msg->len;
    uint32_t   id_len    = (uint32_t)len<<24 | (uint16_t) canDecodeId(msg->id);
    uint8_t    isEmcy    = 0;
    uint8_t    isOvfPrev = 0;

    /* is emergency message? */
    if((id_len & 0xFF80) == 0x0080 && (id_len & 0x7F) != 0 && len == 8) isEmcy = 1;

    /* find locations in buffers */
    uint32_t   CANbufOffset;   /* offset in CAN buffer */
    uint32_t*  CANbuf;         /* location in CAN buffer for new message */
    uint16_t   emcyBufOffset=0;/* offset in emergency temporary buffer */
    uint32_t*  emcyBuf;        /* location in emergency temporary buffer for new message */

    CANbufOffset = CgiLog->CANBufOfs;
    if((CANbufOffset+32) > CgiLog->CANBufSize){
        CgiLog->CANBufOfs = 16;
        isOvfPrev = CgiLog->CANBufOvf;
        CgiLog->CANBufOvf = 1;
    }
    else{
        CgiLog->CANBufOfs = CANbufOffset+16;
    }
    if(isEmcy){
        emcyBufOffset = CgiLog->emcyTempBufStop;
        CgiLog->emcyTempBufStop = emcyBufOffset + 16;
    }

    /* update timestamp. (Timestamp on the buffer must be older than the oldest message) */
    if(isOvfPrev) memcpy(CgiLog->CANBuf[CgiLog->CANBufIdx], CgiLog->CANBufTimestamp, 16);
    if(CgiLog->CANBufOfs == 16) CgiLogTimestamp(CgiLog->CANBufTimestamp, 0);

    /* copy contents */
    CANbuf = (uint32_t*)(CgiLog->CANBuf[CgiLog->CANBufIdx] + CANbufOffset);
    if(isEmcy){
        /* emergency + CAN buffer */
        emcyBuf = (uint32_t*)(CgiLog->emcyTempBuf + emcyBufOffset);

        CO_memcpySwap4((uint8_t*) CANbuf++, (uint8_t*) &timeStamp);
        *emcyBuf++ = timeStamp;
        CO_memcpySwap4((uint8_t*) CANbuf++, (uint8_t*) &id_len);
        *emcyBuf++ = id_len;
        *CANbuf = *(CANbuf+1) = 0;
        memcpy(CANbuf, msg->data, len);
        *emcyBuf++ = *CANbuf++;
        *emcyBuf = *CANbuf;
    }
    else{
        /* CAN buffer only */
        CO_memcpySwap4((uint8_t*) CANbuf++, (uint8_t*) &timeStamp);
        CO_memcpySwap4((uint8_t*) CANbuf++, (uint8_t*) &id_len);
        *CANbuf = *(CANbuf+1) = 0;
        memcpy(CANbuf, msg->data, len);
    }
}


/******************************************************************************/
void CgiLogSaveBuffer(CgiLog_t *CgiLog){

    if(CgiLog->maxDumpFiles == 0) return;

    /* find buffer */
    uint8_t bufIdx = CgiLog->CANBufIdx;
    uint8_t *buf = CgiLog->CANBuf[bufIdx];
    uint32_t bufLen;

    CO_DISABLE_INTERRUPTS();
    /* calculate size and write pointer of the oldest message */
    if(CgiLog->CANBufOvf){
        bufLen = CgiLog->CANBufSize;
        CO_memcpySwap4(buf+4, (uint8_t*) &CgiLog->CANBufOfs);
    }
    else{
        bufLen = CgiLog->CANBufOfs;
    }
    /* swap buffers */
    CgiLog->CANBufIdx = 1 - bufIdx;
    CgiLog->CANBufOvf = 0;
    CgiLog->CANBufOfs = 16;
    CO_ENABLE_INTERRUPTS();

    /* write timestamp in the new buffer */
    CgiLogTimestamp(CgiLog->CANBuf[CgiLog->CANBufIdx], 1);


    /* get filename and path of the saving buffer */
    char_t filename[300];
    char_t *webRootDir;
    TimeDate_Structure td;
    CGI_GetRootDir(&webRootDir);
    RTX_Get_TimeDate(&td);

    /* first make directory (if it does not exist) */
    int32_t fLen = sprintf(filename, "%c:%sdump", CGI_GetRootDrive()+'A', webRootDir);
    mkdir(filename);

    /* write buffer to file */
    sprintf(filename+fLen++, "\\%02d%02d%02d-%02d%02d%02d.dmp", td.yr, td.mn, td.dy, td.hr, td.min, td.sec);
    FILE *file = fopen(filename, "wb");
    if(file){
        fwrite(buf, 1, bufLen, file);
        fclose(file);
    }
    else{
        printf("\nCan't open %s", filename);
        return;
    }


    /* if there is more files than CgiLog->maxDumpFiles, delete the oldest one. */
    LFN_FILE_FIND ff;
    FS_FILETIME fTime;
    int32_t i = 0;

    strcpy(filename+fLen, "*.dmp");
    int32_t ret = BIOS_LFN_Findfirst(filename, &ff, 0);
    fTime.filedate = ff.filetimestamp.filedate;
    fTime.filetime = ff.filetimestamp.filetime;

    while(ret){
        if(ff.filetimestamp.filedate<fTime.filedate || (ff.filetimestamp.filedate==fTime.filedate && ff.filetimestamp.filetime<fTime.filetime)){
            fTime.filedate = ff.filetimestamp.filedate;
            fTime.filetime = ff.filetimestamp.filetime;
            strcpy(filename+fLen, ff.pszLong_Name);
        }
        ret = BIOS_LFN_Findnext(&ff);
        i++;
    }
    if(i > CgiLog->maxDumpFiles){
        remove(filename);
    }
}


/******************************************************************************/
static void CgiLogEmcyProcessOvf(uint8_t* tBuf, uint8_t errorBit, uint16_t errorCode){
    tBuf[8] = errorCode & 0xFF; tBuf[9] = (errorCode>>8) & 0xFF;
    tBuf[11] = errorBit;
    tBuf[6] = tBuf[7] = tBuf[10] = tBuf[12] = tBuf[13] = tBuf[14] = tBuf[15] = 0;
}
void CgiLogEmcyProcess(CgiLog_t *CgiLog){
    /* if suppressed, return */
    if(CgiLog->emcyTempBufHold){
        CgiLog->emcyTempBufHold--;
        return;
    }

    /* verify temporary buffer for new messages */
    uint16_t start = CgiLog->emcyTempBufStart;
    uint16_t stop  = CgiLog->emcyTempBufStop;
    if(start==stop) return;
    CgiLog->emcyTempBufStart = stop;

    /* get time */
    char_t sTime[30];
    TimeDate_Structure td;
    RTX_Get_TimeDate(&td);
    sprintf(sTime, "%02d.%02d.%02d %02d:%02d:%02d", td.dy, td.mn, td.yr, td.hr, td.min, td.sec);

    /* get buffers */
    uint32_t SramOfs = *CgiLog->emcyBufOfs;
    uint8_t *tBuf = CgiLog->emcyTempBuf;

    /* write emergencies from temporary buffer into SRAM */
    while(start != stop){
        /* verify if SRAM is full */
        if((SramOfs + 50) > CgiLog->emcyBufSize) break;/* SRAM is full */
        if((SramOfs + 100) > CgiLog->emcyBufSize){
            /* write emergency about the buffer full */
            CgiLogEmcyProcessOvf(&tBuf[start], CO_EM_EMERGENCY_BUFFER_FULL, CO_EMC_GENERIC);
        }
        uint8_t  nodeId         = tBuf[start+7]&0x7F;
        uint16_t errorCode      = (uint16_t) tBuf[start+9] << 8 | tBuf[start+8];
        uint8_t  errorRegister  = tBuf[start+10];
        uint8_t  errorIndex     = tBuf[start+11];
        uint32_t info;
        CO_memcpySwap4((uint8_t*)&info, &tBuf[start+12]);

        SramOfs += sprintf((CgiLog->emcyBuf+SramOfs),
                "%s - %02X - %04X %02X %02X %08X\n",
                sTime, nodeId, errorCode, errorRegister, errorIndex, (unsigned int)info);

        start += 16;
    }
    *CgiLog->emcyBufOfs = SramOfs;

    /* if next day, store emergencies from SRAM into file */
    if(CgiLog->emcyLastSavedDay != td.dy) CgiLogEmcySave(CgiLog);
}


/******************************************************************************/
void CgiLogEmcySave(CgiLog_t *CgiLog){
    /* get filename on the web root dir */
    char_t filename[300];
    char_t *webRootDir;
    CGI_GetRootDir(&webRootDir);
    sprintf(filename, "%c:%semcy.log", CGI_GetRootDrive()+'A', webRootDir);

    FILE *file;
    file = fopen(filename, "at");
    if(file){
        fputs(CgiLog->emcyBuf + 4, file);    /* write SRAM to file */
        *CgiLog->emcyBufOfs = 4;             /* start SRAM from the beginning */
        fclose(file);
    }
    else
        printf("\nCan't open %s", filename);

    /* update the emcyLastSavedDay */
    TimeDate_Structure td;
    RTX_Get_TimeDate(&td);
    CgiLog->emcyLastSavedDay = td.dy;
}

/*
 * CANopen master functions
 *
 * @file        CO_master.c
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


#include "CO_master.h"


/******************************************************************************/
int sdoClientUpload(
        CO_SDOclient_t *SDOclient,
        uint8_t         nodeID,
        uint16_t        idx,
        uint8_t         subidx,
        uint8_t        *dataRx,
        uint32_t        dataRxSize,
        uint32_t       *dataRxLen,
        uint32_t       *SDOabortCode,
        uint16_t        SDOtimeoutTime,
        uint8_t         blockTransferEnable)
{
    int err = 0;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);


    /* Setup client. */
    if(CO_SDOclient_setup(SDOclient, 0, 0, nodeID) != CO_SDOcli_ok_communicationEnd) {
        err = 1;
    }

    /* Initiate upload. */
    if(err == 0){
        if(CO_SDOclientUploadInitiate(SDOclient, idx, subidx, dataRx,
                dataRxSize, blockTransferEnable) != CO_SDOcli_ok_communicationEnd)
        {
            err = 1;
        }
    }

    /* Upload data. Loop in 10 ms intervals. */
    if(err == 0){
        CO_SDOclient_return_t ret;
        uint16_t timer1msPrev;
        struct timespec sleepTime;

        timer1msPrev = CO_timer1ms;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 10000000;

        do {
            uint16_t timer1ms, timer1msDiff;

            /* Calculate time difference */
            timer1ms = CO_timer1ms;
            timer1msDiff = timer1ms - timer1msPrev;
            timer1msPrev = timer1ms;

            ret = CO_SDOclientUpload(SDOclient, timer1msDiff, SDOtimeoutTime, dataRxLen, SDOabortCode);
            nanosleep(&sleepTime, NULL);
        } while(ret > 0);

        CO_SDOclientClose(SDOclient);

    }

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return err;
}


int sdoClientDownload(
        CO_SDOclient_t *SDOclient,
        uint8_t         nodeID,
        uint16_t        idx,
        uint8_t         subidx,
        uint8_t        *dataTx,
        uint32_t        dataTxLen,
        uint32_t       *SDOabortCode,
        uint16_t        SDOtimeoutTime,
        uint8_t         blockTransferEnable)
{
    int err = 0;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);


    /* Setup client. */
    if(CO_SDOclient_setup(SDOclient, 0, 0, nodeID) != CO_SDOcli_ok_communicationEnd) {
        err = 1;
    }

    /* Initiate download. */
    if(err == 0){
        if(CO_SDOclientDownloadInitiate(SDOclient, idx, subidx, dataTx,
                dataTxLen, blockTransferEnable) != CO_SDOcli_ok_communicationEnd)
        {
            err = 1;
        }
    }

    /* Download data. Loop in 5 ms intervals. */
    if(err == 0){
        CO_SDOclient_return_t ret;
        uint16_t timer1msPrev;
        struct timespec sleepTime;

        timer1msPrev = CO_timer1ms;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 5000000;

        do {
            uint16_t timer1ms, timer1msDiff;

            /* Calculate time difference */
            timer1ms = CO_timer1ms;
            timer1msDiff = timer1ms - timer1msPrev;
            timer1msPrev = timer1ms;

            ret = CO_SDOclientDownload(SDOclient, timer1msDiff, SDOtimeoutTime, SDOabortCode);
            nanosleep(&sleepTime, NULL);
        } while(ret > 0);

        CO_SDOclientClose(SDOclient);

    }

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return err;
}

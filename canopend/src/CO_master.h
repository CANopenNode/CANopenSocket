/**
 * CANopen master functions
 *
 * @file        CO_master.h
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


#ifndef CO_MASTER_H
#define CO_MASTER_H


#include "CO_driver.h"
#include "CO_SDO.h"
#include "CO_SDOmaster.h"
#include <pthread.h>


extern pthread_mutex_t CO_CAN_VALID_mtx; /* from main */
extern volatile uint16_t CO_timer1ms;    /* from main */


/**
 * Sdo client upload.
 *
 * For further details see CANopenNode/stack/CO_master.h file.
 * This is blocking function.
 *
 * @param SDOclient Pointer to CANopen SDO client object.
 * @param nodeID Node-ID of the remote node.
 * @param idx Index of object in object dictionary in remote node.
 * @param subidx Subindex of object in object dictionary in remote node.
 * @param dataRx Pointer to data buffer, into which received data will be written.
 * @param dataRxSize Maximum size of dataRx.
 * @param dataRxLen Return variable - actual data length in dataRx.
 * @param SDOabortCode Return variable - SDO abort code.
 * @param SDOtimeoutTime SDO timeout time in milliseconds.
 * @param blockTransferEnable If true, try to initiate block transfer.
 *
 * @return 0 on success.
 */
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
        uint8_t         blockTransferEnable);


/**
 * Sdo client download.
 *
 * For further details see CANopenNode/stack/CO_master.h file.
 * This is blocking function.
 *
 * @param SDOclient Pointer to CANopen SDO client object.
 * @param nodeID Node-ID of the remote node.
 * @param idx Index of object in object dictionary in remote node.
 * @param subidx Subindex of object in object dictionary in remote node.
 * @param dataTx Pointer to data buffer, which contains the data.
 * @param dataTxLen Length of data in dataTx.
 * @param SDOabortCode Return variable - SDO abort code.
 * @param SDOtimeoutTime SDO timeout time in milliseconds.
 * @param blockTransferEnable If true, try to initiate block transfer.
 *
 * @return 0 on success.
 */
int sdoClientDownload(
        CO_SDOclient_t *SDOclient,
        uint8_t         nodeID,
        uint16_t        idx,
        uint8_t         subidx,
        uint8_t        *dataTx,
        uint32_t        dataTxLen,
        uint32_t       *SDOabortCode,
        uint16_t        SDOtimeoutTime,
        uint8_t         blockTransferEnable);


#endif

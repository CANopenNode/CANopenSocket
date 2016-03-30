/**
 * CANopen master functions
 *
 * @file        CO_master.h
 * @author      Janez Paternoster
 * @copyright   2015 Janez Paternoster
 *
 * This file is part of CANopenSocket, a Linux implementation of CANopen
 * stack with master functionality. Project home page is
 * <https://github.com/CANopenNode/CANopenSocket>. CANopenSocket is based
 * on CANopenNode: <https://github.com/CANopenNode/CANopenNode>.
 *
 * CANopenSocket is free and open source software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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

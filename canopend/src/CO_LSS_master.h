/**
 * CANopen LSS master functions
 *
 * @file        CO_LSS_master.h
 * @author      Janez Paternoster, Martin Wagner
 * @copyright   2015 Janez Paternoster, 2017 Neuberger Gebäudeautomation GmbH
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


#ifndef CO_LSS_MASTER_H
#define CO_LSS_MASTER_H

#include "CANopen.h"
#include "CO_LSSmaster.h"

#include "CO_driver.h"
#include <pthread.h>


extern pthread_mutex_t CO_CAN_VALID_mtx; /* from main */
extern volatile uint16_t CO_timer1ms;    /* from main */

/**
 * Request LSS switch state select.
 *
 * For further details see CANopenNode/stack/CO_LSSmaster.h file.
 * This is blocking function.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @param global do global select if #true
 * @param vendorId LSS address vendor ID
 * @param productCode LSS address product code
 * @param revisionNo LSS address software revision number
 * @param serialNo LSS address device serial number
 * @return 0 on success, others see #CO_LSSmaster_return_t
 */
int lssSwitchStateSelect(
        CO_LSSmaster_t *LSSmaster,
        bool_t          global,
        uint32_t        vendorId,
        uint32_t        productCode,
        uint32_t        revisionNo,
        uint32_t        serialNo);

/**
 * Request LSS switch state deselect.
 *
 * For further details see CANopenNode/stack/CO_LSSmaster.h file.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @return 0 on success, others see #CO_LSSmaster_return_t
 */
int lssSwitchStateDeselect(
        CO_LSSmaster_t *LSSmaster);


/**
 * Request LSS configure Bit Timing
 *
 * For further details see CANopenNode/stack/CO_LSSmaster.h file.
 * This is blocking function.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @param bit bit rate entry from CiA301 bit timing table
 * @return 0 on success, others see #CO_LSSmaster_return_t
 */
int lssConfigureBitTiming(
        CO_LSSmaster_t *LSSmaster,
        uint16_t        bit);


/**
 * Request LSS configure node ID
 *
 * For further details see CANopenNode/stack/CO_LSSmaster.h file.
 * This is blocking function.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @param nodeId new node ID
 * @return 0 on success, others see #CO_LSSmaster_return_t
 */
int lssConfigureNodeId(
        CO_LSSmaster_t *LSSmaster,
        uint8_t         nodeId);


/**
 * Request LSS configure node ID
 *
 * For further details see CANopenNode/stack/CO_LSSmaster.h file.
 * This is blocking function.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @return 0 on success, others see #CO_LSSmaster_return_t
 */
int lssConfigureStore(
        CO_LSSmaster_t *LSSmaster);


/**
 * Request LSS activate bit timing
 *
 * For further details see CANopenNode/stack/CO_LSSmaster.h file.
 * This is blocking function.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @return 0 on success, others see #CO_LSSmaster_return_t
 */
int lssActivateBit(
        CO_LSSmaster_t *LSSmaster,
        uint16_t        switchDelay_ms);


/**
 * Request LSS inquire LSS address
 *
 * For further details see CANopenNode/stack/CO_LSSmaster.h file.
 * This is blocking function.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @param vendorId [out] LSS address vendor ID
 * @param productCode [out] LSS address product code
 * @param revisionNo [out] LSS address software revision number
 * @param serialNo [out] LSS address device serial number
 * @return 0 on success, others see #CO_LSSmaster_return_t
 */
int lssInquireLssAddress(
        CO_LSSmaster_t *LSSmaster,
        uint32_t       *vendorId,
        uint32_t       *productCode,
        uint32_t       *revisionNo,
        uint32_t       *serialNo);



/**
 * Request LSS inquire node ID
 *
 * For further details see CANopenNode/stack/CO_LSSmaster.h file.
 * This is blocking function.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @param nodeId [out] CANopen Node ID
 * @return 0 on success, others see #CO_LSSmaster_return_t
 */
int lssInquireNodeId(
        CO_LSSmaster_t *LSSmaster,
        uint8_t        *nodeId);

/**
 * Request LSS identify fastscan
 *
 * If this function returns successfully one node is selected and waiting
 * for commands. There is no further selection available
 *
 * For further details see CANopenNode/stack/CO_LSSmaster.h file.
 * This is blocking function.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @param timeout_ms LSS master timeout.
 * @param scanVendorId see #CO_LSSmaster_scantype_t
 * @param vendorId [in] [out] (found) LSS address vendor ID
 * @param scanProductCode see #CO_LSSmaster_scantype_t
 * @param productCode [in] [out] (found) LSS address product code
 * @param scanRevisionNumber see #CO_LSSmaster_scantype_t
 * @param revisionNo [in] [out] (found) LSS address software revision number
 * @param scanSerialNo see #CO_LSSmaster_scantype_t
 * @param serialNo [in] [out] (found) LSS address device serial number
 * @return CO_LSSmaster_SCAN_FINISHED on success, others see
 * #CO_LSSmaster_return_t
 */
int lssIdentifyFastscan(
        CO_LSSmaster_t *LSSmaster,
        uint16_t        timeout_ms,
        uint8_t         scanVendorId,
        uint32_t       *vendorId,
        uint8_t         scanProductCode,
        uint32_t       *productCode,
        uint8_t         scanRevisionNumber,
        uint32_t       *revisionNo,
        uint8_t         scanSerialNo,
        uint32_t       *serialNo);

/**
 * Request node enumeration by LSS identify fastscan
 *
 * This initiates node enumeration by the means of LSS fastscan mechanism.
 * When this function is finished
 * - all nodes that match the given criteria are assigned a node ID beginning
 *   with #nodeId. If 127 is reached, the assignment process is stopped, no
 *   matter if there are nodes remaining or not.
 * - no IDs are assigned because the given criteria do not match any node
 * - no IDs are assigned because all nodes are already configured
 *
 * This function needs that no node is selected when starting the scan process.
 *
 * This is blocking function.
 *
 * @param LSSmaster Pointer to CANopen LSS client object.
 * @param timeout_ms LSS master timeout.
 * @param nodeId ID to assign to the first found node
 * @param nodeCount [out] number of found nodes
 * @param store if set to "true",  LSS store configuration command is executed
 * @param scanVendorId see #CO_LSSmaster_scantype_t
 * @param vendorId LSS address vendor ID
 * @param scanProductCode see #CO_LSSmaster_scantype_t
 * @param productCode LSS address product code
 * @param scanRevisionNumber see #CO_LSSmaster_scantype_t
 * @param revisionNo LSS address software revision number
 * @param scanSerialNo see #CO_LSSmaster_scantype_t
 * @param serialNo LSS address device serial number
 * @return 0 on success, others see #CO_LSSmaster_return_t
 */
int lssEnumerateFastscan(
        CO_LSSmaster_t *LSSmaster,
        uint16_t        timeout_ms,
        uint8_t         nodeId,
        uint8_t        *nodeCount,
        bool_t          store,
        uint8_t         scanVendorId,
        uint32_t        vendorId,
        uint8_t         scanProductCode,
        uint32_t        productCode,
        uint8_t         scanRevisionNumber,
        uint32_t        revisionNo,
        uint8_t         scanSerialNo,
        uint32_t        serialNo);



#endif

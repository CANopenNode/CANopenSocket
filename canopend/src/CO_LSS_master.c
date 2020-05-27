/*
 * CANopen LSS master functions
 *
 * @file        CO_LSS_master.c
 * @author      Janez Paternoster, Martin Wagner
 * @copyright   2015 - 2020 Janez Paternoster, 2017 - 2020 Neuberger Gebäudeautomation GmbH
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

#include <string.h>

#include "CO_LSS_master.h"


/******************************************************************************/
int lssSwitchStateSelect(
        CO_LSSmaster_t *LSSmaster,
        bool_t          global,
        uint32_t        vendorId,
        uint32_t        productCode,
        uint32_t        revisionNo,
        uint32_t        serialNo)
{
    uint16_t timer1ms;
    uint16_t timer1msDiff;
    uint16_t timer1msPrev;
    CO_LSSmaster_return_t ret;
    CO_LSS_address_t lssAddress;
    CO_LSS_address_t *p;
    struct timespec sleepTime;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);

    /* Fill LSS address struct */
    if (global != true) {
      lssAddress.identity.vendorID = vendorId;
      lssAddress.identity.productCode = productCode;
      lssAddress.identity.revisionNumber = revisionNo;
      lssAddress.identity.serialNumber = serialNo;
      p = &lssAddress;
    }
    else {
      p = NULL;
    }

    /* Select Slave. Loop in 10 ms intervals. */

    timer1msPrev = CO_timer1ms;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10000000;

    timer1msDiff = 0;
    do {
        ret = CO_LSSmaster_switchStateSelect(LSSmaster, timer1msDiff, p);
        if (ret != CO_LSSmaster_WAIT_SLAVE) {
            break;
        }

        /* Calculate time difference */
        timer1ms = CO_timer1ms;
        timer1msDiff = timer1ms - timer1msPrev;
        timer1msPrev = timer1ms;
        nanosleep(&sleepTime, NULL);
    } while(1);

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return ret;
}


int lssSwitchStateDeselect(
        CO_LSSmaster_t *LSSmaster)
{
    CO_LSSmaster_return_t ret;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);

    ret = CO_LSSmaster_switchStateDeselect(LSSmaster);

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return ret;
}


int lssConfigureBitTiming(
        CO_LSSmaster_t *LSSmaster,
        uint16_t         bit)
{
    uint16_t timer1ms;
    uint16_t timer1msDiff;
    uint16_t timer1msPrev;
    CO_LSSmaster_return_t ret;
    struct timespec sleepTime;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);

    /* convert table to numbers */
    bit = CO_LSS_bitTimingTableLookup[bit];

    /* Configure Slave. Loop in 10 ms intervals. */

    timer1msPrev = CO_timer1ms;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10000000;

    timer1msDiff = 0;
    do {
        ret = CO_LSSmaster_configureBitTiming(LSSmaster, timer1msDiff, bit);
        if (ret != CO_LSSmaster_WAIT_SLAVE) {
            break;
        }

        /* Calculate time difference */
        timer1ms = CO_timer1ms;
        timer1msDiff = timer1ms - timer1msPrev;
        timer1msPrev = timer1ms;
        nanosleep(&sleepTime, NULL);
    } while(1);

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return ret;
}


int lssConfigureNodeId(
        CO_LSSmaster_t *LSSmaster,
        uint8_t         nodeId)
{
    uint16_t timer1ms;
    uint16_t timer1msDiff;
    uint16_t timer1msPrev;
    CO_LSSmaster_return_t ret;
    struct timespec sleepTime;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);

    /* Configure Slave. Loop in 10 ms intervals. */

    timer1msPrev = CO_timer1ms;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10000000;

    timer1msDiff = 0;
    do {
        ret = CO_LSSmaster_configureNodeId(LSSmaster, timer1msDiff, nodeId);
        if (ret != CO_LSSmaster_WAIT_SLAVE) {
            break;
        }

        /* Calculate time difference */
        timer1ms = CO_timer1ms;
        timer1msDiff = timer1ms - timer1msPrev;
        timer1msPrev = timer1ms;
        nanosleep(&sleepTime, NULL);
    } while(1);

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return ret;
}


int lssConfigureStore(
        CO_LSSmaster_t *LSSmaster)
{
    uint16_t timer1ms;
    uint16_t timer1msDiff;
    uint16_t timer1msPrev;
    CO_LSSmaster_return_t ret;
    struct timespec sleepTime;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);

    /* Configure Slave. Loop in 10 ms intervals. */

    timer1msPrev = CO_timer1ms;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10000000;

    timer1msDiff = 0;
    do {
        ret = CO_LSSmaster_configureStore(LSSmaster, timer1msDiff);
        if (ret != CO_LSSmaster_WAIT_SLAVE) {
            break;
        }

        /* Calculate time difference */
        timer1ms = CO_timer1ms;
        timer1msDiff = timer1ms - timer1msPrev;
        timer1msPrev = timer1ms;
        nanosleep(&sleepTime, NULL);
    } while(1);

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return ret;
}


int lssActivateBit(
        CO_LSSmaster_t *LSSmaster,
        uint16_t        switchDelay_ms)
{
    CO_LSSmaster_return_t ret;
    struct timespec sleepTime;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);

    ret = CO_LSSmaster_ActivateBit(LSSmaster, switchDelay_ms);
    if (ret == CO_LSSmaster_OK) {
        /* Sleep until switchover is finished */
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = switchDelay_ms * 1000000 * 2;
        nanosleep(&sleepTime, NULL);
    }

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return ret;
}


int lssInquireLssAddress(
        CO_LSSmaster_t *LSSmaster,
        uint32_t       *vendorId,
        uint32_t       *productCode,
        uint32_t       *revisionNo,
        uint32_t       *serialNo)
{
    uint16_t timer1ms;
    uint16_t timer1msDiff;
    uint16_t timer1msPrev;
    CO_LSSmaster_return_t ret;
    CO_LSS_address_t lssAddress;
    struct timespec sleepTime;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);

    /* Read value. Loop in 10 ms intervals. */

    timer1msPrev = CO_timer1ms;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10000000;

    timer1msDiff = 0;
    do {
        ret = CO_LSSmaster_InquireLssAddress(LSSmaster, timer1msDiff, &lssAddress);
        if (ret != CO_LSSmaster_WAIT_SLAVE) {
            break;
        }

        /* Calculate time difference */
        timer1ms = CO_timer1ms;
        timer1msDiff = timer1ms - timer1msPrev;
        timer1msPrev = timer1ms;
        nanosleep(&sleepTime, NULL);
    } while(1);

    /* Fill LSS address struct */
    *vendorId = lssAddress.identity.vendorID;
    *productCode = lssAddress.identity.productCode;
    *revisionNo = lssAddress.identity.revisionNumber;
    *serialNo = lssAddress.identity.serialNumber;

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return ret;
}


int lssInquireNodeId(
        CO_LSSmaster_t *LSSmaster,
        uint8_t        *nodeId)
{
    uint16_t timer1ms;
    uint16_t timer1msDiff;
    uint16_t timer1msPrev;
    CO_LSSmaster_return_t ret;
    struct timespec sleepTime;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);

    /* Read value. Loop in 10 ms intervals. */

    timer1msPrev = CO_timer1ms;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10000000;

    timer1msDiff = 0;
    do {
        ret = CO_LSSmaster_InquireNodeId(LSSmaster, timer1msDiff, nodeId);
        if (ret != CO_LSSmaster_WAIT_SLAVE) {
            break;
        }

        /* Calculate time difference */
        timer1ms = CO_timer1ms;
        timer1msDiff = timer1ms - timer1msPrev;
        timer1msPrev = timer1ms;
        nanosleep(&sleepTime, NULL);
    } while(1);

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return ret;
}


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
        uint32_t       *serialNo)
{
    uint16_t timer1ms;
    uint16_t timer1msDiff;
    uint16_t timer1msPrev;
    CO_LSSmaster_return_t ret;
    struct timespec sleepTime;
    CO_LSSmaster_fastscan_t fastscan;

    /* stay here, if CAN is not configured */
    pthread_mutex_lock(&CO_CAN_VALID_mtx);

    fastscan.scan[CO_LSS_FASTSCAN_VENDOR_ID] = scanVendorId;
    fastscan.match.identity.vendorID = *vendorId;
    fastscan.scan[CO_LSS_FASTSCAN_PRODUCT] = scanProductCode;
    fastscan.match.identity.productCode = *productCode;
    fastscan.scan[CO_LSS_FASTSCAN_REV] = scanRevisionNumber;
    fastscan.match.identity.revisionNumber = *revisionNo;
    fastscan.scan[CO_LSS_FASTSCAN_SERIAL] = scanSerialNo;
    fastscan.match.identity.serialNumber = *serialNo;
    /* Do scan. Loop in 5 ms intervals. */

    timer1msPrev = CO_timer1ms;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 5000000;

    CO_LSSmaster_changeTimeout(LSSmaster, timeout_ms);

    timer1msDiff = 0;
    do {
        ret = CO_LSSmaster_IdentifyFastscan(LSSmaster, timer1msDiff, &fastscan);
        if (ret != CO_LSSmaster_WAIT_SLAVE) {
            break;
        }

        /* Calculate time difference */
        timer1ms = CO_timer1ms;
        timer1msDiff = timer1ms - timer1msPrev;
        timer1msPrev = timer1ms;
        nanosleep(&sleepTime, NULL);
    } while(1);

    CO_LSSmaster_changeTimeout(LSSmaster, CO_LSSmaster_DEFAULT_TIMEOUT);

    *vendorId = fastscan.found.identity.vendorID;
    *productCode = fastscan.found.identity.productCode;
    *revisionNo = fastscan.found.identity.revisionNumber;
    *serialNo = fastscan.found.identity.serialNumber;

    pthread_mutex_unlock(&CO_CAN_VALID_mtx);

    return ret;
}


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
        uint32_t        serialNo)
{
    CO_LSSmaster_return_t ret;
    /* scanning changes those values */
    uint32_t tmpVendorId;
    uint32_t tmpProductCode;
    uint32_t tmpRevisionNo;
    uint32_t tmpSerialNo;

    /* we scan until no more nodes are found that match the scanning request */
    *nodeCount = 0;
    do {
        /* If we can't assing more node IDs, quit scanning */
        if (nodeId > 127) {
            nodeId = 127;
            ret = CO_LSSmaster_OK;
            break;
        }

        /* scanning changes those values */
        tmpVendorId = vendorId;
        tmpProductCode = productCode;
        tmpRevisionNo = revisionNo;
        tmpSerialNo = serialNo;

        ret = lssIdentifyFastscan(LSSmaster, timeout_ms,
                  scanVendorId, &tmpVendorId,
                  scanProductCode, &tmpProductCode,
                  scanRevisionNumber, &tmpRevisionNo,
                  scanSerialNo, &tmpSerialNo);
        if (ret == CO_LSSmaster_SCAN_NOACK) {
            /* no (more) nodes found */
            ret = CO_LSSmaster_OK;
            break;
        }
        else if (ret != CO_LSSmaster_SCAN_FINISHED) {
            /* error occured */
            break;
        }

        ret = lssConfigureNodeId(LSSmaster, nodeId);
        if (ret != CO_LSSmaster_OK) {
            break;
        }

        if (store == true) {
            ret = lssConfigureStore(LSSmaster);
            if (ret != CO_LSSmaster_OK) {
                break;
            }
        }

        ret = lssSwitchStateDeselect(LSSmaster);
        if (ret != CO_LSSmaster_OK) {
            break;
        }

        nodeId ++;
        (*nodeCount) ++;
    } while(1);

    return ret;
}


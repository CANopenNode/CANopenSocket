/**
 * CANopen time stamp interface.
 *
 * @file        CO_timeStamp.h
 * @author      Janez Paternoster
 * @copyright   2016 - 2020 Janez Paternoster
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


#ifndef CO_TIME_STAMP_H
#define CO_TIME_STAMP_H


#include "CO_driver.h"
#include "CO_SDO.h"


/**
 * Time object, usable for timestamping.
 */
typedef struct {
    uint64_t           *epochTimeBaseMs;    /**< From CO_timeStamp_init(). */
    uint32_t           *epochTimeOffsetMs;  /**< From CO_timeStamp_init(). */
} CO_timeStamp_t;


/**
 * Initialize time object.
 *
 * Function must be called in the communication reset section.
 *
 * @param tm This object will be initialized.
 * @param SDO SDO server object.
 * @param epochTimeBaseMs Pointer to milliseconds since Unix Epoch (1.1.1970, 00:00:00, UTC) - base.
 * @param epochTimeOffsetMs pointer to offset. Set to 0 at program startup and increments since then.
 * @param idx_OD_time Index in Object Dictionary.
 */
void CO_timeStamp_init(
        CO_timeStamp_t         *tm,
        CO_SDO_t               *SDO,
        uint64_t               *epochTimeBaseMs,
        uint32_t               *epochTimeOffsetMs,
        uint16_t                idx_OD_time);


/**
 * Process time object.
 *
 * Function must be called cyclically in 1ms intervals.
 *
 * @param tm This object.
 */
void CO_timeStamp_process(CO_timeStamp_t *tm);


#endif /* CO_TIME_STAMP_H */

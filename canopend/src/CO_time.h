/**
 * CANopen time interface.
 *
 * @file        CO_time.h
 * @author      Janez Paternoster
 * @copyright   2016 Janez Paternoster
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


#ifndef CO_TIME_H
#define CO_TIME_H


#include "CO_driver.h"
#include "CO_SDO.h"


/**
 * Time object, usable for timestamping.
 */
typedef struct {
    uint64_t           *epochTimeBaseMs;    /**< From CO_time_init(). */
    uint32_t           *epochTimeOffsetMs;  /**< From CO_time_init(). */
} CO_time_t;


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
void CO_time_init(
        CO_time_t              *tm,
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
void CO_time_process(CO_time_t *tm);


#endif

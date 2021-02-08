/**
 * Application interface for CANopenNode.
 *
 * @file        CO_application.h
 * @author      Janez Paternoster
 * @copyright   2020 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
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


#ifndef CO_APPLICATION_H
#define CO_APPLICATION_H


#include "CANopen.h"


/* Optional OD entry for CO_CANopenInit() -> CO_EM_init() */
#define OD_STATUS_BITS OD_ENTRY_H2100_errorStatusBits


/**
 * Function is called on program startup.
 *
 * @param CANopenConfigured True, if CANopen Node-Id is known and all CANopen
 * objects are configured.
 * @param [out] errInfo Variable may indicate additional information for some
 * types of errors.
 *
 * @return @ref CO_ReturnError_t CO_ERROR_NO in case of success.
 */
CO_ReturnError_t app_programStart(bool_t CANopenConfigured, uint32_t *errInfo);


/**
 * Function is called after CANopen communication reset.
 *
 * @param CANopenConfigured True, if CANopen Node-Id is known and all CANopen
 * objects are configured.
 */
void app_communicationReset(bool_t CANopenConfigured);


/**
 * Function is called just before program ends.
 */
void app_programEnd();


/**
 * Function is called cyclically from main.
 *
 * @param CANopenConfigured True, if CANopen Node-Id is known and all CANopen
 * objects are configured.
 * @param timer1usDiff Time difference since last call in microseconds
 */
void app_programAsync(bool_t CANopenConfigured, uint32_t timer1usDiff);


/**
 * Function is called cyclically from realtime thread at constant intervals.
 *
 * Code inside this function must be executed fast. Take care on race conditions
 * with app_programAsync.
 *
 * @param CANopenConfigured True, if CANopen Node-Id is known and all CANopen
 * objects are configured.
 * @param timer1usDiff Time difference since last call in microseconds
 */
void app_program1ms(bool_t CANopenConfigured, uint32_t timer1usDiff);


#endif /* CO_APPLICATION_H */

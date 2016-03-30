/*
 * CANopen time interface.
 *
 * @file        CO_time.c
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


#include "CO_time.h"
#include <time.h>
#include <string.h>


/* Set epochTime base and offset */
static void timeZero(CO_time_t *tm) {
    struct timespec tspec;
    struct tm *tstruct;

    clock_gettime(CLOCK_REALTIME, &tspec);
    tstruct = localtime(&tspec.tv_sec);

    /* epochTimeBaseMs is rounded to minutes, 0 seconds, 0 ms. */
    *tm->epochTimeBaseMs = ((uint64_t) (tspec.tv_sec - tstruct->tm_sec)) * 1000;
    *tm->epochTimeOffsetMs = (tspec.tv_nsec / 1000000) + (tstruct->tm_sec * 1000);
}


/* OD function for accessing _OD_time_ (index 0x2130) from SDO server.
 * For more information see file CO_SDO.h. */
static CO_SDO_abortCode_t CO_ODF_time(CO_ODF_arg_t *ODF_arg) {
    CO_time_t *tm;
    CO_SDO_abortCode_t ret = CO_SDO_AB_NONE;

    tm = (CO_time_t*) ODF_arg->object;

    /* Reading Object Dictionary variable */
    if(ODF_arg->reading) {

        /* return current time as string */
        if(ODF_arg->subIndex == 1){
            time_t t;
            char* timeString;

            t = time(NULL);
            timeString = ctime(&t);

            strncpy((char*) ODF_arg->data, timeString, ODF_arg->dataLength);
            ODF_arg->data[ODF_arg->dataLength - 1] = 0;
        }
    }
    else {
        if(ODF_arg->subIndex == 3) { /* write zero to epochTimeOffsetMs */
            uint32_t *value = (uint32_t*) ODF_arg->data;

            if(*value == 0) {
                timeZero(tm);
                *value = *tm->epochTimeOffsetMs;
            }
            else {
                ret = CO_SDO_AB_INVALID_VALUE;
            }
        }
    }

    return ret;
}


/******************************************************************************/
void CO_time_init(
        CO_time_t              *tm,
        CO_SDO_t               *SDO,
        uint64_t               *epochTimeBaseMs,
        uint32_t               *epochTimeOffsetMs,
        uint16_t                idx_OD_time)
{

    tm->epochTimeBaseMs = epochTimeBaseMs;
    tm->epochTimeOffsetMs = epochTimeOffsetMs;

    if(*epochTimeBaseMs == 0) {
        timeZero(tm);
    }

    CO_OD_configure(SDO, idx_OD_time, CO_ODF_time, (void*)tm, 0, 0);
}


/******************************************************************************/
void CO_time_process(CO_time_t *tm){

    if(++(*tm->epochTimeOffsetMs) == 0) {
        /* overflow happened after ~50 days */
        timeZero(tm);
    }
}

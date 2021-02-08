/*
 * Application interface for CANopenNode.
 *
 * @file        CO_application.c
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


#include "CO_application.h"
#include "CO_version.h"
#include "OD.h"
#include "testingVariables.h"
#include <stdio.h>


/* Define object for testingVariables */
testingVariables_t testVar;
/* Extension for OD object */
OD_extension_t OD_version_extension;


/*
 * Custom function for reading OD object _Version_
 *
 * For more information see file CO_ODinterface.h, OD_IO_t.
 */
static OD_size_t OD_read_version(OD_stream_t *stream, uint8_t subIndex,
                                 void *buf, OD_size_t count,
                                 ODR_t *returnCode)
{
    if (stream == NULL || buf == NULL || returnCode == NULL) {
        if (returnCode != NULL) *returnCode = ODR_DEV_INCOMPAT;
        return 0;
    }

    switch (subIndex) {
        case 1: {
            OD_size_t len = strlen(CO_VERSION_CANOPENNODE);
            if (len > count) len = count;
            memcpy(buf, CO_VERSION_CANOPENNODE, len);
            *returnCode = ODR_OK;
            return stream->dataLength = len;
        }
        case 2: {
            OD_size_t len = strlen(CO_VERSION_APPLICATION);
            if (len > count) len = count;
            memcpy(buf, CO_VERSION_APPLICATION, len);
            *returnCode = ODR_OK;
            return stream->dataLength = len;
        }
        default:
            return OD_readOriginal(stream, subIndex, buf, count, returnCode);
    }
}


/******************************************************************************/
CO_ReturnError_t app_programStart(bool_t CANopenConfigured, uint32_t *errInfo) {
    (void) CANopenConfigured;
    CO_ReturnError_t err = CO_ERROR_NO;

    /* increment auto-storage variable */
    OD_PERSIST_TEST_AUTO.x2106_power_onCounter ++;

    /* Initialize custom read-only OD object "Version" */
    OD_version_extension.object = NULL;
    OD_version_extension.read = OD_read_version;
    OD_version_extension.write = NULL;
    OD_extension_init(OD_ENTRY_H2105_version, &OD_version_extension);

    /* Initialize more advanced object, which operates with testing variables
     * OD_ENTRY_H2120_testingVariables is constant defined in OD.h. More
     * flexible alternative for third argument is 'OD_find(&OD, 0x2120)' */
    err = testingVariables_init(&testVar,
                                errInfo,
                                OD_ENTRY_H2120_testingVariables);

    return err;
}


/******************************************************************************/
void app_communicationReset(bool_t CANopenConfigured) {

    /* example printouts */
    if (CANopenConfigured)
        printf("CANopen Node-ID is configured and all services will work.\n");
    else
        printf("CANopen Node-ID is unconfigured, so only LSS slave will work.\n");

    printf("Printing 'OD_PERSIST_APP.x2120_testingVariables.stringLong':\n%s\n",
           OD_PERSIST_TEST.x2120_testingVariables.stringLong);

    fflush(stdout);
}


/******************************************************************************/
void app_programEnd() {

}


/******************************************************************************/
void app_programAsync(bool_t CANopenConfigured, uint32_t timer1usDiff) {
    (void) CANopenConfigured; (void) timer1usDiff; /* unused */
}


/******************************************************************************/
void app_program1ms(bool_t CANopenConfigured, uint32_t timer1usDiff) {
    (void) CANopenConfigured; (void) timer1usDiff; /* unused */
}

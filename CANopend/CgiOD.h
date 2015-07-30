/**
 * Object for CGI function, which acts as SDO client. It can access CANopen
 * Object Dictionary on any device.
 *
 * @file        CgiOD.h
 * @ingroup     CO_CGI_ODClient
 * @version     SVN: \$Id$
 * @author      Janez Paternoster
 * @copyright   2004 - 2013 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <http://canopennode.sourceforge.net>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * CANopenNode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesr General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef CGI_OD_H
#define CGI_OD_H

#include "clib.h"
#include "CO_driver.h"


/**
 * @defgroup CO_CGI_ODClient Web OD client
 * @ingroup CO_CGI
 * @{
 *
 * Object dictionary client is a CGI function. It provides access to object
 * dictionary on any device on CANopen network. It can be used with web browser
 * with html get method. CGI function name is "ODcli".
 *
 * ###Usage
 * http://host/odcli?wnniiiissll=xxxx[&rnniiiissll=]
 *  - w:    'w'rite or 'r'ead.
 *  - nn:   node ID in hex format.
 *  - iiii: Object dictionary index in hex format.
 *  - ss:   Object dictionary subindex in hex format.
 *  - ll:   length of variable (1 to FFFFFFFF) in hex format. If reading, this
 *          value is ignored.
 *  - xxxx: Value to be written in hex and little endian format. Value is ignored
 *          when reading.
 *
 * A text file is returned in following format (AB followed with code means abort):
 * w nn iiii ss ll OK: xx xx
 * w nn iiii ss ll AB: yyyyyyyy
 * r nn iiii ss ll OK: xx xx
 * r nn iiii ss ll AB: yyyyyyyy
 */


/**
 * CGI OD client object.
 */
typedef struct{
    char_t             *buf;            /**< From CgiCli_init_1() */
    uint32_t            bufSize;        /**< From CgiCli_init_1() */
    uint8_t            *SDOBuf;         /**< From CgiCli_init_1() */
    uint32_t            SDOBufSize;     /**< From CgiCli_init_1() */
    CGI_Entry           cgiEntry;       /**< Holds information on the CGI function */
}CgiCli_t;


/**
 * Initialize CGI handler.
 *
 * @param CgiCli This object will be initialized.
 * @param buf Buffer for CGI web page.
 * @param bufSize Size of the above buffer.
 * @param SDOBuf Buffer for SDO data.
 * @param SDOBufSize Size of the above buffer. Maximum size of SDO object in SDO master.
 *
 * @return #CO_ReturnError_t CO_ERROR_NO, CO_ERROR_ILLEGAL_ARGUMENT or CO_ERROR_PARAMETERS
 */
int16_t CgiCli_init_1(
        CgiCli_t               *CgiCli,
        char_t                 *buf,
        uint32_t                bufSize,
        uint8_t                *SDOBuf,
        uint32_t                SDOBufSize);


/**
 * Delete CGI handler.
 *
 * @param CgiCli Object, which contains CGI handler.
 */
void CgiCli_delete(CgiCli_t *CgiCli);


/** @} */
#endif

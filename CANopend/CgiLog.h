/**
 * Object for CGI function, which logs CAN messages as two separate logs. One
 * for Emergency messages (saving to file) and one temporary all messages log.
 *
 * @file        CgiLog.h
 * @ingroup     CO_CGI_CANlog
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef CGI_LOG_H
#define CGI_LOG_H

#include "clib.h"
#include "CO_driver.h"

/* Define global macro:     projectProperties -> C/C++General */
/*       -> PathsAndSymbols -> Symbols -> add -> CO_LOG_CAN_MESSAGES */
#ifndef CO_LOG_CAN_MESSAGES
#error Macro CO_LOG_CAN_MESSAGES must be defined globally!
#endif


/**
 * @defgroup CO_CGI_CANlog Web CAN logging
 * @ingroup CO_CGI
 * @{
 * All CAN messages are stored into RAM buffer.
 *
 * Received or transmitted messages from CO_driver.h are inside CO_logMessage()
 * function copied into RAM buffer 'CANBuf'. Buffer is circular binary. The
 * oldest message may be in the middle of the buffer. Head contains two
 * timestamps, both represents to the same time. Timestamp in the head is always
 * older than timestamps of the messages, but not to old.
 *
 * ####Contents of CAN message log
 *  - First 16 bytes is message head.
 *  - All other are CAN messages, each message is 16-byte wide.
 *
 * ####Format of the head - TTTTMMMMYMDHMSmm
 *  - T: 32 bit timestamp in milliseconds (little endian). It overflows in 50 days.
 *  - M: Address of the oldest CAN message.
 *  - Y: Year (0-99).
 *  - M: Month (1-12).
 *  - D: Day (1-31).
 *  - H: Hour (0-23).
 *  - M: Minute (0-59).
 *  - S: Second (0-59).
 *  - m: 16 bit millisecond (0-999) (little endian).
 *
 * ####Format of the CAN message - TTTTAA0Ndddddddd
 *  - T: 32 bit timestamp in milliseconds (little endian). It overflows in 50 days.
 *  - A: 11-bit CAN address (little endian).
 *  - 0: Zero byte.
 *  - N: Number of data bytes.
 *  - d: 8 data bytes.
 *
 * ####RAM buffer access
 * Buffer is accessible via CGI function <http://host/CANlog>. It returns direct
 * copy of RAM buffer, which is in binary format. <http://host/CANlog?clear>
 * clears the RAM buffer and returns nothing. <http://host/CANlog?dump>
 * saves the the RAM buffer to disc and returns nothing.
 * Internally there are two RAM buffers. When CGI access occurs, buffer maps to
 * web server. In the same time the second buffer empties and maps to CAN
 * logging. So CGI function always returns only new CAN messages since last read.
 * RAM buffer may also be stored to file. (In case of fatal error.)
 *
 *
 * ##Emergency message logging
 *
 * Emergency messages from all CANopen devices are logged into file.
 *
 * Received or transmitted messages from CO_driver.h are inside
 * CO_logMessage() function first verified if they are emergency type. If yes,
 * raw message is first copied to temporary circular buffer 'emcyTempBuf'.
 *
 * Function CgiLogEmcyProcess() is called cyclically from main. It formats raw
 * emergency messages from temporary buffer and stores them to battery powered
 * SRAM as texts separated by newlines. SRAM contains emergency messages from
 * midnight. Contents of SRAM can be accessed via CGI function <http://host/emcy>.
 *
 * ####Contents of SRAM are copied to the file "emcy.log" on the following
 * conditions:
 *  - After midnight. (If there is more Emergencies per day then is the size of
 *     the SRAM buffer, extra emergencies are lost.)
 *  - After the CGI call: <http://host/emcy?flush>. (CGI updates the emcy.log
 *     file and returns nothing.)
 *  - On the program startup.
 *
 * ####emcy.log file contents - TT.TT.TT TT:TT:TT - NN - CCCC EE II AAAAAAAA
 *  - T: timestamp (date, time).
 *  - N: NodeId.
 *  - C: CANopen error code.
 *  - E: CANopen error register.
 *  - I: Index of error condition (stack specific).
 *  - A: Additional information (stack specific).
 * Byte order of emergency message in log file is not equal as in CAN message.
 * For more information on emergency message see <CO_Emergency.h>. First
 * four bytes in SRAM are used as a pointer, all other are as described above.
 *
 * ####Temporary buffer message format - TTTTAAAAdddddddd - 16 bytes
 *  - T: 32 bit timestamp in milliseconds (big endian).
 *  - A: 11-bit CAN address with length in MSB byte (big endian).
 *  - d: 8 data bytes.
 */


/**
 * CGI log object.
 */
typedef struct{
    char_t     *emcyBuf;                /**< From CgiLog_init_1() */
    uint32_t    emcyBufSize;            /**< From CgiLog_init_1() */
    uint8_t    *CANBuf[2];              /**< From CgiLog_init_1() */
    uint32_t    CANBufSize;             /**< From CgiLog_init_1() */
    uint32_t    maxDumpFiles;           /**< From CgiLog_init_1() */
    /** Pointer to location, which contains offset on the emcyBuf for new message */
    uint32_t   *emcyBufOfs;
    /** Temporary FIFO buffer for emergencies. On this size uint16_t emcyTempBufStart/stop
        will overflow, don't change this. Before this must be 32-bit variable.*/
    uint8_t     emcyTempBuf[0x10000];
    uint16_t    emcyTempBufStart;       /**< Start location in buffer */
    uint16_t    emcyTempBufStop;        /**< Stop location in buffer */
    uint16_t    emcyTempBufHold;        /**< Suppress CgiLogEmcyProcess() function for number of cycles */
    uint8_t     emcyLastSavedDay;       /**< The day in month, SRAM was last saved */
    uint8_t     CANBufTimestamp[16];    /**< Auxiliary buffer for timestamp */
    uint8_t     CANBufIdx;              /**< Index of the buffer used */
    uint8_t     CANBufOvf;              /**< True, if circular buffer is full */
    uint32_t    CANBufOfs;              /**< Offset in the buffer for the new message */
    CGI_Entry   cgiEntryEmcy;           /**< Holds information on the CGI emcy function */
    CGI_Entry   cgiEntryCAN;            /**< Holds information on the CGI CAN function */
}CgiLog_t;


/**
 * Initialize CGI handler.
 *
 * @param CgiLog This object will be initialized.
 * @param emcyBuf Pointer to the emergency buffer in SRAM.
 * @param emcyBufSize Size of the above buffer. It limits maximum
 * messages per day and prevents from too large log file.
 * @param CANBuf0, CANBuf1 Two CAN log buffers.
 * @param CANBufSize Size of each of the above buffers.
 * @param maxDumpFiles Maximum number of CAN log dump files on disc.
 *
 * @return #CO_ReturnError_t CO_ERROR_NO, CO_ERROR_ILLEGAL_ARGUMENT or CO_ERROR_PARAMETERS
 */
int16_t CgiLog_init_1(
        CgiLog_t               *CgiLog,
        uint8_t                *emcyBuf,
        uint32_t                emcyBufSize,
        uint8_t                *CANBuf0,
        uint8_t                *CANBuf1,
        uint32_t                CANBufSize,
        uint32_t                maxDumpFiles);


/**
 * Delete CGI handler.
 *
 * @param CgiLog Object, which contains CGI handler.
 */
void CgiLog_delete(CgiLog_t *CgiLog);


/**
 * Copy CAN message to buffers. It is called by driver.
 *
 * @param msg CAN message.
 */
void CO_logMessage(const CanMsg *msg);


/**
 * Store the complete RAM buffer (CAN log) to filesystem.
 *
 * File path is: webRootDir\dump\. File name is generated automatically from
 * date of storage and is: YYMMDD-HHMMSS.dmp. To prevent disk full, older dump
 * files are automatically deleted in this function. Variable 'maxDumpFiles'
 * is used to limit the number of files on disc.
 *
 * @param CgiLog CgiLog object.
 */
void CgiLogSaveBuffer(CgiLog_t *CgiLog);


/**
 * Process emergency messages from temporary RAM buffer to SRAM buffer.
 *
 * @param CgiLog  CgiLog object.
 */
void CgiLogEmcyProcess(CgiLog_t *CgiLog);


/**
 * Save emergencies from SRAM into file.
 *
 * @param CgiLog CgiLog object.
 */
void CgiLogEmcySave(CgiLog_t *CgiLog);


/** @} */
#endif

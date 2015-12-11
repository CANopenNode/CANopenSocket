/**
 * Command interface for CANopenSocket.
 *
 * @file        CO_command.h
 * @author      Janez Paternoster
 * @copyright   2015 Janez Paternoster
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


#ifndef CO_COMMAND_H
#define CO_COMMAND_H


/* Functions from external */
void CO_errExit(char* msg);
void CO_error(const uint32_t info);


/**
 * Path for local type  socket. External process can access command interface
 * over this socket. By default its path is "/tmp/CO_command_socket".
 */
extern char *CO_command_socketPath;


/**
 * Initialize thread and create socket for command interface.
 *
 * @param CO_SDOclient Pointer to CANopen SDO client object.
 *
 * @return 0 on success.
 */
int CO_command_init(CO_SDOclient_t *CO_SDOclient);


/**
 * Terminate thread and remove socket.
 *
 * @return 0 on success.
 */
int CO_command_clear(void);


#endif

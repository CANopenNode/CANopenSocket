/**
 * Command interface for CANopenSocket.
 *
 * @file        CO_command.h
 * @author      Janez Paternoster
 * @copyright   2015 - 2020 Janez Paternoster
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


#ifndef CO_COMMAND_H
#define CO_COMMAND_H

#include <netinet/in.h>


/* Functions from external */
void CO_errExit(char* msg);
void CO_error(const uint32_t info);


/**
 * Path for local type  socket. External process can access command interface
 * over this socket. By default its path is "/tmp/CO_command_socket".
 */
extern char *CO_command_socketPath;


/**
 * Initialize thread and create local domain socket for command interface.
 *
 * Make sure, that global variable CO was properly initialized before this call.
 *
 * @return 0 on success.
 */
int CO_command_init(void);

/**
 * Initialize thread and create tcp socket for command interface.
 * @param port tcp port to listen
 *
 * Make sure, that global variable CO was properly initialized before this call.
 * @return 0 on success.
 */
int CO_command_init_tcp(in_port_t port);


/**
 * Terminate thread and remove socket.
 *
 * @return 0 on success.
 */
int CO_command_clear(void);

/**
 * Terminate thread and remove tcp socket.
 * @param port tcp port to connect to
 *
 * @return 0 on success.
 */
int CO_command_clear_tcp(in_port_t port);


#endif

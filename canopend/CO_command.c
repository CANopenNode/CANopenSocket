/*
 * Socket command interface for CANopenSocket.
 *
 * @file        CO_command.c
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


#include "CANopen.h"
#include "CO_command.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>




#define SOCKET_BUF_SIZE     1000
#define LISTEN_BACKLOG      50


/* Externals */
void CO_errExit(char* msg);
void CO_error(const uint32_t info);
extern volatile sig_atomic_t CO_endProgram;


/* Globals */
char                       *CO_command_socketPath = "/tmp/CO_command_socket";  /* Name of the local domain socket */


/* Variables */
static void*                CO_command_thread(void* arg);
static pthread_t            CO_command_thread_id;
static int                  fdSocket;


/******************************************************************************/
int CO_command_init(void) {
    static struct sockaddr_un addr;

    /* Create, bind and listen socket */
    fdSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fdSocket == -1) {
        CO_errExit("CO_command_init - socket failed");
    }

    if (remove(CO_command_socketPath) == -1 && errno != ENOENT) {
        CO_errExit("CO_command_init - socket remove failed");
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, CO_command_socketPath, sizeof(addr.sun_path) - 1);

    if (bind(fdSocket, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        CO_errExit("CO_command_init - bind failed");
    }

    if (listen(fdSocket, LISTEN_BACKLOG) == -1) {
        CO_errExit("CO_command_init - listen failed");
    }

    /* Create thread */
    if(pthread_create(&CO_command_thread_id, NULL, CO_command_thread, NULL) != 0) {
        CO_errExit("CO_command_init - thread creation failed");
    }

    return 0;
}


/******************************************************************************/
int CO_command_clear(void) {
    static struct sockaddr_un addr;
    int fd;

    /* Establish a client socket connection to finish the CO_command_thread. */
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd == -1) {
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, CO_command_socketPath, sizeof(addr.sun_path) - 1);

    if(connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        return -1;
    }

    close(fd);

    /* Wait for thread to finish. */
    if(pthread_join(CO_command_thread_id, NULL) != 0) {
        return -1;
    }

    /* Remove socket from filesystem. */
    if (remove(CO_command_socketPath) == -1) {
        return -1;
    }

    return 0;
}


/******************************************************************************/
static void* CO_command_thread(void* arg) {
    int fd;
    ssize_t n;
    char buf[SOCKET_BUF_SIZE];

    /* Almost endless loop */
    while(CO_endProgram == 0) {

        /* wait for new command */
        fd = accept(fdSocket, NULL, NULL);
        if(fd == -1) {
            CO_error(0x15100000L);
        }

        /* Read command and send answer. */
        while ((n = read(fd, buf, SOCKET_BUF_SIZE)) > 0) {
            printf("serv:%s\n", buf);
        }

        /* close current communication */
        if(n == -1){
            CO_error(0x15800000L);
        }

        if(close(fd) == -1) {
            CO_error(0x15900000L);
        }
    }

    return NULL;
}

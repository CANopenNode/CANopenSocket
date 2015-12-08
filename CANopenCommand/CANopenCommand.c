/*
 * Client socket command interface for CANopenSocket.
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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>




#define BUF_SIZE            10000


/* Helper functions */
void errExit(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void usageExit(char *progName) {
    fprintf(stderr, "Usage: %s [options] <command string>\n", progName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -s <socket path>    Path of the socket (default '/tmp/CO_command_socket').\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Command string:\n");
    fprintf(stderr, "  -s <socket path>    Path of the socket (default '/tmp/CO_command_socket').\n");
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void sendCommand(int fd, char* command, size_t commandLength);


/******************************************************************************/
int main (int argc, char *argv[]) {
    char *socketPath = "/tmp/CO_command_socket";  /* Name of the local domain socket, configurable by arguments. */

    char buf[BUF_SIZE];
    int fd;
    struct sockaddr_un addr;
    int opt;
    int i;

    if(argc >= 2 && strcmp(argv[1], "--help") == 0) {
        usageExit(argv[0]);
    }

    /* Get program options */
    while((opt = getopt(argc, argv, "s:")) != -1) {
        switch (opt) {
            case 's': socketPath = optarg;     break;
            default:  usageExit(argv[0]);
        }
    }

    /* Create and connect client socket */
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd == -1) {
        errExit("Socket creation failed");
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);

    if(connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        errExit("Socket connect failed");
    }


    /* get command from arguments */
    if(optind < argc) {
        buf[0] = 0;
        size_t buflen = 0;

        for(i=optind; i<argc; i++) {
            strncat(buf, argv[i], (BUF_SIZE - 2) - buflen);
            strcat(buf, " ");
            buflen = strlen(buf);
            if(buflen >= (BUF_SIZE - 1)) {
                fprintf(stderr, "String too long!\n");
                exit(EXIT_FAILURE);
            }
        }
        buf[buflen - 1] = '\n'; /* replace last space with newline */
        sendCommand(fd, buf, buflen);
    }

    /* get commands from stdin, line after line */
    else {
        while(fgets(buf, BUF_SIZE, stdin) != NULL) {
            sendCommand(fd, buf, strlen(buf));
        };
    }

    close(fd);

    exit(EXIT_SUCCESS);
}


static void sendCommand(int fd, char* command, size_t commandLength) {
    size_t n;
    char buf[1000];

    printf("com:%s\n", command);

    if (write(fd, command, commandLength) != commandLength) {
        errExit("Socket write failed");
    }

    n = read(fd, buf, sizeof(buf));

    if(n == -1) {
        errExit("Socket read failed");
    }

    printf("ans:%s\n", buf);
}


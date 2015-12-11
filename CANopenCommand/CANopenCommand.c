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
    fprintf(stderr, "\n");
    fprintf(stderr, "Program reads arguments or standard input or file. It sends commands to\n");
    fprintf(stderr, "canopend via socket, line after line. Result is printed to standard output.\n");
    fprintf(stderr, "For more information see http://www.can-cia.org/, CiA 309 standard.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -f <input file path>  Path of the input file.\n");
    fprintf(stderr, "  -s <socket path>      Path of the socket (default '/tmp/CO_command_socket').\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Command strings:\n");
    fprintf(stderr, "  - SDO upload:   \"[\"<sequence>\"]\" [[net] node] r[ead]  <index> <subindex> [<datatype>]\n");
    fprintf(stderr, "  - SDO download: \"[\"<sequence>\"]\" [[net] node] w[rite] <index> <subindex> <datatype> <value>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  - Datatypes:\n");
    fprintf(stderr, "      - b - Boolean.\n");
    fprintf(stderr, "      - u8, u16, u32, u64 - Unsigned integers.\n");
    fprintf(stderr, "      - i8, i16, i32, i64 - Signed integers.\n");
    fprintf(stderr, "      - r32, r64 - Real numbers.\n");
    fprintf(stderr, "      - t, td - Time of day, time difference.\n");
    fprintf(stderr, "      - vs - Visible string (between double quotes).\n");
    fprintf(stderr, "      - os, us, d - Octet string, unicode string, domain (mime-base64 (RFC2045) should be used).\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  - Response: \"[\"<sequence>\"]\" (OK | <value> | ERROR: (<SDO-abort-code> | <internal-error-code>))\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  - Internal error codes:\n");
    fprintf(stderr, "      - 100 - Request not supported.\n");
    fprintf(stderr, "      - 101 - Syntax error.\n");
    fprintf(stderr, "      - 102 - Request not processed due to internal state.\n");
    fprintf(stderr, "      - 105 - No default node set.\n");
    fprintf(stderr, "      - 106 - Unsupported net.\n");
    fprintf(stderr, "      - 107 - Unsupported node.\n");
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}


static void sendCommand(int fd, char* command, size_t commandLength);


/******************************************************************************/
int main (int argc, char *argv[]) {
    char *socketPath = "/tmp/CO_command_socket";  /* Name of the local domain socket, configurable by arguments. */
    char *inputFilePath = NULL;

    char buf[BUF_SIZE];
    int fd;
    struct sockaddr_un addr;
    int opt;
    int i;

    if(argc >= 2 && strcmp(argv[1], "--help") == 0) {
        usageExit(argv[0]);
    }

    /* Get program options */
    while((opt = getopt(argc, argv, "s:f:")) != -1) {
        switch (opt) {
            case 'f':
                inputFilePath = optarg;
                break;
            case 's':
                socketPath = optarg;
                break;
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


    /* get commands from input file, line after line */
    if(inputFilePath != NULL) {
        FILE *fp = fopen(inputFilePath, "r");
        if(fp == NULL) {
            errExit("Can't open input file");
        }

        while(fgets(buf, BUF_SIZE, fp) != NULL) {
            sendCommand(fd, buf, strlen(buf));
        }

        fclose(fp);
    }

    /* get command from arguments */
    else if(optind < argc) {
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
        }
    }

    close(fd);

    exit(EXIT_SUCCESS);
}


static void sendCommand(int fd, char* command, size_t commandLength) {
    size_t n;
    char buf[1000];

    if (write(fd, command, commandLength) != commandLength) {
        errExit("Socket write failed");
    }

    n = read(fd, buf, sizeof(buf));

    if(n == -1) {
        errExit("Socket read failed");
    }

    printf("%s", buf);
}


/*
 * Client socket command interface for CANopenSocket.
 *
 * @file        CANopenCommand.c
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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>// inet_aton
#include <limits.h>


#ifndef BUF_SIZE
#define BUF_SIZE            1000000
#endif

/* Helper functions */
void errExit(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}


static int printErrorDescription = 0;

static void sendCommand(int fd, char* command, size_t commandLength);


static void printUsage(char *progName) {
fprintf(stderr,
"Usage: %s [options] <command string>\n", progName);
fprintf(stderr,
"\n"
"Program reads arguments from standard input or file. It sends commands to\n"
"canopend via socket, line after line. Result is printed to standard output.\n"
"Socket is either unix domain socket (default) or a remote tcp socket (option -t)."
"For more information see http://www.can-cia.org/, CiA 309 standard.\n"
"\n"
"Options:\n"
"  -f <input file path>  Path to the input file.\n"
"  -s <socket path>      Path to the socket (default '/tmp/CO_command_socket').\n"
"  -t <host>             Connect via tcp to remote <host>.\n"
"  -p <port>             Tcp port to connect to when using -t. Defaults to 60000\n"
"  -h                    Display description of error codes in case of error.\n"
"                        (Default, if command is passed by program arguments.)\n"
"  --help                Display this help.\n"
"  --helpall             Display this help, internal and SDO error codes.\n"
"\n"
"Command strings must start with \"[\"<sequence>\"]\" (except if from arguments):\n"
"  - SDO upload:   [[<net>] <node>] r[ead]  <index> <subindex> [<datatype>]\n"
"  - SDO download: [[<net>] <node>] w[rite] <index> <subindex>  <datatype> <value>\n"
"  - Configure SDO time-out: [<net>] set sdo_timeout <value>\n"
"  - Enable SDO block transfer: [<net>] set sdo_block <value>\n"
"  - Set default node: [<net>] set node <value>\n"
"\n"
"  - Start node:                  [[<net>] <node>] start\n"
"  - Stop node:                   [[<net>] <node>] stop\n"
"  - Set node to pre-operational: [[<net>] <node>] preop[erational]\n"
"  - Reset node:                  [[<net>] <node>] reset node\n"
"  - Reset communication:         [[<net>] <node>] reset comm[unication]\n"
"\n"
"Comments started with '#' are ignored. They may be on the beginning of the line\n"
"or after the command string. 'sdo_timeout' is in milliseconds, 500 by default.\n"
"If <node> is not specified within commands, then value defined by 'set node'\n"
"command is used.\n"
"\n"
"\n"
"Datatypes:\n"
"  - b                 - Boolean.\n"
"  - u8, u16, u32, u64 - Unsigned integers.\n"
"  - i8, i16, i32, i64 - Signed integers.\n"
"  - r32, r64          - Real numbers.\n"
"  - t, td             - Time of day, time difference.\n"
"  - vs                - Visible string (between double quotes).\n"
"  - os, us, d         - Octet string, unicode string, domain\n"
"                        (mime-base64 (RFC2045) should be used).\n"
"\n"
"\n"
"Response: \"[\"<sequence>\"]\" \\\n"
"    OK | <value> | ERROR: <SDO-abort-code> | ERROR: <internal-error-code>\n"
"\n"
"\n"
"See also: https://github.com/CANopenNode/CANopenSocket\n"
"\n"
);
}


/* Extract error description */
typedef struct {
    int code;
    char* desc;
} errorDescs_t;

static const errorDescs_t errorDescs[] = {
        {100, "Request not supported."},
        {101, "Syntax error."},
        {102, "Request not processed due to internal state."},
        {103, "Time-out (where applicable)."},
        {104, "No default net set."},
        {105, "No default node set."},
        {106, "Unsupported net."},
        {107, "Unsupported node."},
        {200, "Lost guarding message."},
        {201, "Lost connection."},
        {202, "Heartbeat started."},
        {203, "Heartbeat lost."},
        {204, "Wrong NMT state."},
        {205, "Boot-up."},
        {300, "Error passive."},
        {301, "Bus off."},
        {303, "CAN buffer overflow."},
        {304, "CAN init."},
        {305, "CAN active (at init or start-up)."},
        {400, "PDO already used."},
        {401, "PDO length exceeded."},
        {501, "LSS implementation- / manufacturer-specific error."},
        {502, "LSS node-ID not supported."},
        {503, "LSS bit-rate not supported."},
        {504, "LSS parameter storing failed."},
        {505, "LSS command failed because of media error."},
        {600, "Running out of memory."},
        {0x00000000, "No abort."},
        {0x05030000, "Toggle bit not altered."},
        {0x05040000, "SDO protocol timed out."},
        {0x05040001, "Command specifier not valid or unknown."},
        {0x05040002, "Invalid block size in block mode."},
        {0x05040003, "Invalid sequence number in block mode."},
        {0x05040004, "CRC error (block mode only)."},
        {0x05040005, "Out of memory."},
        {0x06010000, "Unsupported access to an object."},
        {0x06010001, "Attempt to read a write only object."},
        {0x06010002, "Attempt to write a read only object."},
        {0x06020000, "Object does not exist."},
        {0x06040041, "Object cannot be mapped to the PDO."},
        {0x06040042, "Number and length of object to be mapped exceeds PDO length."},
        {0x06040043, "General parameter incompatibility reasons."},
        {0x06040047, "General internal incompatibility in device."},
        {0x06060000, "Access failed due to hardware error."},
        {0x06070010, "Data type does not match, length of service parameter does not match."},
        {0x06070012, "Data type does not match, length of service parameter too high."},
        {0x06070013, "Data type does not match, length of service parameter too short."},
        {0x06090011, "Sub index does not exist."},
        {0x06090030, "Invalid value for parameter (download only)."},
        {0x06090031, "Value range of parameter written too high."},
        {0x06090032, "Value range of parameter written too low."},
        {0x06090036, "Maximum value is less than minimum value."},
        {0x060A0023, "Resource not available: SDO connection."},
        {0x08000000, "General error."},
        {0x08000020, "Data cannot be transferred or stored to application."},
        {0x08000021, "Data cannot be transferred or stored to application because of local control."},
        {0x08000022, "Data cannot be transferred or stored to application because of present device state."},
        {0x08000023, "Object dictionary not present or dynamic generation fails."},
        {0x08000024, "No data available."}
};

static void printErrorDescs(void) {
    int i, len;

    len = sizeof(errorDescs) / sizeof(errorDescs_t);

    fprintf(stderr, "Internal error codes:\n");

    for(i=0; i<len; i++) {
        const errorDescs_t *ed = &errorDescs[i];

        if(ed->code == 0) break;
        fprintf(stderr, "  - %d - %s\n", ed->code, ed->desc);
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "SDO abort codes:\n");

    for(; i<len; i++) {
        const errorDescs_t *ed = &errorDescs[i];

        fprintf(stderr, "  - 0x%08X - %s\n", ed->code, ed->desc);
    }

    fprintf(stderr, "\n");
}

/******************************************************************************/
int main (int argc, char *argv[]) {
    char *socketPath = "/tmp/CO_command_socket";  /* Name of the local domain socket, configurable by arguments. */
    char *inputFilePath = NULL;
    sa_family_t addrFamily = AF_UNIX;
    char  tcpPort[20] = "60000"; /* default port when used in tcp mode */

    char* buf;
    size_t bufSize = sizeof *buf * BUF_SIZE;
    buf = malloc(bufSize);
    char hostname[HOST_NAME_MAX];
    int fd;
    struct sockaddr_un addr_un;
    int opt;
    int i;

    if(argc >= 2 && strcmp(argv[1], "--help") == 0) {
        printUsage(argv[0]);
        free(buf);
        exit(EXIT_SUCCESS);
    }
    if(argc >= 2 && strcmp(argv[1], "--helpall") == 0) {
        printUsage(argv[0]);
        printErrorDescs();
        free(buf);
        exit(EXIT_SUCCESS);
    }

    /* Get program options */
    while((opt = getopt(argc, argv, "s:f:hp:t:")) != -1) {
        switch (opt) {
            case 'f':
                inputFilePath = optarg;
                break;
            case 's':
                socketPath = optarg;
                break;
            case 'h':
                printErrorDescription = 1;
                break;
            case 't':
                addrFamily = AF_INET;
                strncpy(hostname, optarg, sizeof(hostname));
                break;
            case 'p':
                strncpy(tcpPort, optarg, sizeof(tcpPort));
                break;
            default:
                printUsage(argv[0]);
                free(buf);
                exit(EXIT_FAILURE);
        }
    }

    /* Create and connect client socket */
    if(addrFamily == AF_INET) {

        struct addrinfo hints, *res, *rp;
        int errcode;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags |= AI_CANONNAME;

        errcode = getaddrinfo(hostname, tcpPort, &hints, &res);
        if (errcode != 0) {
            fprintf(stderr, "Error! Getaddrinfo for host %s failed\n", hostname);
            free(buf);
            exit(EXIT_FAILURE);
        }

      /* getaddrinfo() returns a list of address structures.
         Try each address until we successfully connect.
         If socket (or connect) fails, we (close the socket
         and) try the next address. */

      for (rp = res; rp != NULL; rp = rp->ai_next) {
          fd = socket(rp->ai_family, rp->ai_socktype,
                       rp->ai_protocol);
          if (fd == -1) {
              continue;
          }

          if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1) {
              break;                  /* Success */
          }

          close(fd);
          free(buf);
          errExit("Socket connection failed");
      }
    }
    else { // addrFamily == AF_UNIX
      fd = socket(AF_UNIX, SOCK_STREAM, 0);
      if(fd == -1) {
          free(buf);
          errExit("Socket creation failed");
      }

      memset(&addr_un, 0, sizeof(struct sockaddr_un));
      addr_un.sun_family = addrFamily;
      strncpy(addr_un.sun_path, socketPath, sizeof(addr_un.sun_path) - 1);

      if(connect(fd, (struct sockaddr *)&addr_un, sizeof(struct sockaddr_un)) == -1) {
          free(buf);
          errExit("Socket connection failed");
      }
    }

    printErrorDescription = 1; // always print error description


    /* get commands from input file, line after line */
    if(inputFilePath != NULL) {
        FILE *fp = fopen(inputFilePath, "r");
        if(fp == NULL) {
            free(buf);
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

        /* Add sequence number if not present on command line arguments */
        if(argv[optind][0] != '[') {
            strcat(buf, "[1] ");
        }

        for(i=optind; i<argc; i++) {
            strncat(buf, argv[i], (BUF_SIZE - 2) - buflen);
            strcat(buf, " ");
            buflen = strlen(buf);
            if(buflen >= (BUF_SIZE - 1)) {
                fprintf(stderr, "String too long!\n");
                free(buf);
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

    free(buf);
    exit(EXIT_SUCCESS);
}


static void sendCommand(int fd, char* command, size_t commandLength) {
    size_t n;

    char* replyBuf;
    size_t bufSize = sizeof *replyBuf * BUF_SIZE;
    replyBuf = malloc(bufSize);

    // send command
    if (write(fd, command, commandLength) != commandLength) {
        errExit("Socket write failed");
    }

    //read reply
    n = read(fd, replyBuf, bufSize - 1);

    if(n < 0) {
        errExit("Socket read failed");
    }
    else {
        replyBuf[n] = 0;
    }

    if(printErrorDescription == 1) {
        //check for error reply
        char *errLoc = strstr(replyBuf, "ERROR:");
        char *endLoc = strstr(replyBuf, "\r\n");

        if(errLoc != NULL && endLoc != NULL) {
            //parse error code
            int num;
            char *sRet = NULL;

            errLoc += 6;

            num = strtol(errLoc, &sRet, 0);
            if(strlen(errLoc) != 0 && sRet == strchr(errLoc, '\r')) {
                int i, len;

                len = sizeof(errorDescs) / sizeof(errorDescs_t);

                //lookup error code and print
                for(i=0; i<len; i++) {
                    const errorDescs_t *ed = &errorDescs[i];
                    if(ed->code == num) {
                        sprintf(endLoc, " - %s\r\n", ed->desc);
                        break;
                    }
                }
            }
        }
    }

    printf("%s", replyBuf);
    free(replyBuf);
}


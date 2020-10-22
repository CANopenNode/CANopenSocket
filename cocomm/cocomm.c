/*
 * Client socket command interface for CANopenSocket.
 *
 * @file        cocomm.c
 * @author      Janez Paternoster
 * @copyright   2020 Janez Paternoster
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
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <limits.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/can.h>

#ifndef BUF_SIZE
#define BUF_SIZE 1000
#endif

#define STRINGIFY(x) #x
#define TOSTR(x) STRINGIFY(x)

/* configurable options */
static int printErrorDescription = 1; /* info text on command failure */
static size_t commBufSize = BUF_SIZE; /* max size of command receive and response buffer */

static int sendCommand(int fd_gtw, char* command, size_t commandLength);

static void printUsage(char *progName) {
fprintf(stderr,
"Usage: %s [options] ['<command string>' ['<command string>'] ...]\n", progName);
fprintf(stderr,
"\n"
"Program reads CANopen gateway command strings from arguments, standard input or\n"
"file. It sends commands to canopend via socket, line after line. Result is\n"
"printed to standard output. Command strings from arguments must be quoted.\n"
"Socket is either unix domain socket (default) or a remote tcp socket\n"
"(option -t). For more information see http://www.can-cia.org/, CiA 309 standard.\n"
"\n"
"Options:\n"
"  -f <input file>  Path to the input file.\n"
"  -s <socket path> Path to the unix socket. If not specified, path is obtained\n"
"                   from  environmental variable, configured with:\n"
"                   'export cocomm_socket=<socket path>'. If latter is not\n"
"                   specified, default value is used: '/tmp/CO_command_socket'.\n"
"  -t <host>        Connect via tcp to remote <host>. Set also with\n"
"                   'export cocomm_host=<host>'. Unix socket is used by default.\n"
"  -p <port>        Tcp port to connect to when using -t. Set also with\n"
"                   'export cocomm_port=<port>'. Default is 60000.\n"
"  -h <0|1>         Display description of error codes in case of error. Set also\n"
"                   with 'export cocomm_err_desc=<0|1>'. Default is 1.\n"
"  -b <buffer size> Buffer size for command and response. Set also with\n"
"                   'export cocomm_buff_size=<buffer size>'. Default is "TOSTR(BUF_SIZE)".\n"
"  -d <can device>  If specified, then candump of specified CAN device will be\n"
"                   printed after the command response. Set also with\n"
"                   'export cocomm_candump=<can device>'. Not used by default.\n"
"  -n <count>       Print <count> of candump messages, then exit. Set also with\n"
"                   'export cocomm_candump_count=<count>'. Default is 10.\n"
"  -T <msec>        Exit candump after <msec> without reception. Set also with\n"
"                   'export cocomm_candump_timeout=<msec>'. Default is 1000.\n"
"  --help           Display this help.\n"
"  --helpall        Display this help, internal and SDO error codes.\n"
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
    /* configurable options */
    char *inputFilePath = NULL;
    char *socketPath = "/tmp/CO_command_socket"; /* Name of the local domain socket */
    char hostname[HOST_NAME_MAX]; /* name of the remote TCP host */
    char tcpPort[20] = "60000"; /* default port when used in tcp mode */
    char *candump = NULL;
    long candumpCount = 10;
    long candumpTmo = 1000;

    char* commBuf;
    int fd_gtw;
    int fd_candump;
    int opt;
    struct sockaddr_un addr_un;
    sa_family_t addrFamily = AF_UNIX;

    if(argc >= 2 && strcmp(argv[1], "--help") == 0) {
        printUsage(argv[0]);
        exit(EXIT_SUCCESS);
    }
    if(argc >= 2 && strcmp(argv[1], "--helpall") == 0) {
        printUsage(argv[0]);
        printErrorDescs();
        exit(EXIT_SUCCESS);
    }

    /* Get program options from environment variables */
    char *env;
    if ((env = getenv("cocomm_host")) != NULL) {
        strncpy(hostname, env, sizeof(hostname));
        addrFamily = AF_INET;
    }
    if ((env = getenv("cocomm_port")) != NULL) {
        strncpy(tcpPort, env, sizeof(tcpPort));
    }
    if ((env = getenv("cocomm_socket")) != NULL) {
        socketPath = env;
        addrFamily = AF_UNIX;
    }
    if ((env = getenv("cocomm_err_desc")) != NULL) {
        printErrorDescription = atoi(env);
    }
    if ((env = getenv("cocomm_buff_size")) != NULL) {
        commBufSize = atoi(env);
    }
    if ((env = getenv("cocomm_candump")) != NULL) {
        candump = env;
    }
    if ((env = getenv("cocomm_candump_count")) != NULL) {
        candumpCount = atol(env);
    }
    if ((env = getenv("cocomm_candump_timeout")) != NULL) {
        candumpTmo = atol(env);
    }

    /* Get program options from arguments */
    while((opt = getopt(argc, argv, "f:s:t:p:h:b:d:n:T:")) != -1) {
        switch (opt) {
            case 'f':
                inputFilePath = optarg;
                break;
            case 's':
                addrFamily = AF_UNIX;
                socketPath = optarg;
                break;
            case 't':
                addrFamily = AF_INET;
                strncpy(hostname, optarg, sizeof(hostname));
                break;
            case 'p':
                strncpy(tcpPort, optarg, sizeof(tcpPort));
                break;
            case 'h':
                printErrorDescription = atoi(optarg);
                break;
            case 'b':
                commBufSize = atoi(optarg);
                break;
            case 'd':
                candump = optarg;
                break;
            case 'n':
                candumpCount = atol(optarg);
                break;
            case 'T':
                candumpTmo = atol(optarg);
                break;
            default:
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (commBufSize < 100) {
        fprintf(stderr, "Error! To small buffer size: %lu\n", commBufSize);
        exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
        }

      /* getaddrinfo() returns a list of address structures.
         Try each address until we successfully connect.
         If socket (or connect) fails, we (close the socket
         and) try the next address. */

      for (rp = res; rp != NULL; rp = rp->ai_next) {
          fd_gtw = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
          if (fd_gtw == -1) {
              continue;
          }

          if (connect(fd_gtw, rp->ai_addr, rp->ai_addrlen) != -1) {
              break; /* Success */
          }

          close(fd_gtw);
          perror("Socket connection failed");
          exit(EXIT_FAILURE);
      }
    }
    else { // addrFamily == AF_UNIX
      fd_gtw = socket(AF_UNIX, SOCK_STREAM, 0);
      if(fd_gtw == -1) {
          perror("Socket creation failed");
          exit(EXIT_FAILURE);
      }

      memset(&addr_un, 0, sizeof(struct sockaddr_un));
      addr_un.sun_family = addrFamily;
      strncpy(addr_un.sun_path, socketPath, sizeof(addr_un.sun_path) - 1);

      if(connect(fd_gtw, (struct sockaddr *)&addr_un, sizeof(struct sockaddr_un)) == -1) {
          fprintf(stderr, "Socket connection failed \"%s\": ", socketPath);
          perror(NULL);
          exit(EXIT_FAILURE);
      }
    }

    /* Prepare candump */
    if (candump != NULL && candumpCount > 0 && candumpTmo > 0) {
        struct sockaddr_can sockAddr;

        fd_candump = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (fd_candump < 0) {
            perror("CAN socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct timeval tv;
        tv.tv_sec = candumpTmo / 1000;
        tv.tv_usec = (candumpTmo % 1000) * 1000;
        setsockopt(fd_candump, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.can_family = AF_CAN;
        sockAddr.can_ifindex = if_nametoindex(candump);
        int ret = bind(fd_candump, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
        if (ret < 0) {
            fprintf(stderr, "CAN Socket binding failed \"%s\": ", candump);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
    }
    else {
        candumpCount = 0;
    }

    /* get commands from input file, line after line */
    commBuf = malloc(commBufSize);
    int ret = EXIT_SUCCESS;
    if(inputFilePath != NULL) {
        FILE *fp = fopen(inputFilePath, "r");
        if(fp == NULL) {
            free(commBuf);
            perror("Can't open input file");
            exit(EXIT_FAILURE);
        }

        while(fgets(commBuf, commBufSize, fp) != NULL) {
            if (sendCommand(fd_gtw, commBuf, strlen(commBuf)) == EXIT_FAILURE) {
                ret = EXIT_FAILURE;
            }

        }

        fclose(fp);
    }

    /* get command from arguments */
    else if(optind < argc) {
        for(int i = optind; i < argc; i++) {
            char *comm = argv[i];
            commBuf[0] = 0;

            /* Add sequence number if not present on command line argument */
            if(argv[optind][0] != '[') {
                sprintf(commBuf, "[%d] ", i - optind + 1);
            }

            if((strlen(commBuf) + strlen(comm)) >= (commBufSize - 2)) {
                fprintf(stderr, "Command string too long!\n");
                free(commBuf);
                exit(EXIT_FAILURE);
            }

            strcat(commBuf, comm);
            strcat(commBuf, "\n");

            if (sendCommand(fd_gtw, commBuf, strlen(commBuf)) == EXIT_FAILURE) {
                ret = EXIT_FAILURE;
            }
        }
    }

    /* get commands from stdin, line after line */
    else {
        while(fgets(commBuf, commBufSize, stdin) != NULL) {
            if (sendCommand(fd_gtw, commBuf, strlen(commBuf)) == EXIT_FAILURE) {
                ret = EXIT_FAILURE;
            }
        }
    }

    close(fd_gtw);

    free(commBuf);

    if (ret == EXIT_SUCCESS && candumpCount > 0) {
        for (int i = 0; i < candumpCount; i++) {
            const char hex_asc[] = "0123456789ABCDEF";
            struct can_frame canFrame;

            ssize_t nbytes = read(fd_candump, &canFrame, sizeof(struct can_frame));

            if (nbytes < (ssize_t)sizeof(struct can_frame)) {
                perror("CAN raw socket read timeout");
                exit(EXIT_FAILURE);
            }

            char buf[17];
            char* pbuf = &buf[0];
            for (int i = 0; i < canFrame.can_dlc && i < sizeof(buf); i++) {
                uint8_t byte = canFrame.data[i];
                *pbuf++ = hex_asc[byte >> 4];
                *pbuf++ = hex_asc[byte & 0x0F];
            }
            *pbuf = 0;

            printf ("%03X#%s\n", canFrame.can_id, buf);
        }
        close(fd_candump);
    }

    exit(ret);
}


static int sendCommand(int fd_gtw, char* command, size_t commandLength) {
    size_t n;

    char* replyBuf;
    replyBuf = malloc(commBufSize);

    // send command
    if (write(fd_gtw, command, commandLength) != commandLength) {
        perror("Socket write failed");
        exit(EXIT_FAILURE);
    }

    //read reply
    n = read(fd_gtw, replyBuf, commBufSize - 1);

    if(n < 0) {
        perror("Socket read failed");
        exit(EXIT_FAILURE);
    }
    else {
        replyBuf[n] = 0;
    }

    //check for error reply
    char *errLoc = strstr(replyBuf, "ERROR:");

    if(printErrorDescription == 1 && errLoc != NULL) {
        char *endLoc = strstr(replyBuf, "\r\n");

        if(endLoc != NULL) {
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

    return errLoc == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}

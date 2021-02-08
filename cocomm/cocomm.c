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
#include <bits/getopt_core.h>
#include <string.h>
#include <netdb.h>
#include <limits.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/can.h>
#include <signal.h>

#ifndef BUF_SIZE
#define BUF_SIZE 1000
#endif
#define BUF_LAG 100 /* max size of error response */

/* colors and stream for printing status */
char *greenC, *redC, *resetC;
FILE *errStream;


static void printUsage(char *progName) {
fprintf(errStream,
"Usage: %s [options] ['<command string>' ['<command string>'] ...]\n"
"\n"
"Program reads CANopen gateway command strings from arguments, standard input or\n"
"file. It sends commands to canopend via socket, line after line. Response status\n"
"is printed to standard error and response data (for example, value from write\n"
"command) is printed to standard output. Command strings from arguments must\n"
"be quoted. Socket is either unix domain socket (default) or a remote tcp socket\n"
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
"  -i               If set, then standard input will be read after each command\n"
"                   string from arguments. Useful with write commands.\n"
"  -o all|data|flat By defult (setting 'all') outupt is split to colored stderr\n"
"                   and stdout. 'data' prints data only to stdout. 'flat' prints\n"
"                   all to stdout, set also with 'export cocomm_flat=<0|1>'.\n"
"  -d <can device>  If specified, then candump of specified CAN device will be\n"
"                   printed after the command response. Set also with\n"
"                   'export cocomm_candump=<can device>'. Not used by default.\n"
"  -n <count>       Print <count> of candump messages, then exit. Set also with\n"
"                   'export cocomm_candump_count=<count>'. Default is 10.\n"
"  -T <msec>        Exit candump after <msec> without reception. Set also with\n"
"                   'export cocomm_candump_timeout=<msec>'. Default is 1000.\n"
"  --help           Display this help.\n"
"\n"
"For help on command strings type '%s \"help\"'.\n"
"\n"
"See also: https://github.com/CANopenNode/CANopenSocket\n"
"\n", progName, progName);
}

/* print reply, status to errStream (red or green), value to stdout */
static int printReply(int fd_gtw) {
    char* replyBuf = malloc(BUF_SIZE + BUF_LAG + 1);
    size_t count = 0; /* count of bytes in replyBuf */
    int firstPass = 1;
    int ret = EXIT_SUCCESS;

    if(replyBuf == NULL) {
        perror("replyBuf malloc");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        ssize_t nRead = read(fd_gtw, &replyBuf[count], BUF_SIZE); /* blocking*/

        if(nRead > 0) {
            count += nRead;
            replyBuf[count] = 0;

            if (firstPass == 1) {
                /* check for response type. Only response value goes to stdout*/
                if (strstr(replyBuf, "] ERROR:") != NULL
                    && strstr(replyBuf, "\r\n") != NULL
                ) {
                    fprintf(errStream, "%s%s%s", redC, replyBuf, resetC);
                    ret = EXIT_FAILURE;
                    break;
                }
                else if (strstr(replyBuf, "] OK\r\n") != NULL) {
                    fprintf(errStream, "%s%s%s", greenC, replyBuf, resetC);
                    break;
                }
                else {
                    char *replyBufTrimmed = replyBuf;
                    char *seq = strstr(replyBuf, "] ");
                    char *end = strstr(replyBuf, "\r\n");
                    if (seq != NULL && (size_t)(seq - replyBuf) < 15) {
                        replyBufTrimmed = seq + 2;
                        seq[1] = 0;
                        count -= strlen(replyBuf) + 1;
                        fprintf(errStream, "%s%s%s ", greenC, replyBuf, resetC);
                    }
                    if (end != NULL) {
                        end[0] = 0;
                        fputs(replyBufTrimmed, stdout);
                        fflush(stdout);
                        fputs("\r\n", errStream);
                        break;
                    }
                    else {
                        /* print replyBuf to stdout, except last BUF_LAG bytes.
                         * move them to the beginning of the replyBuf */
                        if (count > BUF_LAG) {
                            size_t nWrite = count - BUF_LAG;
                            fwrite(replyBufTrimmed, 1, nWrite, stdout);
                            replyBufTrimmed += nWrite;
                            count = BUF_LAG;
                        }
                        memmove(replyBuf, replyBufTrimmed, count);
                    }
                }
                firstPass = 0;
            }
            else {
                char *end = strstr(replyBuf, "\r\n");
                if (end != NULL) {
                    char *errResp = strstr(replyBuf, "\n...ERROR:0x");
                    if (errResp != NULL) {
                        errResp[0] = 0;
                        fputs(replyBuf, stdout);
                        fflush(stdout);
                        fprintf(errStream, "\n%s%s%s", redC, &errResp[1], resetC);
                        ret = EXIT_FAILURE;
                    }
                    else {
                        end[0] = 0;
                        fputs(replyBuf, stdout);
                        fflush(stdout);
                        fprintf(errStream, "\n%s...success%s\r\n", greenC, resetC);
                    }
                    break;
                }
                /* print replyBuf to stdout, except last BUF_LAG bytes.
                 * move them to the beginning of the replyBuf */
                if (count > BUF_LAG) {
                    size_t nWrite = count - BUF_LAG;
                    fwrite(replyBuf, 1, nWrite, stdout);
                    count = BUF_LAG;
                    memmove(replyBuf, &replyBuf[nWrite], count);
                }
            }
        }
        else if (nRead == 0) {
            fprintf(errStream, "%sError, zero response%s\n", redC, resetC);
            ret = EXIT_FAILURE;
            break;
        }
        else {
            perror("Socket read failed");
            free(replyBuf);
            exit(EXIT_FAILURE);
        }
        fflush(stdout);
    }
    free(replyBuf);

    return ret;
}


/******************************************************************************/
int main (int argc, char *argv[]) {
    /* configurable options */
    enum {out_all, out_data, out_flat} outputType = out_all;
    char *inputFilePath = NULL;
    char *socketPath = "/tmp/CO_command_socket"; /* Name of the local domain socket */
    char hostname[HOST_NAME_MAX]; /* name of the remote TCP host */
    char tcpPort[20] = "60000"; /* default port when used in tcp mode */
    int additionalReadStdin = 0;
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
    if ((env = getenv("cocomm_flat")) != NULL) {
        if (strcmp(env, "1") == 0) {
            outputType = out_flat;
        }
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
    while((opt = getopt(argc, argv, "f:s:t:p:io:d:n:T:")) != -1) {
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
            case 'i':
                additionalReadStdin = 1;
                break;
            case 'o':
                if (strcmp(optarg, "data") == 0)
                    outputType = out_data;
                else if (strcmp(optarg, "flat") == 0)
                    outputType = out_flat;
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

    switch (outputType) {
        default:
        case out_all:
            greenC = "\033[32m";
            redC = "\033[31m";
            resetC = "\033[0m";
            errStream = stderr;
            break;
        case out_data:
            greenC = redC = resetC = "";
            errStream = fopen("/dev/null", "w+");
            break;
        case out_flat:
            greenC = redC = resetC = "";
            errStream = stdout;
            break;
    }

    /* Ignore the SIGPIPE signal, which may happen, if gateway broke
     * the connection. Program will exit with error message anyway.
     * Signal may be triggered by socket write call. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
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

        /* getaddrinfo() returns a list of address structures. Try each address
         * until we successfully connect. If socket (or connect) fails,
         * we (close the socket and) try the next address. */

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
    int ret = EXIT_SUCCESS;

    commBuf = malloc(BUF_SIZE);
    if(commBuf == NULL) {
        perror("commBuf malloc");
        exit(EXIT_FAILURE);
    }

    if(inputFilePath != NULL) {
        FILE *fp = fopen(inputFilePath, "r");
        if(fp == NULL) {
            perror("Can't open input file");
            free(commBuf);
            exit(EXIT_FAILURE);
        }

        while(fgets(commBuf, BUF_SIZE, fp) != NULL) {
            size_t len = strlen(commBuf);
            if (len < 1) continue;

            // send command
            if (write(fd_gtw, commBuf, len) != len) { /* blocking function */
                perror("Socket write failed");
                free(commBuf);
                exit(EXIT_FAILURE);
            }

            // print reply, if command is complete
            if (commBuf[len - 1] == '\n') {
                if (printReply(fd_gtw) == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                }
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
            if(comm[0] != '[' && comm[0] != '#') {
                sprintf(commBuf, "[%d] ", i - optind + 1);
            }

            if((strlen(commBuf) + strlen(comm)) >= (BUF_SIZE - 2)) {
                fprintf(errStream, "%sCommand string too long!%s\n", redC, greenC);
                continue;
            }

            strcat(commBuf, comm);

            if (additionalReadStdin == 0) {
                strcat(commBuf, "\n");
                size_t len = strlen(commBuf);
                if (write(fd_gtw, commBuf, len) != len) { /* blocking function */
                    perror("Socket write failed");
                    free(commBuf);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                strcat(commBuf, " ");
                size_t len = strlen(commBuf);
                char lastChar;

                do {
                    if (fgets(commBuf+len, BUF_SIZE-1-len, stdin) == NULL)
                        strcat(commBuf, "\n");

                    len = strlen(commBuf);
                    lastChar = commBuf[len - 1];

                    if (len < BUF_SIZE-2 && lastChar != '\n')
                        continue;

                    // send command
                    if (write(fd_gtw, commBuf, len) != len) { /* blocking f. */
                        perror("Socket write failed");
                        free(commBuf);
                        exit(EXIT_FAILURE);
                    }

                    commBuf[0] = 0;
                    len = 0;
                } while (lastChar != '\n');
            }

            if (printReply(fd_gtw) == EXIT_FAILURE) {
                ret = EXIT_FAILURE;
            }
        }
    }

    /* get commands from stdin, line after line */
    else {
        while(fgets(commBuf, BUF_SIZE, stdin) != NULL) {
            size_t len = strlen(commBuf);
            if (len < 1) continue;

            // send command
            if (write(fd_gtw, commBuf, len) != len) { /* blocking function */
                perror("Socket write failed");
                free(commBuf);
                exit(EXIT_FAILURE);
            }

            // print reply, if command is complete
            if (commBuf[len - 1] == '\n') {
                if (printReply(fd_gtw) == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                }
            }
        }
    }

    close(fd_gtw);

    free(commBuf);

    /* candump output */
    if (candumpCount > 0) {
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
            for (int j = 0; j < canFrame.can_dlc && j < sizeof(buf); j++) {
                uint8_t byte = canFrame.data[j];
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

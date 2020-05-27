/*
 * Client socket command interface (Apache CGI) for CANopenSocket.
 *
 * @file        CANopenCGI.c
 * @author      Janez Paternoster
 * @copyright   2016 - 2020 Janez Paternoster
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
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <fnmatch.h>
#include <pwd.h>
#include <grp.h>
#include <sys/un.h>
#include <sys/socket.h>


#ifndef BUF_SIZE
#define BUF_SIZE            1000000
#endif


/* Helper functions */
static void errExitErrno(char* msg) {
    printf("%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

static void errExitErrnoUGID(char* msg) {
    struct passwd *user;
    struct group *grp;

    printf("%s: %s\n", msg, strerror(errno));

    user = getpwuid(getuid());
    grp = getgrgid(getgid());

    printf("Access from user: %s, group: %s\n",
        (user == NULL) ? NULL : user->pw_name,
        (grp == NULL) ? NULL : grp->gr_name);

    exit(EXIT_FAILURE);
}

static void errExit(char* msg) {
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

static void strcpyToUpper(char *dest, const char *src) {
    char in;

    do {
        in = *(src++);
        *(dest++) = toupper(in);
    } while(in != 0);
}

static void strToUpper(char *str) {
    char c;

    do {
        c = *(str);
        *(str++) = toupper(c);
    } while(c != 0);
}

static void strToLower(char *str) {
    char c;

    do {
        c = *(str);
        *(str++) = tolower(c);
    } while(c != 0);
}

/* Decode hex string 'str' of length 'len' and return numerical value.
 * In case of error in string, set 'err' to 1. */
static unsigned int hex2dec(const char *str, int len, int *err){
    unsigned int val = 0;
    int i;

    for(i=0; i<len; i++) {
        char c = str[i];
        if(c >= '0' && c <= '9') {
            c = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            c = c - ('A' - 10);
        }
        else {
            *err = 1;
            return 0;
        }
        val = val << 4 | c;
    }
    return val;
}

static void sendCommand(int fd, int sequence, char* command);

static void printUsage(void) {
printf(
"Usage: canopen.cgi?wnniiiissdd=xxxx[&rnniiiissdd=]\n"
"  - w    - One digit - 'W'rite or 'R'ead.\n"
"  - nn   - Two hex digits of node ID.\n"
"  - iiii - Four hex digits of Object Dictionary Index.\n"
"  - ss   - Two hex digits of Object Dictionary Subindex.\n"
"  - dd   - One to three digits of data type.\n"
"  - xxxx - Value to be written.\n"
"\n"
"Datatypes:\n"
"  - b                 - Boolean.\n"
"  - u8, u16, u32, u64 - Unsigned integers.\n"
"  - i8, i16, i32, i64 - Signed integers.\n"
"  - r32, r64          - Real numbers.\n"
"  - t, td             - Time of day, time difference.\n"
"  - vs                - Visible string (between double quotes).\n"
"  - os, us, d         - Octet string, unicode string, domain."
);
}


/******************************************************************************/
int main (int argc, char *argv[], char *env[]) {
    char socketPath[260] = {0};  /* Name of the local domain socket. */

    FILE *fp;
    int fdSocket;
    struct sockaddr_un addr;
    char *queryString;
    int queryStringAllocated = 0;

    /* whitelist and blacklist are arrays of null separated strings, which
     * contains patterns for comparision with commands from query string. */
    char *whitelist;
    char *blacklist;
    int whitelistLen;
    int blacklistLen;


    /* Print mime */
    printf("Content-type:text/plain\n\n");


    /* Get program options from configuration file */
    fp = fopen("canopen.conf", "r");
    if(fp == NULL) {
        errExitErrno("Can't open configuration file");
    }
    else {
        const char spaceDelim[] = " \t\n\r\f\v";
        char buf[1000];
        int wlSize = 1000;   /* byte length */
        int blSize = 1000;
        int wlDataSize = 0;
        int blDataSize = 0;

        whitelist = (char *) malloc(wlSize);
        blacklist = (char *) malloc(blSize);;
        whitelistLen = 0;   /* number of tokens in list */
        blacklistLen = 0;
        if(whitelist == NULL || blacklist == NULL) {
            errExitErrno("Whitelist or Blacklist can't be allocated.");
        }

        while(fgets(buf, sizeof(buf), fp) != NULL) {
            char *token;
            token = strtok(buf, spaceDelim);

            if(token == NULL) {

            }
            else if(strcasecmp(token, "socketPath") == 0) {
                if(strlen(socketPath) != 0) {
                    errExit("Duplicate 'socketPath' in canopen.conf.");
                }
                strncpy(socketPath, strtok(NULL, spaceDelim), sizeof(socketPath));
                socketPath[sizeof(socketPath)-1] = 0;
            }
            else if(strcasecmp(token, "allow") == 0) {
                int prevDataSize = wlDataSize;

                token = strtok(NULL, spaceDelim);
                wlDataSize += (strlen(token) + 1);
                while(wlDataSize > wlSize) {
                    wlSize *= 2;
                    whitelist = (char *) realloc(whitelist, wlSize);
                    if(whitelist == NULL) {
                        errExitErrno("Whitelist can't be allocated.");
                    }
                }
                strcpyToUpper(&whitelist[prevDataSize], token);
                whitelistLen ++;
            }
            else if(strcasecmp(token, "deny") == 0) {
                int prevDataSize = blDataSize;

                token = strtok(NULL, spaceDelim);
                blDataSize += (strlen(token) + 1);
                while(blDataSize > blSize) {
                    blSize *= 2;
                    blacklist = (char *) realloc(blacklist, blSize);
                    if(blacklist == NULL) {
                        errExitErrno("Blacklist can't be allocated.");
                    }
                }
                strcpyToUpper(&blacklist[prevDataSize], token);
                blacklistLen ++;
            }
        }
    }

    fclose(fp);


    /* Create and connect client socket */
    fdSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fdSocket == -1) {
        errExitErrno("Socket creation failed");
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);

    if(connect(fdSocket, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        errExitErrnoUGID("Socket connection failed");
    }


    /* get query string */
    queryString = getenv("QUERY_STRING"); /* HTTP GET method. */
    if(queryString != NULL && strlen(queryString) == 0) {
        queryString = malloc(BUF_SIZE);
        if(queryString == NULL) {
            errExitErrno("queryString can't be allocated.");
        }
        queryStringAllocated = 1;
        fgets(queryString, BUF_SIZE, stdin); /* HTTP POST method. */
    }
    if(queryString == NULL && argc >= 2) {
        queryString = argv[1];  /* If no query string, try first argument. */
    }


    /* get commands from query string */
    if(queryString != NULL && strlen(queryString) > 0) {
        char *command;
        int sequence = 1;

        /* put whole query string to upper case */
        strToUpper(queryString);

        command = strtok(queryString, "&");
        while(command != NULL) {
            int i;
            int offset;
            int passed = 0;

            /* Test whitelist and blacklist */
            offset = 0;
            for(i=0; i<whitelistLen; i++) {
                char *patern = &whitelist[offset];
                if(fnmatch(patern, command, 0) == 0) {
                    passed = 1;
                    break;
                }
                offset += strlen(patern) + 1;
            }
            if(passed == 1) {
                offset = 0;
                for(i=0; i<blacklistLen; i++) {
                    char *patern = &blacklist[offset];
                    if(fnmatch(patern, command, 0) == 0) {
                        passed = -1; /* not allowed */
                        break;
                    }
                    offset += strlen(patern) + 1;
                }
            }

            /* Send command or error message */
            if(strlen(command) < 9) {
                printf("? %s [%d] ERROR: 101 - Syntax error in command.\n", command, sequence);
            }
            else if(passed == 1) {
                sendCommand(fdSocket, sequence, command);
            }
            else {
                printf("%c %c%c%c%c%c%c%c%c [%d] ERROR: 100 - Access restriction, command %s.\n",
                    command[0], command[1], command[2], command[3], command[4],
                    command[5], command[6], command[7], command[8],
                    sequence, (passed==0)?"not on whitelist":" on blacklist");
            }

            command = strtok(NULL, "&");
            sequence ++;
        }
    }
    else {
        printUsage();
    }


    close(fdSocket);
    free(whitelist);
    free(blacklist);
    if(queryStringAllocated == 1) {
        free(queryString);
    }

    exit(EXIT_SUCCESS);
}


static void sendCommand(int fd, int sequence, char* command) {
    int i, err;
    char comm;
    unsigned int nodeId, idx, sidx;
    char dataType[4];
    char *value = "";

    char buf[BUF_SIZE];

    /* Parse command. It is at least 8 characters long. */
    err = 0;

    comm = command[0];
    if(comm != 'R' && comm != 'W') {
        err = 1;
    }

    nodeId = hex2dec(&command[1], 2, &err);
    if(nodeId < 1 || nodeId > 127) {
        err = 1;
    }

    idx = hex2dec(&command[3], 4, &err);
    sidx = hex2dec(&command[7], 2, &err);

    for(i=0; i<sizeof(dataType); i++) {
        char c = command[9+i];

        if(c == '=' || c == 0) {
            dataType[i] = 0;
            if(c == '=') {
                value = &command[10+i];
            }
            break;
        }
        dataType[i] = c;
    }
    if(i > 3) {
        err = 1;
        dataType[0] = 0;
    }
    if(strlen(value) > (sizeof(buf) - 50)) {
        err = 1;
    }

    /* Write command according to CiA309-3. */
    if(err == 0) {
        size_t wlen, rlen;

        strToLower(dataType);

        wlen = sprintf(buf, "[%d] 0x%02X %c 0x%04X 0x%02X %s %s\n",
                sequence, nodeId, tolower(comm), idx, sidx, dataType, value);

        if (write(fd, buf, wlen) != wlen) {
            errExit("Socket write failed");
        }

        rlen = read(fd, buf, sizeof(buf) - 1);

        if(rlen < 0) {
            errExit("Socket read failed");
        }
        else {
            buf[rlen] = 0;
        }

        printf("%c %02X%04X%02X %s",
            comm, nodeId, idx, sidx, buf);
    }
    else {
        printf("? %s [%d] ERROR: 101 - Syntax error in command.\n",
                    command, sequence);
    }
}

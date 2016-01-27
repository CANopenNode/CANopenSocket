/*
 * Client socket command interface (Apache CGI) for CANopenSocket.
 *
 * @file        CANopenCGI.c
 * @author      Janez Paternoster
 * @copyright   2016 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * CANopenNode is free and open source software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
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
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <fnmatch.h>
#include <sys/un.h>
#include <sys/socket.h>


/* Helper functions */
static void errExitErrno(char* msg) {
    printf("%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

static void errExit(char* msg) {
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

static void sendCommand(int fd, int sequence, char* command);

static int commandSuccessfull = 0;

static void printUsage(void) {
printf(
"Usage: canopen.cgi?wnniiiissll=xxxx[&rnniiiissll=]\n"
"\n"
"Where w(r), nn, iiii, ss, ll, xxxx are fixed width fields with the following meaning:\n"
"  w    - 'w'rite or 'r'ead.\n"
"  nn   - node ID in hex format.\n"
"  iiii - Object dictionary index in hex format.\n"
"  ss   - Object dictionary subindex in hex format.\n"
"  ll   - length of variable (1 to FFFFFFFF) in hex format. If reading, this value is ignored.\n"
"  xxxx - Value to be written in hex and little endian format. If reading, this value is ignored.\n"
);
}


/******************************************************************************/
int main (int argc, char *argv[], char *env[]) {
    char socketPath[260] = {0};  /* Name of the local domain socket. */

    FILE *fp;
    int fdSocket;
    struct sockaddr_un addr;
    char *queryString;

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
                strcpy(&whitelist[prevDataSize], token);
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
                strcpy(&blacklist[prevDataSize], token);
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
        errExitErrno("Socket connection failed");
    }


    /* get commands from query string */
    queryString = getenv("QUERY_STRING");
    if(queryString == NULL && argc >= 2) {
        queryString = argv[1];  /* If no query string, try first argument. */
    }
    if(queryString != NULL) {
        char *command;
        int sequence = 1;

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
            if(passed == 1) {
                sendCommand(fdSocket, sequence, command);
            }
            else {
                printf("[%d] ERROR: 100 - Access restriction, command %s.\n",
                    sequence, (passed==0)?"not on whitelist":" on blacklist");
            }

            command = strtok(NULL, "&");
            sequence ++;
        }
    }


    close(fdSocket);
    free(whitelist);
    free(blacklist);

    if(commandSuccessfull == 0) {
        printUsage();
    }

    exit(EXIT_SUCCESS);
}


static void sendCommand(int fd, int sequence, char* command) {
    size_t commandLength, n;
    char bufWrite[1000];
    char bufRead[1000];

    commandLength = strlen(command);
    
    if (write(fd, command, commandLength) != commandLength) {
        errExit("Socket write failed");
    }

    n = read(fd, bufRead, sizeof(bufRead));

    if(n == -1) {
        errExit("Socket read failed");
    }

    printf("%s", bufRead);

    commandSuccessfull = 1;
}

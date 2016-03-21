/*
 * Command interface for CANopenSocket - ASCII helper functions.
 *
 * @file        CO_comm_helpers.c
 * @author      Janez Paternoster
 * @copyright   2015 Janez Paternoster
 *
 * This file is part of CANopenSocket, a Linux implementation of CANopen
 * stack with master functionality. Project home page is
 * <https://github.com/CANopenNode/CANopenSocket>. CANopenSocket is based
 * on CANopenNode: <https://github.com/CANopenNode/CANopenNode>.
 *
 * CANopenSocket is free and open source software: you can redistribute
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


#include "CO_comm_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>


const char spaceDelim[] = " \t\n\r\f\v";


/* Storage for strtok_r. Functions here may be used within single thread only. */
static char *strtokSavePtr = NULL;


/******************************************************************************/
int dtpHex(char *strout, int stroutSize, char* bufSdo, int bufLen) {
    int i;
    char *out = strout;
    int len = 0;

    strout[0] = 0;

    for(i=0; i<bufLen; i++) {
        len += 3;
        if(len >= stroutSize) {
            break;
        }
        out += sprintf(out, "%02hhX ", bufSdo[i]);
    }

    len = strlen(strout);

    /* remove end space */
    if(len > 0) {
        strout[--len] = 0;
    }

    return len;
}

static int dtpStr(char *strout, int stroutSize, char* bufSdo, int bufLen) {
    int i, n = 0;

    /* A visible string with whitespace is enclosed with double quotes to denote it as
     * single argument of the command. If a double quote is used within the string, the quotes
     * are escaped by a second quotes, e.g. “Hello “”World””, CANopen is great”. */

    stroutSize -= 2;

    strout[n++] = '"';
    for(i=0; i<bufLen; i++) {
        char c = bufSdo[i];

        if(n >= stroutSize || c == 0) {
            break;
        }

        strout[n++] = c;
        if(c == '"') {
            strout[n++] = '"';
        }
    }
    strout[n++] = '"';
    strout[n] = '\0';

    return n;
}

static int dtpU8 (char *strout, int stroutSize, char* bufSdo, int bufLen) {uint8_t   num; memcpy(&num, bufSdo, 1);      return sprintf(strout, "0x%02hhX", num);}
static int dtpU16(char *strout, int stroutSize, char* bufSdo, int bufLen) {uint16_t  num; CO_memcpySwap2(&num, bufSdo); return sprintf(strout, "0x%04hX",  num);}
static int dtpU32(char *strout, int stroutSize, char* bufSdo, int bufLen) {uint32_t  num; CO_memcpySwap4(&num, bufSdo); return sprintf(strout, "0x%08X",   num);}
static int dtpU64(char *strout, int stroutSize, char* bufSdo, int bufLen) {uint64_t  num; CO_memcpySwap8(&num, bufSdo); return sprintf(strout, "0x%016lX", num);}
static int dtpI8 (char *strout, int stroutSize, char* bufSdo, int bufLen) {int8_t    num; memcpy(&num, bufSdo, 1);      return sprintf(strout, "%hhd",     num);}
static int dtpI16(char *strout, int stroutSize, char* bufSdo, int bufLen) {int16_t   num; CO_memcpySwap2(&num, bufSdo); return sprintf(strout, "%hd",      num);}
static int dtpI32(char *strout, int stroutSize, char* bufSdo, int bufLen) {int32_t   num; CO_memcpySwap4(&num, bufSdo); return sprintf(strout, "%d",       num);}
static int dtpI64(char *strout, int stroutSize, char* bufSdo, int bufLen) {int64_t   num; CO_memcpySwap8(&num, bufSdo); return sprintf(strout, "%ld",      num);}
static int dtpR32(char *strout, int stroutSize, char* bufSdo, int bufLen) {float32_t num; CO_memcpySwap4(&num, bufSdo); return sprintf(strout, "%g",       num);}
static int dtpR64(char *strout, int stroutSize, char* bufSdo, int bufLen) {float64_t num; CO_memcpySwap8(&num, bufSdo); return sprintf(strout, "%g",       num);}


/******************************************************************************/
int dtsHex(char* bufSdo, int bufSdoSize, char *strin) {
    char *token;
    char *savePtr;
    int len = 0;
    uint8_t *ubuf = (uint8_t *) bufSdo;

    token = strtok_r(strin, spaceDelim, &savePtr);

    while(token != NULL) {
        char *sRet;

        /* Finish if comment */
        if(*token == '#') {
            return len;
        }

        /* Tokens must be two digit hex characters. Verify also memory overflow */
        if(strlen(token) != 2 || len >= bufSdoSize) {
            return 0;
        }

        ubuf[len++] = (uint8_t)strtoul(token, &sRet, 16);

        if(sRet != strchr(token, '\0')) {
            return 0;
        }

        token = strtok_r(NULL, spaceDelim, &savePtr);
    }

    return len;
}

static int dtsStr(char* bufSdo, int bufSdoSize, char *strin) {
    size_t len = strlen(strin);
    int in = 0;
    int out = 0;
    int string_closed = 0;

    /* Remove blank spaces. Must begin with double quote. */
    while(in < len) {
        char c = strin[in++];

        if(isspace(c) == 0) {
            if(c == '"') {
                break;
            }
            else {
                return 0;
            }
        }
    }

    /* Get string between double quotes. Change "" into " inside string. */
    while(in < len) {
        char c = strin[in++];

        /* verify overflow */
        if(out >= bufSdoSize) {
            return 0;
        }

        /* "" inside string means ". Single " ends the string. */
        if(c == '"'){
            c = strin[in++];
            if(c != '"') {
                string_closed = 1;
                break;
            }
        }
        bufSdo[out++] = c;
    }

    /* Verify tail of string. */
    if(string_closed == 1) {
        /* Comment or nothing is allowed to follow. */
        while(in < len) {
            char c = strin[in++];

            if(isspace(c) == 0) {
                if(c == '#') {
                    break;
                }
                else {
                    return 0;
                }
            }
        }
    }
    else {
        /* There was no ending quote. */
        return 0;
    }

    return out;
}

static int dtsU8 (char* bufSdo, int bufSdoSize, char *strin) {int err = 0; uint8_t   num = (uint8_t) getU32(strin, 0,        UCHAR_MAX, &err); if(err != 0) return 0; memcpy(bufSdo, &num, 1);      return 1;}
static int dtsU16(char* bufSdo, int bufSdoSize, char *strin) {int err = 0; uint16_t  num = (uint16_t)getU32(strin, 0,        USHRT_MAX, &err); if(err != 0) return 0; CO_memcpySwap2(bufSdo, &num); return 2;}
static int dtsU32(char* bufSdo, int bufSdoSize, char *strin) {int err = 0; uint32_t  num =           getU64(strin, 0,        UINT_MAX,  &err); if(err != 0) return 0; CO_memcpySwap4(bufSdo, &num); return 4;}
static int dtsU64(char* bufSdo, int bufSdoSize, char *strin) {int err = 0; uint64_t  num =           getU64(strin, 0,        ULONG_MAX, &err); if(err != 0) return 0; CO_memcpySwap8(bufSdo, &num); return 8;}
static int dtsI8 (char* bufSdo, int bufSdoSize, char *strin) {int err = 0; int8_t    num = (int8_t)  getI32(strin, CHAR_MIN, CHAR_MAX,  &err); if(err != 0) return 0; memcpy(bufSdo, &num, 1);      return 1;}
static int dtsI16(char* bufSdo, int bufSdoSize, char *strin) {int err = 0; int16_t   num = (int16_t) getI32(strin, SHRT_MIN, SHRT_MAX,  &err); if(err != 0) return 0; CO_memcpySwap2(bufSdo, &num); return 2;}
static int dtsI32(char* bufSdo, int bufSdoSize, char *strin) {int err = 0; int32_t   num =           getI64(strin, INT_MIN,  INT_MAX,   &err); if(err != 0) return 0; CO_memcpySwap4(bufSdo, &num); return 4;}
static int dtsI64(char* bufSdo, int bufSdoSize, char *strin) {int err = 0; int64_t   num =           getI64(strin, LONG_MIN, LONG_MAX,  &err); if(err != 0) return 0; CO_memcpySwap8(bufSdo, &num); return 8;}
static int dtsR32(char* bufSdo, int bufSdoSize, char *strin) {int err = 0; float32_t num =           getR32(strin,                      &err); if(err != 0) return 0; CO_memcpySwap4(bufSdo, &num); return 4;}
static int dtsR64(char* bufSdo, int bufSdoSize, char *strin) {int err = 0; float64_t num =           getR64(strin,                      &err); if(err != 0) return 0; CO_memcpySwap8(bufSdo, &num); return 8;}


static const dataType_t dataTypes[] = {
    {"b",   1, dtpI8,  dtsI8 }, /* bool_t (BOOLEAN) */
    {"u8",  1, dtpU8,  dtsU8 }, /* uint8_t (UNSIGNED8) */
    {"u16", 2, dtpU16, dtsU16}, /* uint16_t (UNSIGNED16) */
    {"u32", 4, dtpU32, dtsU32}, /* uint32_t (UNSIGNED32) */
    {"u64", 8, dtpU64, dtsU64}, /* uint64_t (UNSIGNED64) */
    {"i8",  1, dtpI8,  dtsI8 }, /* int8_t (INTEGER8) */
    {"i16", 2, dtpI16, dtsI16}, /* int16_t (INTEGER16) */
    {"i32", 4, dtpI32, dtsI32}, /* int32_t (INTEGER32) */
    {"i64", 8, dtpI64, dtsI64}, /* int64_t (INTEGER64) */
    {"r32", 4, dtpR32, dtsR32}, /* float32_t (REAL32) */
    {"r64", 8, dtpR64, dtsR64}, /* float64_t (REAL64) */
    {"t",   0, dtpHex, dtsHex}, /*  (TIME_OF_DAY (with two arguments: day, ms)) */
    {"td",  0, dtpHex, dtsHex}, /*  (TIME_DIFFERENCE) */
    {"vs",  0, dtpStr, dtsStr}, /* char_t (VISIBLE_STRING) */
    {"os",  0, dtpHex, dtsHex}, /* ochar_t (OCTET_STRING) (mime-base64 (RFC2045) should be used here) */
    {"us",  0, dtpHex, dtsHex}, /*  (UNICODE_STRING) (mime-base64 (RFC2045) should be used here) */
    {"d",   0, dtpHex, dtsHex}  /* domain_t (DOMAIN) (mime-base64 (RFC2045) should be used here) */
};


/******************************************************************************/
char *getTok(char* initStr, const char *delim, int *err) {
    char *token;

    if(*err == 1) {
        return NULL;
    }

    if((token = strtok_r(initStr, delim, &strtokSavePtr)) == NULL) {
        *err = 1;
        return NULL;
    }

    return token;
}


void lastTok(char* initStr, const char *delim, int *err) {
    char *token;
    int errTokLast = 0;

    token = getTok(initStr, delim, &errTokLast);

    if(token != NULL) {
        if(*token != '#') {
            *err = 1;
        }
    }
}


uint32_t getU32(char *token, uint32_t min, uint32_t max, int *err) {
    uint32_t num;
    char *sRet = NULL;

    if(token == NULL || *err == 1) {
        *err = 1;
        return 0;
    }

    num = strtoul(token, &sRet, 0);
    if(sRet != strchr(token, '\0') || num < min || num > max) {
        *err = 1;
        return 0;
    }

    return num;
}


int32_t getI32(char *token, int32_t min, int32_t max, int *err) {
    int32_t num;
    char *sRet = NULL;

    if(token == NULL || *err == 1) {
        *err = 1;
        return 0;
    }

    num = strtol(token, &sRet, 0);
    if(sRet != strchr(token, '\0') || num < min || num > max) {
        *err = 1;
        return 0;
    }

    return num;
}


uint64_t getU64(char *token, uint64_t min, uint64_t max, int *err) {
    uint64_t num;
    char *sRet = NULL;

    if(token == NULL || *err == 1) {
        *err = 1;
        return 0;
    }

    num = strtoull(token, &sRet, 0);
    if(sRet != strchr(token, '\0') || num < min || num > max) {
        *err = 1;
        return 0;
    }

    return num;
}


int64_t getI64(char *token, int64_t min, int64_t max, int *err) {
    int64_t num;
    char *sRet = NULL;

    if(token == NULL || *err == 1) {
        *err = 1;
        return 0;
    }

    num = strtoll(token, &sRet, 0);
    if(sRet != strchr(token, '\0') || num < min || num > max) {
        *err = 1;
        return 0;
    }

    return num;
}


float32_t getR32(char *token, int *err) {
    float32_t num;
    char *sRet = NULL;

    if(token == NULL || *err == 1) {
        *err = 1;
        return 0.0;
    }

    num = strtof(token, &sRet);
    if(sRet != strchr(token, '\0')) {
        *err = 1;
        return 0.0;
    }

    return num;
}


float64_t getR64(char *token, int *err) {
    float64_t num;
    char *sRet = NULL;

    if(token == NULL || *err == 1) {
        *err = 1;
        return 0.0;
    }

    num = strtod(token, &sRet);
    if(sRet != strchr(token, '\0')) {
        *err = 1;
        return 0.0;
    }

    return num;
}


const dataType_t *getDataType(char *token, int *err) {
    int i, len;

    if(token == NULL || *err == 1) {
        *err = 1;
        return NULL;
    }

    len = sizeof(dataTypes) / sizeof(dataType_t);

    for(i=0; i<len; i++) {
        const dataType_t *dt =  &dataTypes[i];
        if(strcmp(token, dt->syntax) == 0) {
            return dt;
        }
    }

    *err = 1;
    return NULL;
}


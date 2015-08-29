/*
 * CANopen main program file for Linux SocketCAN.
 *
 * @file        main
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
//#include "eeprom.h"
//#include "CgiLog.h"
//#include "CgiOD.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <pthread.h>
#include <sys/timerfd.h>

#include <net/if.h>
#include <sys/socket.h>
#include <linux/can.h>


#define NSEC_PER_SEC        (1000000000)    /* The number of nsecs per sec. */
#define TMR_TASK_INTERVAL   (1000)          /* Interval of tmrTask thread in microseconds */
#define INCREMENT_1MS(var)  (var++)         /* Increment 1ms variable in tmrTask */


/* Global variables and objects */
    volatile uint16_t   CO_timer1ms = 0U;   /* variable increments each millisecond */
    volatile bool_t     CO_CAN_OK = false;  /* CAN in normal mode indicator */

//    CO_EE_t             CO_EEO;             /* EEprom object */

//    CgiLog_t            CgiLogObj;
//    CgiCli_t            CgiCliObj;
//    CgiLog_t           *CgiLog = &CgiLogObj;
//    CgiCli_t           *CgiCli = &CgiCliObj;
//
//    #define CgiCANLogBufSize    0x80000L
//    #define CgiCliBufSize       0x10000
//    #define CgiCliSDObufSize    0x800
//    uint8_t CgiCANLogBuf0[CgiCANLogBufSize];
//    uint8_t CgiCANLogBuf1[CgiCANLogBufSize];
//    char_t CgiCliBuf[CgiCliBufSize];
//    uint8_t CgiCliSDObuf[CgiCliSDObufSize];


/* Program structure and critical sections
 * Program runs in multiple threads.
 * For program structure see CANopenNode/README.
 * For info on critical sections see CANopenNode/stack/drvTemplate/CO_driver.h.
 */

/* Timer periodic thread */
    static void* tmrTask_thread(void* arg);

/* CAN Receive thread */
    static void* CANrx_thread(void* arg);
    static bool_t CANrx_lockedBySync = false;
    static pthread_mutex_t CANrx_mtx = PTHREAD_MUTEX_INITIALIZER;


/* Helper functions ***********************************************************/
void CO_errExit(char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

static void usageExit(char *progName){
    fprintf(stderr, "Usage: %s <device> -i <Node ID (1..127)> [options]\n", progName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -p <RT priority>    Realtime priority of CANrx thread (default=1). Timer\n");
    fprintf(stderr, "                      thread has priority of CANrx + 1. If -1, no RT is used.\n");
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void cbCANrx_lockBySync(bool_t syncReceived){
    //CALLBACK function is called immediately after SYNC message on CAN bus.
    //It mutes CAN receive thread.
    if(!syncReceived){
        if(pthread_mutex_lock(&CANrx_mtx) != 0)
            CO_errExit("CANrx - Mutex lock failed");
    }
    CANrx_lockedBySync = true;
}
static void cbCANrx_unlockBySync(void){
    if(CANrx_lockedBySync){
        if(pthread_mutex_unlock(&CANrx_mtx) != 0)
            CO_errExit("CANrx - Mutex unlock failed");
        CANrx_lockedBySync = false;
    }
}

//static void wakeUpTask(uint32_t arg){
//    RTX_Wakeup(arg);
//}

/* main ***********************************************************************/
int main (int argc, char *argv[]){
    CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
    uint8_t powerOn = 1;
    struct timespec sleep50ms, sleep2ms;
    pthread_t tmrTask_id, CANrx_id;
    //Real time priority for CANrx thread. Thread tmrTask has priority of
    //CANrx + 1. If zero, no RT is used for threads.
    int nodeId = -1;    //Set to 1..127 by arguments
    int rtPriority = 1; //Real time priority, 1 by default, settable by arguments.
    char* CANdevice = NULL;//CAN device, settable by arguments.
    int CANdeviceIndex = 0;
    int CANsocket = -1;
    struct sockaddr_can CANsocketAddr;
    int opt;


    /* Get program options */
    while((opt = getopt(argc, argv, "i:p:")) != -1) {
        switch (opt) {
            case 'i': nodeId = strtol(optarg, NULL, 0);     break;
            case 'p': rtPriority = strtol(optarg, NULL, 0); break;
            default:  usageExit(argv[0]);
        }
    }

    if(optind < argc){
        CANdevice = argv[optind];
        CANdeviceIndex = if_nametoindex(CANdevice);
    }

    if(nodeId < 1 || nodeId > 127){
        fprintf(stderr, "Wrong node ID (%d)\n", nodeId);
        usageExit(argv[0]);
    }

    if(rtPriority != -1 && (rtPriority < sched_get_priority_min(SCHED_FIFO)
                   || (rtPriority + 1) > sched_get_priority_max(SCHED_FIFO))){
        fprintf(stderr, "Wrong RT priority (%d)\n", rtPriority);
        usageExit(argv[0]);
    }

    if(CANdeviceIndex == 0){
        char err[120];
        snprintf(err, 120, "Can't find CAN device \"%s\"", CANdevice);
        CO_errExit(err);
    }


    printf("%s - starting CANopen device with Node ID %d(0x%02X) ...\n", argv[0], nodeId, nodeId);


    /* Init variables */
    sleep50ms.tv_sec = 0;
    sleep50ms.tv_nsec = 50000000L;
    sleep2ms.tv_sec = 0;
    sleep2ms.tv_nsec = 2000000L;


    /* Verify, if OD structures have proper alignment of initial values */
    if(CO_OD_RAM.FirstWord != CO_OD_RAM.LastWord) {
        fprintf(stderr, "Program init - %s - Error in CO_OD_RAM.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(CO_OD_EEPROM.FirstWord != CO_OD_EEPROM.LastWord) {
        fprintf(stderr, "Program init - %s - Error in CO_OD_EEPROM.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(CO_OD_ROM.FirstWord != CO_OD_ROM.LastWord) {
        fprintf(stderr, "Program init - %s - Error in CO_OD_ROM.\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    /* Generate thread with constant interval and thread for CAN receive */
    if(pthread_create(&tmrTask_id, NULL, tmrTask_thread, NULL) != 0)
        CO_errExit("Program init - tmrTask thread creation failed");
    if(pthread_create(&CANrx_id, NULL, CANrx_thread, NULL) != 0)
        CO_errExit("Program init - CANrx thread creation failed");

    /* Set threads as Realtime */
    if(rtPriority > 0){
        struct sched_param param;

        param.sched_priority = rtPriority + 1;
        if(pthread_setschedparam(tmrTask_id, SCHED_FIFO, &param) != 0)
            CO_errExit("Program init - tmrTask thread set scheduler failed");

        param.sched_priority = rtPriority;
        if(pthread_setschedparam(CANrx_id, SCHED_FIFO, &param) != 0)
            CO_errExit("Program init - CANrx thread set scheduler failed");
    }


    /* Sreate and bind socket */
    CANsocket = socket(AF_CAN, SOCK_RAW, CAN_RAW);
    if(CANsocket < 0)
        CO_errExit("Program init - Socket creation failed");

    CANsocketAddr.can_family = AF_CAN;
    CANsocketAddr.can_ifindex = CANdeviceIndex;
    if(bind(CANsocket, (struct sockaddr*)&CANsocketAddr, sizeof(CANsocketAddr)) != 0)
        CO_errExit("Program init - Socket binding failed");

//    /* initialize battery powered SRAM */
//    struct{
//        uint8_t* EEPROMptr;
//        uint32_t EEPROMsize;
//        uint8_t* emcyBufPtr;
//        uint32_t emcyBufSize;
//        uint32_t size;
//    }SRAM;
//
//    SRAM.size = 0;
//    /* Sizes must be divisible by 4. (struct should have uint32_t as last element.) */
//    SRAM.EEPROMsize = sizeof(CO_OD_EEPROM);                   SRAM.size += SRAM.EEPROMsize;
//    SRAM.emcyBufSize = OD_CANopenLog.emcySRAMSize&0xFFFFFFFC; SRAM.size += SRAM.emcyBufSize;
//
//    if(RTX_MemWindow(gSysPublicData.sramAddr, SRAM.size)){
//        SRAM.EEPROMptr = 0;
//        SRAM.emcyBufPtr = 0;
//        printf("\nError: SRAM initialization failed.");
//        return 1;
//    }
//    else{
//        SRAM.EEPROMptr = gSysPublicData.sramAddr;
//        SRAM.emcyBufPtr = SRAM.EEPROMptr + SRAM.EEPROMsize;
//    }


//    /* initialize EEPROM - part 1 */
//    CO_ReturnError_t eeStatus = CO_EE_init_1(&CO_EEO, SRAM.EEPROMptr, (uint8_t*) &CO_OD_EEPROM, sizeof(CO_OD_EEPROM),
//                            (uint8_t*) &CO_OD_ROM, sizeof(CO_OD_ROM));


//    /* initialize CGI interface. These functions are bound with a fixed name and */
//    /* are executed by the Web server task, if a HTTP request with such a fixed */
//    /* name comes in. This mechanism allows dynamic usage of the IPC@CHIP Web server. */
//    OD_CANopenLog.CANLogSize = CgiCANLogBufSize;
//
//    if(   CgiLog_init_1(CgiLog, SRAM.emcyBufPtr, SRAM.emcyBufSize, CgiCANLogBuf0,
//                        CgiCANLogBuf1, CgiCANLogBufSize, OD_CANopenLog.maxDumpFiles)
//       || CgiCli_init_1(CgiCli, CgiCliBuf, CgiCliBufSize, CgiCliSDObuf, CgiCliSDObufSize))
//    {
//        printf("\nError: CGI initialization failed.");
//        exit(EXIT_FAILURE);
//    }


    /* increase variable each startup. Variable is stored in eeprom. */
    OD_powerOnCounter++;


    while(reset != CO_RESET_APP && reset != CO_RESET_QUIT){
/* CANopen communication reset - initialize CANopen objects *******************/
        CO_ReturnError_t err;
        uint16_t timer1msPrevious;

        printf("%s - communication reset ...\n", argv[0]);

        /* Wait tmrTask and CANrx, then configure CAN */
        CO_LOCK_OD();
        if(pthread_mutex_lock(&CANrx_mtx) != 0)
            CO_errExit("mainline - Mutex lock CANrx failed");

        CO_CAN_OK = false;

        CO_UNLOCK_OD();
        if(pthread_mutex_unlock(&CANrx_mtx) != 0)
            CO_errExit("mainline - Mutex unlock CANrx failed");

        CO_CANsetConfigurationMode(CANsocket);


        /* initialize CANopen */
        err = CO_init(CANsocket, nodeId, 0);
        if(err != CO_ERROR_NO){
            fprintf(stderr, "Communication reset - %s - CANopen initialization failed.\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        /* set callback functions for task control. */
        CO_SYNC_initCallback(CO->SYNC, cbCANrx_lockBySync);
        /* Prepare function, which will wake this task after CAN SDO response is */
        /* received (inside CAN receive interrupt). */
//        CO->SDO->pFunctSignal = wakeUpTask;    /* will wake from RTX_Sleep_Time() */
//        CO->SDO->functArg = RTX_Get_TaskID();  /* id of this task */

//        /* initialize eeprom - part 2 */
//        CO_EE_init_2(&CO_EEO, eeStatus, CO->SDO, CO->em);


        /* just for info */
        if(powerOn){
            CO_errorReport(CO->em, CO_EM_MICROCONTROLLER_RESET, CO_EMC_NO_ERROR, OD_powerOnCounter);
            powerOn = 0;
        }


        /* start CAN */
        CO_CANsetNormalMode(CO->CANmodule[0]);
        CO_CAN_OK = true;

        reset = CO_RESET_NOT;
        timer1msPrevious = CO_timer1ms;
        printf("%s - running ...\n", argv[0]);


#if 1
        struct can_filter rfilter[50];
        can_err_mask_t err_mask;
        int loopback, recv_own_msgs, enable_can_fd;
        socklen_t len1 = sizeof(rfilter);
        socklen_t len2 = sizeof(err_mask);
        socklen_t len3 = sizeof(loopback);
        socklen_t len4 = sizeof(recv_own_msgs);
        socklen_t len5 = sizeof(enable_can_fd);

        getsockopt(CANsocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, &len1);
        getsockopt(CANsocket, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, &len2);
        getsockopt(CANsocket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, &len3);
        getsockopt(CANsocket, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs, &len4);
        getsockopt(CANsocket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_can_fd, &len5);
        int ifil;
        for(ifil=0; ifil<50; ifil++)
            printf("filter[%2d].id=%08X, filter[%2d].mask=%08X\n", ifil, rfilter[ifil].can_id, ifil, rfilter[ifil].can_mask);
        printf("filterSize=%d, errMask=%x, loopback=%d, recv_own_msgs=%d, enable_can_fd=%d\n",
               len1, err_mask, loopback, recv_own_msgs, enable_can_fd);
#endif


        while(reset == CO_RESET_NOT){
/* loop for normal program execution ******************************************/
            uint16_t timer1msCopy, timer1msDiff;

            timer1msCopy = CO_timer1ms;
            timer1msDiff = timer1msCopy - timer1msPrevious;
            timer1msPrevious = timer1msCopy;


            /* CANopen process */
            reset = CO_process(CO, timer1msDiff);

            /* Nonblocking application code may go here. */

//            CgiLogEmcyProcess(CgiLog);


            if(CO->SDO->state == CO_SDO_ST_UPLOAD_BL_SUBBLOCK) {
                nanosleep(&sleep2ms, NULL);
            }
            else {
                nanosleep(&sleep50ms, NULL);
            }


            /* Process EEPROM */
//            CO_EE_process(&CO_EEO);
        }
    }


/* program exit ***************************************************************/
    /* lock threads */
    CO_LOCK_OD();
    if(pthread_mutex_lock(&CANrx_mtx) != 0)
        CO_errExit("Program exit - Mutex lock CANrx failed");

    /* delete objects from memory */
    CO_delete(CANsocket);
//    CgiLog_delete(CgiLog);
//    CgiCli_delete(CgiCli);
    close(CANsocket);


    printf("%s - finished.\n", argv[0]);
//    if(reset == CO_RESET_APP) reboot(); /* CANopen reset node */
    exit(EXIT_SUCCESS);
}


/* timer thread executes in constant intervals ********************************/
static void* tmrTask_thread(void* arg) {
    struct timespec tmr;

    if(clock_gettime(CLOCK_MONOTONIC, &tmr) == -1)
        CO_errExit("tmrTask - gettime failed");

    for(;;){
        /* Wait for timer to expire, then calculate next shot */
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &tmr, NULL);
        tmr.tv_nsec += (TMR_TASK_INTERVAL*1000);
        if(tmr.tv_nsec >= NSEC_PER_SEC) {
            tmr.tv_nsec -= NSEC_PER_SEC;
            tmr.tv_sec++;
        }

#if 1
        /* trace timing */
        long dt = 1000;    //delta-time in microseconds
        struct timespec tnow;
        static struct timespec tprev;
        static int cnt = -1, dtmin = 1000, dtmax= 0, dtmaxmax = 0;

        clock_gettime(CLOCK_MONOTONIC, &tnow);
        if(cnt >= 0) {
            dt = (tnow.tv_sec - tprev.tv_sec) * 1000000;
            dt += (tnow.tv_nsec - tprev.tv_nsec) / 1000;
        }
        tprev.tv_sec = tnow.tv_sec;
        tprev.tv_nsec = tnow.tv_nsec;

        if(dt < dtmin) dtmin = dt;
        if(dt > dtmax) dtmax = dt;
        if(dt > dtmaxmax) dtmaxmax = dt;

        if(++cnt == 1000) {
            printf("tmr: %9ld, %6d, %6d, %6d\n", tnow.tv_nsec/1000, dtmin, dtmax, dtmaxmax);
            cnt = 0; dtmin = 1000; dtmax = 0;
        }
#endif

        /* Lock PDOs and OD */
        CO_LOCK_OD();

        INCREMENT_1MS(CO_timer1ms);

        if(CO_CAN_OK) {
            bool_t syncWas;

            /* Process Sync and read inputs */
            syncWas = CO_process_SYNC_RPDO(CO, TMR_TASK_INTERVAL);

            /* Reenable CANrx, if it was disabled by SYNC callback */
            if(syncWas){
                cbCANrx_unlockBySync();
            }

            /* Further I/O or nonblocking application code may go here. */

            /* Write outputs */
            CO_process_TPDO(CO, syncWas, TMR_TASK_INTERVAL);
        }

        CO_UNLOCK_OD();

#if 0
        /* verify overflow */
        if(timer_overflow) {
            CO_errorReport(CO->em, CO_EM_ISR_TIMER_OVERFLOW, CO_EMC_SOFTWARE_INTERNAL, 0);
        }
#endif

    }

    return NULL;
}


/* CAN receive thread *********************************************************/
static void* CANrx_thread(void* arg) {

    for(;;){
        struct can_frame msg;
        int n, size;

        size = sizeof(struct can_frame);
        n = read(CO->CANmodule[0]->fdSocket, &msg, size);
        if(n != size) {
            CO_errorReport(CO->em, CO_EM_GENERIC_SOFTWARE_ERROR, CO_EMC_GENERIC, n);
        }else{
            CO_CANreceive(CO->CANmodule[0], &msg);
        }
    }

    return NULL;
}

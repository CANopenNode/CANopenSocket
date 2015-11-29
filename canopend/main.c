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
#include "eeprom.h"
#include "Linux_tasks.h"
//#include "CgiLog.h"
//#include "CgiOD.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#ifndef CO_SINGLE_THREAD
#include <pthread.h>
#endif


#define NSEC_PER_SEC            (1000000000)    /* The number of nanoseconds per second. */
#define NSEC_PER_MSEC           (1000000)       /* The number of nanoseconds per millisecond. */
#define TMR_TASK_INTERVAL_NS    (1000000)       /* Interval of taskTmr in nanoseconds */
#define TMR_TASK_OVERFLOW_US    (3000)          /* Overflow detect limit for taskTmr in microseconds */
#define INCREMENT_1MS(var)      (var++)         /* Increment 1ms variable in taskTmr */


/* Global variables and objects */
    volatile uint16_t       CO_timer1ms = 0U;   /* variable increments each millisecond */
    static int              rtPriority = -1;    /* Real time priority, configurable by arguments. (-1=RT disabled) */
    static int              mainline_epoll_fd;  /* epoll file descriptor for mainline */
    CO_EE_t                 CO_EEO;             /* EEprom object */


/* Realtime thread */
#ifndef CO_SINGLE_THREAD
    static void*            rt_thread(void* arg);
    static pthread_t        rt_thread_id;
    static int              rt_thread_epoll_fd;
#endif


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


/* Signal handler */
    static volatile sig_atomic_t endProgram = 0;
    static void sigHandler(int sig){
        endProgram = 1;
    }


/* Helper functions ***********************************************************/
void CO_errExit(char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

static void usageExit(char *progName){
    fprintf(stderr, "Usage: %s <device> -i <Node ID (1..127)> [options]\n", progName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -p <RT priority>    Realtime priority of RT task (RT disabled by default).\n");
    fprintf(stderr, "  -r                  Enable reboot on CANopen NMT reset_node command. \n");
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}


/******************************************************************************/
/** Mainline and RT thread                                                   **/
/******************************************************************************/
int main (int argc, char *argv[]){
    CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
    CO_ReturnError_t eeStatus;
    uint8_t powerOn = 1;
    int CANdevice0Index = 0;
    int opt;
    bool_t firstRun = true;

    char* CANdevice = NULL;     /* CAN device, configurable by arguments. */
    int nodeId = -1;            /* Set to 1..127 by arguments */
    bool_t rebootEnable = false;/* Configurable by arguments */

    if(argc < 3 || strcmp(argv[1], "--help") == 0) usageExit(argv[0]);


    /* Get program options */
    while((opt = getopt(argc, argv, "i:p:r")) != -1) {
        switch (opt) {
            case 'i': nodeId = strtol(optarg, NULL, 0);     break;
            case 'p': rtPriority = strtol(optarg, NULL, 0); break;
            case 'r': rebootEnable = true;                  break;
            default:  usageExit(argv[0]);
        }
    }

    if(optind < argc){
        CANdevice = argv[optind];
        CANdevice0Index = if_nametoindex(CANdevice);
    }

    if(nodeId < 1 || nodeId > 127){
        fprintf(stderr, "Wrong node ID (%d)\n", nodeId);
        usageExit(argv[0]);
    }

    if(rtPriority != -1 && (rtPriority < sched_get_priority_min(SCHED_FIFO)
                         || rtPriority > sched_get_priority_max(SCHED_FIFO))){
        fprintf(stderr, "Wrong RT priority (%d)\n", rtPriority);
        usageExit(argv[0]);
    }

    if(CANdevice0Index == 0){
        char s[120];
        snprintf(s, 120, "Can't find CAN device \"%s\"", CANdevice);
        CO_errExit(s);
    }


    printf("%s - starting CANopen device with Node ID %d(0x%02X) ...\n", argv[0], nodeId, nodeId);


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


    /* initialize EEPROM - part 1 */
    eeStatus = CO_EE_init_1(&CO_EEO, NULL, (uint8_t*) &CO_OD_EEPROM, sizeof(CO_OD_EEPROM),
                            (uint8_t*) &CO_OD_ROM, sizeof(CO_OD_ROM));


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

    /* Catch signals SIGINT and SIGTERM */
    if(signal(SIGINT, sigHandler) == SIG_ERR)
        CO_errExit("Program init - SIGINIT handler creation failed");
    if(signal(SIGTERM, sigHandler) == SIG_ERR)
        CO_errExit("Program init - SIGTERM handler creation failed");

    /* increase variable each startup. Variable is stored in eeprom. */
    OD_powerOnCounter++;


    while(reset != CO_RESET_APP && reset != CO_RESET_QUIT && endProgram == 0){
/* CANopen communication reset - initialize CANopen objects *******************/
        CO_ReturnError_t err;

        printf("%s - communication reset ...\n", argv[0]);

        /* Wait rt_thread, then configure CAN */
        if(!firstRun){
            CO_LOCK_OD();
            CO->CANmodule[0]->CANnormal = false;
            CO_UNLOCK_OD();
        }

        CO_CANsetConfigurationMode(CANdevice0Index);


        /* initialize CANopen */
        err = CO_init(CANdevice0Index, nodeId, 0);
        if(err != CO_ERROR_NO){
            char s[120];
            snprintf(s, 120, "Communication reset - CANopen initialization failed, err=%d", err);
            CO_errExit(s);
        }


        /* initialize eeprom - part 2 */
        CO_EE_init_2(&CO_EEO, eeStatus, CO->SDO, CO->em);


        /* just for info */
        if(powerOn){
            CO_errorReport(CO->em, CO_EM_MICROCONTROLLER_RESET, CO_EMC_NO_ERROR, OD_powerOnCounter);
            powerOn = 0;
        }


        /* Configure callback functions for task control */
        CO_EM_initCallback(CO->em, taskMain_cbSignal);
        CO_SDO_initCallback(CO->SDO, taskMain_cbSignal);
        CO_SDOclient_initCallback(CO->SDOclient, taskMain_cbSignal);

        CO_SYNC_initCallback(CO->SYNC, CANrx_lockCbSync);


        /* First time only initialization. */
        if(firstRun){
            firstRun = false;

            /* Configure epoll for mainline */
            mainline_epoll_fd = epoll_create(4);
            if(mainline_epoll_fd == -1)
                CO_errExit("Program init - epoll_create mainline failed");

            /* Init mainline */
            taskMain_init(mainline_epoll_fd, &OD_performance[ODA_performance_mainCycleMaxTime]);


#ifdef CO_SINGLE_THREAD
            /* Init taskRT */
            CANrx_taskTmr_init(mainline_epoll_fd, TMR_TASK_INTERVAL_NS, &OD_performance[ODA_performance_timerCycleMaxTime]);

            OD_performance[ODA_performance_timerCycleTime] = TMR_TASK_INTERVAL_NS/1000; /* informative */

            /* Set priority for mainline */
            if(rtPriority > 0){
                struct sched_param param;

                param.sched_priority = rtPriority;
                if(sched_setscheduler(0, SCHED_FIFO, &param) != 0)
                    CO_errExit("Program init - mainline set scheduler failed");
            }
#else
            /* Configure epoll for rt_thread */
            rt_thread_epoll_fd = epoll_create(2);
            if(rt_thread_epoll_fd == -1)
                CO_errExit("Program init - epoll_create rt_thread failed");

            /* Init taskRT */
            CANrx_taskTmr_init(rt_thread_epoll_fd, TMR_TASK_INTERVAL_NS, &OD_performance[ODA_performance_timerCycleMaxTime]);

            OD_performance[ODA_performance_timerCycleTime] = TMR_TASK_INTERVAL_NS/1000; /* informative */

            /* Create rt_thread */
            if(pthread_create(&rt_thread_id, NULL, rt_thread, NULL) != 0)
                CO_errExit("Program init - rt_thread creation failed");

            /* Set priority for rt_thread */
            if(rtPriority > 0){
                struct sched_param param;

                param.sched_priority = rtPriority;
                if(pthread_setschedparam(rt_thread_id, SCHED_FIFO, &param) != 0)
                    CO_errExit("Program init - rt_thread set scheduler failed");
            }
#endif
        }


        /* start CAN */
        CO_CANsetNormalMode(CO->CANmodule[0]);

        reset = CO_RESET_NOT;

        printf("%s - running ...\n", argv[0]);


        while(reset == CO_RESET_NOT && endProgram == 0){
/* loop for normal program execution ******************************************/
            int ready;
            struct epoll_event ev;

            ready = epoll_wait(mainline_epoll_fd, &ev, 1, -1);

            if(ready != 1){
                if(errno != EINTR){
                    CO_errorReport(CO->em, CO_EM_GENERIC_SOFTWARE_ERROR, CO_EMC_SOFTWARE_INTERNAL, 0x11100000L | errno);
                }
            }

#ifdef CO_SINGLE_THREAD
            else if(CANrx_taskTmr_process(ev.data.fd)){
                /* code was processed in the above function. Additional code process below */
                INCREMENT_1MS(CO_timer1ms);
                /* Detect timer large overflow */
                if(OD_performance[ODA_performance_timerCycleMaxTime] > TMR_TASK_OVERFLOW_US && rtPriority > 0){
                    CO_errorReport(CO->em, CO_EM_ISR_TIMER_OVERFLOW, CO_EMC_SOFTWARE_INTERNAL, 0x22400000L | OD_performance[ODA_performance_timerCycleMaxTime]);
                }
            }
#endif

            else if(taskMain_process(ev.data.fd, &reset, CO_timer1ms)){
                /* code was processed in the above function. Additional code process below */
                //            CgiLogEmcyProcess(CgiLog);
				CO_EE_process(&CO_EEO);
#if 0
				static uint16_t maxIntervalMain = 0;
				if(maxIntervalMain < timer1msDiff){
					maxIntervalMain = timer1msDiff;
					printf("Maximum main interval time: %d milliseconds\n", maxIntervalMain);
				}
				printf("%2d ", timer1msDiff);
#endif
            }

            else{
                /* No file descriptor was processed. */
                CO_errorReport(CO->em, CO_EM_GENERIC_SOFTWARE_ERROR, CO_EMC_SOFTWARE_INTERNAL, 0x11200000L);
            }
        }
    }


/* program exit ***************************************************************/
    /* join threads */
    endProgram = 1;
#ifndef CO_SINGLE_THREAD
    if(pthread_join(rt_thread_id, NULL) != 0){
        CO_errExit("Program end - pthread_join failed");
    }
#endif

    /* delete objects from memory */
    CANrx_taskTmr_close();
    taskMain_close();
    CO_delete(CANdevice0Index);

    printf("%s on %s (nodeId=0x%02X) - finished.\n\n", argv[0], CANdevice, nodeId);

    /* Flush all buffers (and reboot) */
    if(rebootEnable && reset == CO_RESET_APP){
        sync();
        if(reboot(LINUX_REBOOT_CMD_RESTART) != 0){
            CO_errExit("Program end - reboot failed");
        }
    }

    exit(EXIT_SUCCESS);
}


#ifndef CO_SINGLE_THREAD
/* Realtime thread for CAN receive and taskTmr ********************************/
static void* rt_thread(void* arg){

    /* Endless loop */
    while(endProgram == 0){
        int ready;
        struct epoll_event ev;

        ready = epoll_wait(rt_thread_epoll_fd, &ev, 1, -1);

        if(ready != 1){
            if(errno != EINTR){
                CO_errorReport(CO->em, CO_EM_GENERIC_SOFTWARE_ERROR, CO_EMC_SOFTWARE_INTERNAL, 0x12100000L | errno);
            }
        }

        else if(CANrx_taskTmr_process(ev.data.fd)){
            /* code was processed in the above function. Additional code process below */
            INCREMENT_1MS(CO_timer1ms);
            /* Detect timer large overflow */
            if(OD_performance[ODA_performance_timerCycleMaxTime] > TMR_TASK_OVERFLOW_US && rtPriority > 0){
                CO_errorReport(CO->em, CO_EM_ISR_TIMER_OVERFLOW, CO_EMC_SOFTWARE_INTERNAL, 0x22400000L | OD_performance[ODA_performance_timerCycleMaxTime]);
            }
#if 0
			static uint16_t maxInterval = 0;
			static uint16_t cntxx = 0;
			if(maxInterval < OD_performance[ODA_performance_timerCycleMaxTime]){
				maxInterval = OD_performance[ODA_performance_timerCycleMaxTime];
				printf("Maximum timer interval time: %5d microseconds\n", maxInterval);
			}
			if(++cntxx == 1000){
				cntxx = 0;
				printf("\n");
			}
#endif
        }

        else{
            /* No file descriptor was processed. */
            CO_errorReport(CO->em, CO_EM_GENERIC_SOFTWARE_ERROR, CO_EMC_SOFTWARE_INTERNAL, 0x12200000L);
        }
    }

    return NULL;
}
#endif

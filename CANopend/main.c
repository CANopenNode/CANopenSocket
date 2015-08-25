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


#define NSEC_PER_SEC        (1000000000)    /* The number of nsecs per sec. */
#define TMR_TASK_INTERVAL   (1000)          /* Interval of tmrTask thread in microseconds */
#define INCREMENT_1MS(var)  (var++)         /* Increment 1ms variable in tmrTask */
#define USE_RT_SCHEDULER


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


/* Threads and race condition.
 * There are three threads. Each runs in endless loop:
 * - mainline: after initialization it processes time-non-critical tasks like
 *      SDO, Heartbeat, etc. nanosleep() is after each cycle. Critical sections
 *      which accesses data received from CAN(CANrx thread) and data from
 *      Object dictionary(tmrTask thread) are protected by mutexes.
 * - tmrTask: Realtime thread first copies data from received PDOs(CANopen
 *      process data objects) into preconfigured places in Object dictionary(OD).
 *      Then it prepares transmitting PDOs from OD data and sends it over CAN
 *      according to PDO transmission and mapping parameters.
 *      tmrTask thread executes fast and is protected by mutex.
 * - CANrx: Realtime thread receives CAN messages. It delivers CAN message to
 *      correct CANopen object. Message is later processed by another thread.
 *      It is locked by mutex only by cbCANrx_lockBySync() function. Other
 *      CANopen objects don't need locking of the CANrx.
 */

/* Timer periodic thread */
    static void* tmrTask_thread(void* arg);
    static pthread_mutex_t tmrTask_mtx = PTHREAD_MUTEX_INITIALIZER;

/* CAN Receive thread */
    static void* CANrx_thread(void* arg);
    static bool_t CANrx_lockedBySync = false;
    static pthread_mutex_t CANrx_mtx = PTHREAD_MUTEX_INITIALIZER;


/* Helper functions ***********************************************************/
static void errExit(char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

static void cbCANrx_lockBySync(void *arg){
    //CALLBACK function is called immediatelly after SYNC message on CAN bus.
    //It mutes CAN receive thread.
    if(pthread_mutex_lock(&CANrx_mtx) != 0)
        errExit("CANrx - Mutex lock failed");
    CANrx_lockedBySync = true;
}
static void cbCANrx_unlockBySync(void){
    if(CANrx_lockedBySync){
        if(pthread_mutex_unlock(&CANrx_mtx) != 0)
            errExit("CANrx - Mutex unlock failed");
        CANrx_lockedBySync = false;
    }
}

//static void wakeUpTask(uint32_t arg){
//    RTX_Wakeup(arg);
//}

/* main ***********************************************************************/
int main (int argc, char *argv[]){
    CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
    struct timespec sleep50ms, sleep2ms;
    pthread_t tmrTask_id, CANrx_id;
#ifdef USE_RT_SCHEDULER
    const struct sched_param tmrTask_priority = {1};
    const struct sched_param CANrx_priority = {1};
#endif
    uint8_t powerOn = 1;

    printf("%s - starting ...\n", argv[0]);


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


    /* Generate RT thread with constant interval */
    if(pthread_create(&tmrTask_id, NULL, tmrTask_thread, NULL) != 0)
        errExit("Program init - tmrTask thread creation failed");
#ifdef USE_RT_SCHEDULER
    if(pthread_setschedparam(tmrTask_id, SCHED_FIFO, &tmrTask_priority) != 0)
        errExit("Program init - tmrTask thread set scheduler failed");
#endif

    /* Generate RT thread for CAN receive */
    if(pthread_create(&CANrx_id, NULL, CANrx_thread, NULL) != 0)
        errExit("Program init - CANrx thread creation failed");
#ifdef USE_RT_SCHEDULER
    if(pthread_setschedparam(CANrx_id, SCHED_FIFO, &CANrx_priority) != 0)
        errExit("Program init - CANrx thread set scheduler failed");
#endif

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
        if(pthread_mutex_lock(&tmrTask_mtx) != 0)
            errExit("mainline - Mutex lock tmrTask failed");
        if(pthread_mutex_lock(&CANrx_mtx) != 0)
            errExit("mainline - Mutex lock CANrx failed");

        CO_CAN_OK = false;

        if(pthread_mutex_unlock(&tmrTask_mtx) != 0)
            errExit("mainline - Mutex unlock tmrTask failed");
        if(pthread_mutex_unlock(&CANrx_mtx) != 0)
            errExit("mainline - Mutex unlock CANrx failed");

        CO_CANsetConfigurationMode(ADDR_CAN1);


        /* initialize CANopen */
        err = CO_init();
        if(err != CO_ERROR_NO){
            fprintf(stderr, "Communication reset - %s - CANopen initialization failed.\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        /* set callback functions for task control. */
        CO_SYNC_initCallback(CO->SYNC, cbCANrx_lockBySync, NULL);
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
        CO_CANsetNormalMode(ADDR_CAN1);
        CO_CAN_OK = true;

        reset = CO_RESET_NOT;
        timer1msPrevious = CO_timer1ms;
        printf("%s - running ...\n", argv[0]);

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
    if(pthread_mutex_lock(&tmrTask_mtx) != 0)
        errExit("Program exit - Mutex lock tmrTask failed");
    if(pthread_mutex_lock(&CANrx_mtx) != 0)
        errExit("Program exit - Mutex lock CANrx failed");

    /* delete objects from memory */
    CO_delete();
//    CgiLog_delete(CgiLog);
//    CgiCli_delete(CgiCli);


    printf("%s - finished.\n", argv[0]);
//    if(reset == CO_RESET_APP) reboot(); /* CANopen reset node */
    exit(EXIT_SUCCESS);
}


/* timer thread executes in constant intervals ********************************/
static void* tmrTask_thread(void* arg) {
    struct timespec tmr;

    if(clock_gettime(CLOCK_MONOTONIC, &tmr) == -1)
        errExit("tmrTask - gettime failed");

    for(;;) {
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
        if(pthread_mutex_lock(&tmrTask_mtx) != 0)
            errExit("tmrTask - Mutex lock failed");

        INCREMENT_1MS(CO_timer1ms);

        if(CO_CAN_OK) {
            int16_t i;
            bool_t syncWas = false;

            /* Process SYNC */
            switch(CO_SYNC_process(CO->SYNC, TMR_TASK_INTERVAL, OD_synchronousWindowLength)){
                case 1: syncWas = true; break;  //immediatelly after SYNC message
                case 2: CO_CANclearPendingSyncPDOs(CO->CANmodule[0]); break; //outside SYNC window
            }

            /* Process RPDOs */
            for(i=0; i<CO_NO_RPDO; i++){
                CO_RPDO_process(CO->RPDO[i], syncWas);
            }

            /* Reenable CANrx, if it was disabled by SYNC callback */
            cbCANrx_unlockBySync();

            /* Further I/O or nonblocking application code may go here. */

            /* Verify PDO Change Of State and process PDOs */
            for(i=0; i<CO_NO_TPDO; i++){
                if(!CO->TPDO[i]->sendRequest) CO->TPDO[i]->sendRequest = CO_TPDOisCOS(CO->TPDO[i]);
                CO_TPDO_process(CO->TPDO[i], CO->SYNC, syncWas, TMR_TASK_INTERVAL);
            }
        }

        if(pthread_mutex_unlock(&tmrTask_mtx) != 0)
            errExit("tmrTask - Mutex unlock failed");

#if 0
        /* verify overflow */
        if(timer_overflow_??) {
            CO_errorReport(CO->em, CO_EM_ISR_TIMER_OVERFLOW, CO_EMC_SOFTWARE_INTERNAL, 0);
        }
#endif

    }

    return NULL;
}


/* CAN receive thread *********************************************************/
static void* CANrx_thread(void* arg) {

//    CO_CANinterrupt(CO->CANmodule[0], event, msg);

    return NULL;
}

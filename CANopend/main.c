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


/* Global variables and objects */
    volatile uint16_t   CO_timer1ms = 0U; /* variable increments each millisecond */
//    CO_EE_t             CO_EEO;           /* EEprom object */

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


/* Receive pooling function from driver */
#ifndef USE_CAN_CALLBACKS
    void CO_CANreceive(CO_CANmodule_t *CANmodule);
#endif


/* Timer thread with period 1ms */
    static void* tmr1ms_thread(void* arg);
    static int tmr1ms_fd;
    static bool_t tmr1ms_justStarted;


/* Wake up task ***************************************************************/
static void wakeUpTask(uint32_t arg){
//    RTX_Wakeup(arg);
}

/* main ***********************************************************************/
int main (int argc, char *argv[]){
    int s;
    CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
    struct timespec sleep50ms, sleep2ms;
    struct itimerspec tmr1ms_spec;
    pthread_t thread_timer1ms = 0;
    uint8_t powerOn = 1;

    printf("%s - starting ...\n", argv[0]);


    /* Init variables */
    sleep50ms.tv_sec = 0;
    sleep50ms.tv_nsec = 50000000L;
    sleep2ms.tv_sec = 0;
    sleep2ms.tv_nsec = 2000000L;


    /* Prepare timer and thread  with 1ms interval */
    tmr1ms_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if(tmr1ms_fd == -1) {
        perror("Timerfd creation failed.");
        exit(EXIT_FAILURE);
    }

    tmr1ms_spec.it_interval.tv_sec = 0;
    tmr1ms_spec.it_interval.tv_nsec = 1000000L;
    tmr1ms_spec.it_value.tv_sec = 0;
    tmr1ms_spec.it_value.tv_nsec = 1;
    tmr1ms_justStarted = true;
    s = timerfd_settime(tmr1ms_fd, 0, &tmr1ms_spec, NULL);
    if(s != 0) {
        perror("Timerfd failed to start.");
        exit(EXIT_FAILURE);
    }

    s = pthread_create(&thread_timer1ms, NULL, tmr1ms_thread, NULL);
    if(s != 0) {
        perror("Timer thread creation failed.");
        exit(EXIT_FAILURE);
    }


    /* Verify, if OD structures have proper alignment of initial values */
    if(CO_OD_RAM.FirstWord != CO_OD_RAM.LastWord) {
        fprintf(stderr, "%s - Error in CO_OD_RAM.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(CO_OD_EEPROM.FirstWord != CO_OD_EEPROM.LastWord) {
        fprintf(stderr, "%s - Error in CO_OD_EEPROM.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(CO_OD_ROM.FirstWord != CO_OD_ROM.LastWord) {
        fprintf(stderr, "%s - Error in CO_OD_ROM.\n", argv[0]);
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

        /* disable timer and CAN interrupts */
//        if(TaskTimerID) RTX_Suspend_Task(TaskTimerID);
        CO_CANsetConfigurationMode(ADDR_CAN1);


        /* initialize CANopen */
        err = CO_init();
        if(err != CO_ERROR_NO){
            perror("CANopen initialization failed.");
            exit(EXIT_FAILURE);
            /* CO_errorReport(CO->em, CO_EM_MEMORY_ALLOCATION_ERROR, CO_EMC_SOFTWARE_INTERNAL, err); */
        }


//        /* initialize eeprom - part 2 */
//        CO_EE_init_2(&CO_EEO, eeStatus, CO->SDO, CO->em);


        /* just for info */
        if(powerOn){
            CO_errorReport(CO->em, CO_EM_MICROCONTROLLER_RESET, CO_EMC_NO_ERROR, OD_powerOnCounter);
            powerOn = 0;
        }


        /* start CAN */
        CO_CANsetNormalMode(ADDR_CAN1);


        /* Configure Timer interrupt function for execution every 1 millisecond */
//        if(TaskTimerID){
//            RTX_Resume_Task(TaskTimerID);
//        }
//        else{
//            TaskTimerID = RTX_NewTask(&TaskTimerDef);
//        }

        reset = CO_RESET_NOT;
        timer1msPrevious = CO_timer1ms;
        printf("%s - running ...\n", argv[0]);

        /* Prepare function, which will wake this task after CAN SDO response is */
        /* received (inside CAN receive interrupt). */
        CO->SDO->pFunctSignal = wakeUpTask;    /* will wake from RTX_Sleep_Time() */
//        CO->SDO->functArg = RTX_Get_TaskID();  /* id of this task */

        while(reset == CO_RESET_NOT){
/* loop for normal program execution ******************************************/
            uint16_t timer1msCopy, timer1msDiff;

            timer1msCopy = CO_timer1ms;
            timer1msDiff = timer1msCopy - timer1msPrevious;
            timer1msPrevious = timer1msCopy;


            /* CANopen process */
            reset = CO_process(CO, timer1msDiff);

//            CgiLogEmcyProcess(CgiLog);


            if(CO->SDO->state == CO_SDO_ST_UPLOAD_BL_SUBBLOCK) {
                nanosleep(&sleep2ms, NULL);
            }
            else {
                nanosleep(&sleep50ms, NULL);
            }


//            CO_EE_process(&CO_EEO);
        }
    }


/* program exit ***************************************************************/
    /* delete timer task */
        //Will be removed on exit
    //TODO mutex

    /* delete objects from memory */
    CO_delete();
//    CgiLog_delete(CgiLog);
//    CgiCli_delete(CgiCli);


    printf("%s - finished.\n", argv[0]);
//    if(reset == CO_RESET_APP) BIOS_Reboot(); /* CANopen reset node */
    exit(EXIT_SUCCESS);
}


/* timer thread executes every millisecond ************************************/
static void* tmr1ms_thread(void* arg) {
    ssize_t n;
    uint64_t tmrNumExp;
    struct timespec clockStart, clockStop;
    long ticks;

    volatile static uint16_t cnt = 0, cnt2 = 0;

    for(;;) {
        /* Wait for timer to expire */
        n = read(tmr1ms_fd, &tmrNumExp, sizeof(tmrNumExp));
        if(n != sizeof(uint64_t)) {
            perror("Timer thread failed.");
            exit(EXIT_FAILURE);
        }

        CO_timer1ms += (uint16_t) tmrNumExp;

        /* verify overflow */
        if(tmr1ms_justStarted) {
            tmr1ms_justStarted = false;
        }
        else if(tmrNumExp != 1) {
            CO_errorReport(CO->em, CO_EM_ISR_TIMER_OVERFLOW, CO_EMC_SOFTWARE_INTERNAL, 0);
        }

#ifndef USE_CAN_CALLBACKS
    /* process received CAN messages */
//    CO_CANreceive(CO->CANmodule[0]);
#if CO_NO_CAN_MODULES >= 2
    CO_CANreceive(CO->CANmodule[1]);
#endif
#endif

        CO_process_RPDO(CO);

        if(++cnt == 1000) {
            cnt = 0;
            printf("tmr: %d\n", cnt2++);
        }

        CO_process_TPDO(CO);

    }

    return NULL;
}


#ifdef USE_CAN_CALLBACKS
/* CAN callback function ******************************************************/
int CAN1callback(CanEvent event, const CanMsg *msg){
    return CO_CANinterrupt(CO->CANmodule[0], event, msg);
}

int CAN2callback(CanEvent event, const CanMsg *msg){
    return CO_CANinterrupt(CO->CANmodule[1], event, msg);
}
#endif

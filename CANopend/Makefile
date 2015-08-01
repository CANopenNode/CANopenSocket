# Makefile for CANopend.


STACK_SRC =     CANopenNode/stack
STACKDRV_SRC =  CANopenNode/stack/drvTemplate
CANOPEN_SRC =   CANopenNode
APPL_SRC =      .


INCLUDE_DIRS =  $(CANOPEN_SRC)  \
              -I$(STACK_SRC)    \
              -I$(STACKDRV_SRC) \
              -I$(APPL_SRC)


SOURCES =       $(STACKDRV_SRC)/CO_driver.c     \
                $(STACKDRV_SRC)/eeprom.c        \
                $(STACK_SRC)/crc16-ccitt.c      \
                $(STACK_SRC)/CO_SDO.c           \
                $(STACK_SRC)/CO_Emergency.c     \
                $(STACK_SRC)/CO_NMT_Heartbeat.c \
                $(STACK_SRC)/CO_SYNC.c          \
                $(STACK_SRC)/CO_PDO.c           \
                $(STACK_SRC)/CO_HBconsumer.c    \
                $(STACK_SRC)/CO_SDOmaster.c     \
                $(CANOPEN_SRC)/CANopen.c        \
                $(APPL_SRC)/CO_OD.c             \
                $(APPL_SRC)/main.c
#                $(APPL_SRC)/CgiLog.c            \
#                $(APPL_SRC)/CgiOD.c             \


OBJS = ${SOURCES:%.c=%.o}
CC = gcc
CFLAGS = -Wall -I$(INCLUDE_DIRS)


.PHONY: all clean

all: canopend

clean:
	rm -f $(OBJS) canopend

%.o: %.c
	$(CC) $(CFLAGS) -c -o $*.o $<

canopend: $(OBJS)
	$(CC) $(CFLAGS)  $(OBJS) -o $@


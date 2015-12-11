# Makefile for CANopend.


STACKDRV_SRC =  ../CANopenNode/stack/socketCAN
STACK_SRC =     ../CANopenNode/stack
CANOPEN_SRC =   ../CANopenNode
APPL_SRC =      .


LINK_TARGET  =  canopend


INCLUDE_DIRS = -I$(STACKDRV_SRC) \
               -I$(STACK_SRC)    \
               -I$(CANOPEN_SRC)  \
               -I$(APPL_SRC)


SOURCES =       $(STACKDRV_SRC)/CO_driver.c     \
                $(STACKDRV_SRC)/CO_OD_storage.c \
                $(STACKDRV_SRC)/CO_Linux_tasks.c\
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
                $(APPL_SRC)/CO_command.c        \
                $(APPL_SRC)/CO_comm_helpers.c   \
                $(APPL_SRC)/CO_master.c         \
                $(APPL_SRC)/main.c


OBJS = $(SOURCES:%.c=%.o)
CC = gcc
CFLAGS = -Wall -g $(INCLUDE_DIRS)#-DCO_SINGLE_THREAD
LDFLAGS = -lrt -pthread


.PHONY: all clean

all: clean $(LINK_TARGET)

clean:
	rm -f $(OBJS) $(LINK_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LINK_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

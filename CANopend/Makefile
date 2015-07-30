# Makefile for CANopend.


CO = CANopenNode


INCLUDE_DIRS = . \
    -I$(CO)/stack \
	-I$(CO)/stack/socketCAN \
	-I$(CO)


SOURCES = $(CO)/stack/socketCAN/CO_driver.c \
	$(CO)/stack/socketCAN/eeprom.c \
	$(CO)/stack/crc16-ccitt.c \
	$(CO)/stack/CO_SDO.c \
	$(CO)/stack/CO_Emergency.c \
	$(CO)/stack/CO_NMT_Heartbeat.c \
	$(CO)/stack/CO_SYNC.c \
	$(CO)/stack/CO_PDO.c \
	$(CO)/stack/CO_HBconsumer.c \
	$(CO)/stack/CO_SDOmaster.c \
	$(CO)CANopen.c \
	CgiLog.c \
	CgiOD.c \
	CO_OD.c \
	main.c


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
	$(CC) $(CFLAGS)  $(OBJS) -o canopend


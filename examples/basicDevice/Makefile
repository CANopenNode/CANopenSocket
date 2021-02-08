# Makefile for application with CANopenNode and Linux socketCAN


DRV_SRC = ../../CANopenNode/socketCAN
CANOPEN_SRC = ../../CANopenNode
APPL_SRC = .


LINK_TARGET = basicDevice
VERSION_FILE = CO_version.h


INCLUDE_DIRS = \
	-I$(DRV_SRC) \
	-I$(CANOPEN_SRC) \
	-I$(APPL_SRC)


SOURCES = \
	$(DRV_SRC)/CO_driver.c \
	$(DRV_SRC)/CO_error.c \
	$(DRV_SRC)/CO_epoll_interface.c \
	$(DRV_SRC)/CO_storageLinux.c \
	$(CANOPEN_SRC)/301/CO_ODinterface.c \
	$(CANOPEN_SRC)/301/CO_NMT_Heartbeat.c \
	$(CANOPEN_SRC)/301/CO_HBconsumer.c \
	$(CANOPEN_SRC)/301/CO_Emergency.c \
	$(CANOPEN_SRC)/301/CO_SDOserver.c \
	$(CANOPEN_SRC)/301/CO_TIME.c \
	$(CANOPEN_SRC)/301/CO_SYNC.c \
	$(CANOPEN_SRC)/301/CO_PDO.c \
	$(CANOPEN_SRC)/301/crc16-ccitt.c \
	$(CANOPEN_SRC)/301/CO_storage.c \
	$(CANOPEN_SRC)/303/CO_LEDs.c \
	$(CANOPEN_SRC)/305/CO_LSSslave.c \
	$(CANOPEN_SRC)/CANopen.c \
	$(APPL_SRC)/CO_application.c \
	$(APPL_SRC)/OD.c \
	$(APPL_SRC)/testingVariables.c \
	$(DRV_SRC)/CO_main_basic.c


OBJS = $(SOURCES:%.c=%.o)
CC ?= gcc
OPT =
OPT += -g
#OPT += -O2
#OPT += -DCO_SINGLE_THREAD
#OPT += -DCO_CONFIG_DEBUG=0xFFFF
#OPT += -Wextra -Wshadow -pedantic -fanalyzer
#OPT += -DCO_USE_GLOBALS
#OPT += -DCO_MULTIPLE_OD
CFLAGS = -Wall -DCO_DRIVER_CUSTOM $(OPT) $(INCLUDE_DIRS)
LDFLAGS =
LDFLAGS += -g
LDFLAGS += -pthread

#Options can be also passed via make: 'make OPT="-g" LDFLAGS="-pthread"'


.PHONY: all clean

all: clean version $(LINK_TARGET)

clean:
	rm -f $(OBJS) $(LINK_TARGET) $(VERSION_FILE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LINK_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

version:
	echo "#define CO_VERSION_CANOPENNODE \"$(shell git -C $(CANOPEN_SRC) describe --tags --long --dirty)\"" > $(VERSION_FILE)
	echo "#define CO_VERSION_APPLICATION \"$(shell git describe --tags --long --dirty)\"" >> $(VERSION_FILE)

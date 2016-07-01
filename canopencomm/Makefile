# Makefile for CANopenCommand.


APPL_SRC =      .


LINK_TARGET  =  canopencomm


INCLUDE_DIRS = -I$(APPL_SRC)


SOURCES =       $(APPL_SRC)/CANopenCommand.c


OBJS = $(SOURCES:%.c=%.o)
CC ?= gcc
CFLAGS = -Wall -g $(INCLUDE_DIRS)
LDFLAGS =


.PHONY: all clean

all: clean $(LINK_TARGET)

clean:
	rm -f $(OBJS) $(LINK_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LINK_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

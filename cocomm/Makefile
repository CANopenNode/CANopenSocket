# Makefile for CANopenCommand.

APPL_SRC = .
LINK_TARGET = cocomm
INCLUDE_DIRS = -I$(APPL_SRC)
SOURCES = $(APPL_SRC)/cocomm.c

OBJS = $(SOURCES:%.c=%.o)
CC ?= gcc
OPT = -g
#OPT = -g -pedantic -Wshadow -fanalyzer
CFLAGS = -Wall $(OPT) $(INCLUDE_DIRS)
LDFLAGS =

.PHONY: all clean

all: clean $(LINK_TARGET)

clean:
	rm -f $(OBJS) $(LINK_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LINK_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

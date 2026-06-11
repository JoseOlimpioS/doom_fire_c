#
# $Id:$
#
# Makefile
#
# Written by Gustavo Bacagine <gustavo.bacagine@protonmail.com>
#
# Description: Makefile for doom_fire - KDevelop Integration Feature
#
# Date: Jun 2026
#

TARGET  = doom_fire

# Base CFLAGS
CFLAGS  =  -Wall -Wextra -ansi -pedantic -D_XOPEN_SOURCE=501
CC      = gcc

ifdef LINUX
  CFLAGS  += -DLINUX
endif

ifdef _WIN32
  CFLAGS  += -D_WIN32
  LDLIBS  += -lws2_32
  TARGET     := $(TARGET).exe
endif

ifdef DEBUG
  CFLAGS  += -DDEBUG -g -O0 -fsanitize=address 
  LDFLAGS += -fsanitize=address -fno-omit-frame-pointer
else
  CFLAGS  += -O2
endif

OBJS = doom_fire.o
SRC = doom_fire.c

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: clean check_goto $(TARGET)

clean:
	rm -rvf *.o

distclean: clean
	rm -rvf $(TARGET)

check_goto:
	@grep -rn --color "goto" $(SRC) && { echo "\033[1;31mError:\033[m '\033[1;33mgoto\033[m' detected in source file!\nCompilation aborted..." && grep -rn --color "goto" $(SRC) && echo "Error: 'goto' detected in source file!\nCompilation aborted..."; exit 1;} || true


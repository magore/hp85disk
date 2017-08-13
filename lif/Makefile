#  @file Makefile for LIF utilities
#
#  @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
#  @see http://github.com/magore/hp85disk
#  @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
#
# @par Edit History
# - [1.0]   [Mike Gore]  Initial revision of file.

GIT_VERSION := $(shell stat -c%x ../update.last 2>/dev/null)
LOCAL_MOD := $(shell ls -rt teledisk/*.[ch] *.[ch] | tail -1 | xargs stat -c%x)

DIR=$(shell pwd)

# Optional Stand Alone TELEDISK to LIF conversion support
# Note this will not be part of the hp85disk AVR code because of ram memory limitations
TELEDISK=1


CFLAGS = -O -D_GNU_SOURCE -g -static
CFLAGS += -DLIF_STAND_ALONE
CFLAGS += -DGIT_VERSION="\"$(GIT_VERSION)\""
CFLAGS += -DLOCAL_MOD="\"$(LOCAL_MOD)\""

SRC =  lifsup.c lifutils.c teledisk/td0_lzss.c
LIBS = -lm

ifeq ($(TELEDISK),1)
	# Add teledisk support
	CFLAGS += -I. -Iteledisk
	CFLAGS += -DTELEDISK
	CFLAGS += -L${DIR}/teledisk
	SRC += td02lif.c
endif


BIN = lif
LIB =

# Optional teledisk support
ifeq ($(TELEDISK),1)
    BIN += td02lif
endif

all:	$(BIN) $(LIB)

install:	all
	install -s lif /usr/local/bin/lif
	install -s td02lif /usr/local/bin/lif

td02lif:    ${SRC}
	gcc ${CFLAGS} -o td02lif ${SRC} ${LIBS}

# Create a stand alone test program called printf
lif:	lifutils.c
	gcc $(CFLAGS) ${SRC} -o lif ${LIBS}

BIN_EXE := $(addsuffix .exe,${BIN})

clean:
	rm -f ${BIN} ${BIN_EXE} ${LIB}


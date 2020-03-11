#  @file Makefile for LIF utilities
#
#  @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
#  @see http://github.com/magore/hp85disk
#  @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
#
# @par Edit History
# - [1.0]   [Mike Gore]  Initial revision of file.

GIT_VERSION := $(shell stat -c%x ../update.last 2>/dev/null)

DIR=$(shell pwd)

# HPDir to hpdisk configuration generator

CFLAGS = -O -D_GNU_SOURCE -g -static
CFLAGS += -DMKCFG_STAND_ALONE
CFLAGS += -DGIT_VERSION="\"$(GIT_VERSION)\""
CFLAGS += -DLOCAL_MOD="\"$(LOCAL_MOD)\""

SRC =  mkcfg.c
LIB =
LIBS = -lm

BIN = mkcfg
LIB =

all:    $(BIN) $(LIB)

install:        all
	install -s mkcfg /usr/local/bin/mkcfg
	install -p hpdir.ini /usr/local/bin/hpdir.ini

mkcfg:    ${SRC}
	gcc ${CFLAGS} -o mkcfg ${SRC} ${LIBS}


BIN_EXE := $(addsuffix .exe,${BIN})

clean:
	rm -f ${BIN} ${BIN_EXE} ${LIB}


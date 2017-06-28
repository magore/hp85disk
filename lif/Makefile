all:	lif

GIT_VERSION := $(shell stat -c%x ../update.last 2>/dev/null)
LOCAL_MOD := $(shell ls lifutils.c | tail -1 | xargs stat -c%x)

CFLAGS = -DLIF_STAND_ALONE -g
CFLAGS += -DGIT_VERSION="\"$(GIT_VERSION)\""
CFLAGS += -DLOCAL_MOD="\"$(LOCAL_MOD)\""

install:	lif
	install -s lif /usr/local/bin/lif

# Create a stand alone test program called printf
lif:	lifutils.c
	gcc $(CFLAGS) lifutils.c -o lif -lm

clean:
	rm -f lif

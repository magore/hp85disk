#  @file   Makefile
#  @brief  Build TeleDisk decoder Library for optional addition to my LIF utilities 
#          See Credits for support code used
#  @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved. GPL
#  @see http://github.com/magore/hp85disk
#  @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details
# 
#  @Credits
#      * We use stripped down version of teledisk_loader reader code to decode TELEDISK format
#        * See: https://github.com/jfdelnero/libhxcfe/tree/master/sources/loaders/teledisk_loader
#        * Part of HxCFloppyEmulator project Copyright (C) 2006-2014 Jean-Franois DEL NERO
#      * LZSS and Teledisk Documention Copyright 2007-2008 Dave Dunfield All rights reserved.
#      * CRC code By Ashley Roll Digital Nemesis Pty Ltd www.digitalnemesis.com
# 

CFLAGS = -O -D_GNU_SOURCE

CFLAGS += -I.
CFLAGS += -I..
CFLAGS += -DLIF_STAND_ALONE -g

LIB =  libtd02lif.a
OBJ = crc.o td0_lzss.o teledisk_loader.o
LIB = libtd02lif.a

all:	${LIB}


crc.o:	 crc.c
	gcc ${CFLAGS} -c crc.c -o crc.o

td0_lzss.o:	 td0_lzss.c
	gcc ${CFLAGS} -c td0_lzss.c -o td0_lzss.o

teledisk_loader.o:	 teledisk_loader.c
	gcc ${CFLAGS} -c teledisk_loader.c -o teledisk_loader.o

libtd02lif.a: ${OBJ}
	   ar rv ${LIB} crc.o td0_lzss.o teledisk_loader.o
	   ranlib ${LIB} 
clean:
	rm -f ${LIB} ${OBJ}

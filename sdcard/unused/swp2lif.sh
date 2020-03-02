#!/bin/sh -
#
# Created by Vassilis PREVELAKIS
# Series80 groups
# https://groups.io/g/hpseries80/topic/dondisks/655979

FN=$1
FO=$2

FB=`echo $FN | sed -n 's/\..*$//p'`
printf "\200\0%-6.6s\0\0\0\2\10\0\0\0" $FB > $FO
dd if=$FN of=/dev/stdout bs=1 skip=16 count=240 >> $FO
dd if=/dev/zero of=/dev/stdout bs=256 count=1  >> $FO
printf "%-10.10s\340\211\0\0\0\4" $FB >> $FO
dd if=$FN of=/dev/stdout bs=1 skip=272 count=240 >> $FO
dd if=/dev/zero of=/dev/stdout bs=256 count=1  >> $FO
dd if=$FN of=/dev/stdout bs=256 skip=2 >> $FO

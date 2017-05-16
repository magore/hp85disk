#!/bin/bash
#
for i in $*
do
	if [ ! -f "$i" ]
	then
		continue
	fi
	sed -i -e "s/listening == SS80_MLA/SS80_is_MLA(listening)/g" "$i"
	sed -i -e "s/listening == AMIGO_MLA/AMIGO_is_MLA(listening)/g" "$i"
	sed -i -e "s/listening == PRINTER_MLA/PRINTER_is_MLA(listening)/g" "$i"
	sed -i -e "s/listen_last == PRINTER_MLA/PRINTER_is_MLA(listen_last)/g" "$i"

	sed -i -e "s/ch == SS80_MLA/SS80_is_MLA(ch)/g" "$i"
	sed -i -e "s/ch == AMIGO_MLA/AMIGO_is_MLA(ch)/g" "$i"
	sed -i -e "s/ch == PRINTER_MLA/PRINTER_is_MLA(ch)/g" "$i"

	sed -i -e "s/talking == SS80_MTA/SS80_is_MTA(talking)/g" "$i"
	sed -i -e "s/talking == AMIGO_MTA/AMIGO_is_MTA(talking)/g" "$i"
	sed -i -e "s/talking == PRINTER_MTA/PRINTER_is_MTA(talking)/g" "$i"

	sed -i -e "s/ch == SS80_MTA/SS80_is_MTA(ch)/g" "$i"
	sed -i -e "s/ch == AMIGO_MTA/AMIGO_is_MTA(ch)/g" "$i"
	sed -i -e "s/ch == PRINTER_MTA/PRINTER_is_MTA(ch)/g" "$i"

	sed -i -e "s/ch == SS80_MSA/SS80_is_MSA(ch)/g" "$i"
	sed -i -e "s/ch == AMIGO_MSA/AMIGO_is_MSA(ch)/g" "$i"
done

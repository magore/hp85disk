#!/bin/bash
#

# The following drive definitions MUST match those in hpdisk.cfg
#
# Drives the script knows about
# AMIGO Drives: HP9121D 
# 14 Directory Sectors, 1120 Sectors total image size
# SS80 Drives: HP9134L
# 123 Directory Sectors, 58176 Sectors total image size

# ======================================================
# Title: Display the absolute path of a file - uses echo
# Arguments: path
# Notes: if argument is -bash then return absolute of current directory
# Example:
#     absolute ~/file.txt

if [ -f lif-functions.sh ]
then
	BINHOME=.
else
	BINHOME=$(echo -n $(dirname $(readlink -f "$0" )) )
fi

# echo "BINHOME:[$BINHOME]"
source "$BINHOME"/lif-functions.sh
SDCARD="$BINHOME/.."
CONFIGS="$SDCARD/configs"

# Note the LIF image file names and generated configuration files must match in each section below
echo "Creating AMIGO and SS80 disks"
declare D
for D in 0 1 2 3
do
	# The generated file name below must match these LIF images 
	SS80_BLOCKS=$(mkcfg -m 9134D -b)
	runlog lif create "ss80-"$D.lif "SS80-"$D 123 $SS80_BLOCKS
	AMIGO_BLOCKS=$(mkcfg -m 9121 -b)
	runlog lif create "amigo"$D.lif "AMIGO"$D 14 $AMIGO_BLOCKS

	declare S
	for S in $@
	do
		if [ -d "$S" ]
		then
			echo "Adding Directory:$S"
			runlog lif_add_files "ss80-"$D.lif "$S"
			runlog lif_add_files "amigo"$D.lif "$S"
		else
			echo "Skipping file: $S"
		fi
	done
done


echo "Creating AMIGO ONLY Config amigo.cfg"
cat "$CONFIGS/header.cfg" >"$SDCARD/amigo.cfg"
declare D
for D in 0 1 2 3 
do
	#               TYPE    FILE         PPR/ADDRESS
	mkcfg -m 9121 -f /amigo$D.lif -a $D >>"$SDCARD/amigo.cfg"
done

echo "Creating COMBINED Config hpdisk.cfg"
cat "$CONFIGS/header.cfg" >"$SDCARD/hpdisk.cfg"

echo "Creating AMIGO records in hpdisk.cfg"
declare D
for D in 0 1 
do
	# The generated file name must match the LIF images previously created above
	#               TYPE    FILE         PPR/ADDRESS
	mkcfg -m 9121 -f /amigo$D.lif -a $D >>"$SDCARD/hpdisk.cfg"
done

echo "Creating SS80 records in hpdisk.cfg"
for D in 0 1 
do
	# The generated file name must match the LIF images previously created above
    S=$(($D+2))
	#               TYPE    FILE         PPR/ADDRESS
    mkcfg -m 9134D -f /ss80-$D.lif -a $S >>"$SDCARD/hpdisk.cfg"
done

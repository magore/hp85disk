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
	# runlog lif create "amigo"$D.lif "AMIGO"$D 14 1120
	# runlog lif create "ss80-"$D.lif "SS80-"$D 256 58176
	runlog lif createdisk "amigo"$D.lif "AMIGO"$D 9121
	runlog lif createdisk "ss80-"$D.lif "SS80-"$D 9134d
done

echo "Adding files"
for D in 0 1 2 3
do
	declare S
	for S in $@
	do
		if [ -d "$S" ]
		then
			echo "Adding Directory:[$S]"
			lif_add_files "ss80-"$D.lif "$S"
			echo "Adding Directory:[$S]"
			lif_add_files "amigo"$D.lif "$S"
		else
			echo "Skipping file: $S"
		fi
	done
done

echo "Listing files"
declare D
for D in 0 1 2 3
do
	lif dir "ss80-"$D.lif
	lif dir "amigo"$D.lif
done


echo "Creating AMIGO ONLY Config amigo.cfg"
cat "$CONFIGS/debug_defaults.cfg" >"$SDCARD/amigo.cfg"
cat "$CONFIGS/printer_defaults.cfg" >>"$SDCARD/amigo.cfg"
declare D
for D in 0 1 2 3 
do
	#               TYPE    FILE         PPR/ADDRESS
	mkcfg -s -m 9121 -f /amigo$D.lif -a $D -p $D >>"$SDCARD/amigo.cfg"
done

echo "Creating COMBINED Config hpdisk.cfg"
cat "$CONFIGS/debug_defaults.cfg" >"$SDCARD/hpdisk.cfg"
cat "$CONFIGS/printer_defaults.cfg" >>"$SDCARD/hpdisk.cfg"
cat "$CONFIGS/ss80_defaults.cfg" >>"$SDCARD/hpdisk.cfg"

echo "Creating AMIGO records in hpdisk.cfg"
declare D
for D in 0 1 
do
	# The generated file name must match the LIF images previously created above
	#               TYPE    FILE         PPR/ADDRESS
	mkcfg -s -m 9121 -f /amigo$D.lif -a $D -p $D >>"$SDCARD/hpdisk.cfg"
done

echo "Creating SS80 records in hpdisk.cfg"
for D in 0 1 
do
	# The generated file name must match the LIF images previously created above
    S=$(($D+2))
	#               TYPE    FILE         PPR/ADDRESS
    mkcfg -s -m 9134D -f /ss80-$D.lif -p $S -a $S >>"$SDCARD/hpdisk.cfg"
done

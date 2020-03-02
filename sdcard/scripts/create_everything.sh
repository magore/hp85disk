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

BINHOME=$(echo -n $(dirname $(readlink -f "$0" )) )
# echo "BINHOME:[$BINHOME]"
source "$BINHOME"/lif-functions.sh

echo "Creating AMIGO and SS80 disks"
declare D
for D in 1 2 3 4
do
	runlog lif create "amigo"$D.lif "$AMIGO"$D 14 1120
	runlog lif create "ss80-"$D.lif "SS80-"$D 123 58176

	declare S
	for S in $@
	do
		if [ -d "$S" ]
		then
			echo "Adding Directory:$S"
			runlog lif_add_files "amigo"$D.lif "$S"
			runlog lif_add_files "ss80-"$D.lif "$S"
		else
			echo "Skipping file: $S"
		fi
	done
done

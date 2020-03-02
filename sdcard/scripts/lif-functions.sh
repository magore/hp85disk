#!/bin/bash
#

# The following drive definitions MUST match those in hpdisk.cfg
#
# Drives the script knows about
# AMIGO Drives: HP9121D 
# 14 Directory Sectors, 1120 Sectors total image size
# SS80 Drives: HP9134L
# 123 Directory Sectors, 58176 Sectors total image size

# runlog Shell Function
# Echo a command and run it
# 

# Shell Function
# Add files in a directory to a LIF image
# Usage: lif_addfiles LIF_image directory
#
#   This function adds ALL text and LIF files found inside specified directory
#   LIF files must be for the HP85 and have just file in them of the same name

#   Files IN LIF volumes should just have one name in them
#     === IMPORTANT: ALL file names without their suffix MUST be UNIQUE ===
#

runlog()
{
	echo "$*"
	"$@"
}

error()
{
	echo "Error: $*"
	exit 1
}

warning()
{
	echo "Warning: $*"
	exit 1
}

# ======================================================
# Title: Display the absolute path of a file - uses echo
# Arguments: path
# Notes: if argument is -bash then return absolute of current directory
# Example:
#     absolute ~/file.txt
absolute()
{
    if [ -z "$1" ]
    then
        fatal " $FUNCNAME: missing argument: [$0]"
    fi

    # FIXME if the argument is -bash, then is this the correct course of action?
    if [ "$1" = "-bash" ]
    then
        echo -n $(readlink -f "$(pwd)")
        return
    fi
    # Our full path name
    echo -n $(readlink -f "$1")
}

# Given a LIF file and a named file in it return its type
lif_filetype()
{
	declare LIF="$1"
	declare NAME="$2"

	if [ -z "$LIF" -o -z "$NAME" ]
	then
		warning "Usage: $FUNCNAME LIF_image FILE_name_in_image"
		return 1
	fi

	if [ ! -f "$LIF" ]
	then
		warning "$LIF not found"
		return 1
	fi

	# A list of all files in a lif
	echo =$(lif dir "$LIF" | egrep -E -i '[0-9A-F][0-9A-F][0-9A-F][0-9A-F]h' | \
		grep -i -w "$NAME" | sed -e "s/\..*$//" | tr '\n' ' ' | sed -e "s/ $//")
}

lif_add_files()
{
	declare LIF="$1"
	declare DIR="$2"

	if [ -z "$LIF" -o -z "$DIR" ]
	then
		error "Usage: $FUNCNAME LIF_file Directory"
	fi

	if [ ! -d "$DIR" ]
	then
		error "$FUNCNAME Directory missing"
	fi

	# echo "LIF:[$LIF] DIR:[$DIR]"

	# Convert ascii text files in to HP85 ASCII BASIC plain text files E010 DTA8x format
	declare TXT=$(ls  "$DIR" | egrep -E -i '\.txt$' | tr '\n' ' ' | sed -e "s/ $//")
	echo "TXT:[$TXT]"
	# We can convert TXT format to E010 DTA8x ASCII BASIC LIF file
	for i in $TXT
	do
		
		declare NAME=$(basename $i | sed -s "s/\..*$//")
		runlog lif add "$LIF" $NAME "$DIR/$i"
	done

	# Copy single file LIF image source files into new LIF image
	# We assume that you are adding HP85 programs 
	declare LIFS=$(ls "$DIR" | egrep -E -i '\.lif$' | tr '\n' ' ' | sed -e "s/ $//")

	# echo "LIFS:[$LIFS]"

	for i in $LIFS
	do
		declare NAME=$(basename $i | sed -s "s/\..*$//")
		runlog lif addbin "$LIF" "$NAME" "$DIR"/"$i"
	done
}


# Extract a files from a LIF image into single file LIG images or text files
lif_extract_all()
{
	declare LIF="$1"
	declare DIR="$2"
	if [ -z "$LIF" -o -z "$DIR" ]
	then
		error "Usage: $FUNCNAME LIF_image DIR_target"
		return 1
	fi
	if [ ! -d "$DIR" ]
	then
		echo "Creating Target Directory:[$DIR]"
		mkdir -p "$DIR"
	fi

	# Get a list of all the files in the LIF file
	declare FILES=$(lif dir "$LIF" | egrep -E -i '[0-9A-F][0-9A-F][0-9A-F][0-9A-F]h' | \
		cut -d ' ' -f1 | sed -e "s/\..*$//" | tr '\n' ' ' | sed -e "s/ $//")

	# echo "FILES:[$FILES]"

	for i in $FILES
	do
		declare TYPE=$(lif_filetype $LIF $i | egrep -E -i 'E01[0123]h')
		if [ -n "$TYPE" ]
		then
			lif extract "$LIF" $i "$DIR"/$i.txt
		else
			lif extractbin "$LIF" $i $DIR/$i.lif
		fi
	done
}

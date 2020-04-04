# ========================================
# HP85 Disk Emulator is Copyright 2014 Mike Gore
#
# This file is part of HP85 Disk Emulator
# HP85 Disk Emulator is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# HP85 Disk Emulator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# ========================================

### Project name (also used for output file name)
PROJECT = gpib

# ==============================================
# Debug serial port for firmware command interface
export BAUD ?= 115200UL
# BAUD ?= 500000UL

### Serial Port for emulator user interface
export PORT ?= /dev/ttyUSB0
# ==============================================

### Target AVR device used by GCC for this project
export DEVICE ?= atmega1284p
export F_CPU  ?= 20000000 

# avrdude device programmer name as known by avrdude
#     Note: avrdude -c list 
#        Will display all of ALL supported pogrammers
# I am using the atmelice_isp  Atmel-ICE (ARM/AVR) in ISP mode
export AVRDUDE_ISP ?= atmelice_isp

# AVRDUDE ISP PORT
export AVRDUDE_PORT ?= usb

# avrdude device name
export AVRDUDE_DEVICE ?= m1284

# avrdude programming speed
export AVRDUDE_SPEED ?= 5

# optiboot support
export OPTIBOOT      ?= 1

# ==============================================
# AVR programming FUSES
# Fuse bits for ATMEGA1284P
# See: http://www.engbedded.com/fusecalc/
#
# NOTE: We MUST disable JTAG so we can use all of Port C GPIO bits
# FYI: flash might randomly fail when JTAG is disabled - so just retry it
export fuses=-U lfuse:w:0xd6:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m


# ==============================================
# Project HARDWARE OPTIONS
### Board Version Specific defines
#
# My original V1 release from 2015 to 2019
# 1 for original hardware release 2015 to 2019
#
# 2 for Jay Hamlin V2 circuit board design
BOARD 					?= 2

# Parallel Poll Response (PPR) bit order
#   The bus SPEC's require the disk 1 replies on by pulling BIT 8 low .. disk 8 pulls BIT 1 low
#   So the bit order is reversed!
#
# Jay Hamlin V2 circuit board design 
#   For this version we decided to do the PPR bit order swap in software 
#   I now have documentation, in the new PPR functions, to help remind me why I did it 8-)
PPR_REVERSE_BITS		?= 1

# My original V1 release from 2015 to 2019 did the PPR reversal in hardware 
ifeq ($(BOARD),1)
    PPR_REVERSE_BITS=0
endif

# ==============================================
# Hardware Addon Options 
# Evaluate interrupt driven I2C code - compiled bit it is not being used yet
I2C_SUPPORT 			?= 1

# Do we have an RTC attachaed - code currently assumes DS1307 or DS3231
RTC_SUPPORT 			?= 1

# Do we have an I2C LCD
LCD_SUPPORT 			?= 1
# ==============================================

# Enable AMIGO support Code
# 0 Disables 
AMIGO 					?= 1

### Source files and search directory
# 0 Disables 
FATFS_SUPPORT			?= 1
FATFS_TESTS				?= 1

# Extended user interactive fatfs tests
# 0 Disables 
FATFS_UTILS_FULL		?= 0

#GPIB Extended tests
# 0 Disables 
GPIB_EXTENDED_TESTS		?= 0

# Extended user interactive posix tests
# 0 Disables 
POSIX_TESTS=1
POSIX_EXTENDED_TESTS 	?= 0

#LIF support for modifying LIF disik images used by the emulator
# 0 Disables 
LIF_SUPPORT 			?= 1

# ==============================================
# Source filess to build the project 

HARDWARE = \
	hardware/hal.c \
	hardware/ram.c \
	hardware/delay.c \
	hardware/rs232.c \
	hardware/spi.c \
	hardware/TWI_AVR8.c 

# Evaluate interrupt driven I2C code - compiled bit it is not being used yet
ifeq ($(I2C_SUPPORT),1)
	HARDWARE += hardware/i2c.c 
endif

# Do we have an RTC attached
ifeq ($(RTC_SUPPORT),1)
	HARDWARE += hardware/rtc.c 
endif

# Do we have an I2C LCD
ifeq ($(LCD_SUPPORT),1)
	HARDWARE += hardware/LCD.c 
	HARDWARE += display/lcd_printf.c
endif

LIB = \
	lib/stringsup.c \
	lib/timer_hal.c \
	lib/timer.c \
	lib/time.c \
	lib/queue.c 

PRINTF = \
	printf/printf.c \
	printf/mathio.c 

FATFS = \
	fatfs/ff.c  \
	fatfs/ffsystem.c  \
	fatfs/ffunicode.c \
	fatfs.hal/diskio.c  \
	fatfs.hal/mmc.c  \
	fatfs.hal/mmc_hal.c  \
	fatfs.sup/fatfs_sup.c  

ifeq ($(FATFS_TESTS),1)
	FATFS += fatfs.sup/fatfs_tests.c  
endif

GPIB   = \
	gpib/gpib_hal.c \
	gpib/gpib.c \
	gpib/gpib_task.c \
	gpib/gpib_tests.c \
	gpib/drives.c \
	gpib/drives_sup.c \
	gpib/ss80.c \
	gpib/amigo.c \
	gpib/printer.c \
    gpib/controller.c

POSIX = 
	POSIX += posix/posix.c

ifeq ($(POSIX_TESTS),1)
	POSIX += posix/posix_tests.c  
endif

LIF = 
ifeq ($(LIF_SUPPORT),1)
	LIF +=lif/lifsup.c
	LIF +=lif/lifutils.c
endif

# Define ALL of the source files
CSRC = \
	$(HARDWARE) \
	$(LIB) \
	$(PRINTF) \
	$(FATFS) \
	$(POSIX) \
	$(GPIB) \
	$(LIF) \
	main.c 

# Use GIT last modify time if we have it 
# GIT_VERSION := $(shell git log -1 2>&1 | grep "^Date:")
# update.last is safer to use, the file is touched by my git commit script
GIT_VERSION := $(shell stat -c%x update.last 2>/dev/null)
#LOCAL_MOD := $(shell ls -rt $(CSRC) | tail -1 | xargs stat -c%x)
LOCAL_MOD := $(shell ls -rt */*[ch] | tail -1 | xargs stat -c%x)

# Assembler sources
ASRC    = 

### Optimization level (0, 1, 2, 3, 4 or s)
# We MUST use s for this project - otherwise it is too big
OPTIMIZE = s

CC = avr-gcc
### C Standard level (c89, gnu89, c99 or gnu99)
CSTD = gnu99

### Include dirs, library dirs and definitions
LIBS    =
LIBDIRS =
INCDIRS =. hardware lib printf fatfs fatfs.hal fatfs.sup gpib posix lif

DEFS    = AVR F_CPU=$(F_CPU) SDEBUG=0x11 SPOLL=1 $(DEVICE) \
	DEFINE_PRINTF \
	FLOATIO 

ifeq ($(OPTIBOOT),1)
	DEFS += OPTIBOOT
endif

# Default Controller values
DEFS += HP9134D



ifeq ($(AMIGO),1)
	DEFS += AMIGO 
endif

DEFS += BAUD=$(BAUD)

ifeq ($(I2C_SUPPORT),1)
	DEFS += I2C_SUPPORT
endif

ifeq ($(RTC_SUPPORT),1)
	DEFS += RTC_SUPPORT
endif

ifeq ($(FATFS_SUPPORT),1)
	DEFS += FATFS_SUPPORT
	DEFS += DRV_MMC=0
endif

ifeq ($(FATFS_UTILS_FULL),1)
	DEFS += FATFS_UTILS_FULL
endif

ifeq ($(FATFS_TESTS),1)
	DEFS += FATFS_TESTS
endif

# We have an I2C LCD
ifeq ($(LCD_SUPPORT),1)
	DEFS += LCD_SUPPORT
endif

ifeq ($(LIF_SUPPORT),1)
	DEFS += LIF_SUPPORT
endif

ifeq ($(POSIX_TESTS),1)
	DEFS += POSIX_TESTS
endif

ifeq ($(POSIX_EXTENDED_TESTS),1)
	DEFS += POSIX_TESTS
endif

# ==============================================
# BOARD Specific Defines
# Version 2 Circuit Board by Jay Hamlin
DEFS += BOARD=$(BOARD)
# V1 circuit design by Mike Gore reverses PPR bits in hardware
# V2 Circuit Board by Jay Hamlin uses software
DEFS += PPR_REVERSE_BITS=$(PPR_REVERSE_BITS)
# ==============================================


ADEFS   =

### Warning contorls
WARNINGS = all extra

### Output file format (ihex, bin or both) and debugger type
OUTPUT  = ihex
DEBUG   = dwarf-2

### Programs to build porject
CC      = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE    = avr-size
NM      = avr-nm


# Define all object files
COBJ      = $(CSRC:.c=.o) 
AOBJ      = $(ASRC:.S=.o)
#COBJ      := $(addprefix $(OBJDIR)/,$(COBJ))
#AOBJ      := $(addprefix $(OBJDIR)/,$(AOBJ))
#PROJECT   := $(PROJECT)


# Next to examples are used to set memory sections ofr MEGA series AVRs only
# COMMON = -mmcu=$(DEVICE) -Wl,--section-start,.data=0x801100,--defsym=__heap_start=0x803000,--defsym=__heap_end=0x807fff
# COMMON = -Wl,--section-start,.data=0x801100,--defsym=__heap_end=0x807fff

## Options common to compile, link and assembly rules
COMMON = -Wl,-u,vfprintf -lprintf_flt -lm

# Flags for C files
CFLAGS = $(COMMON)
CFLAGS += -DGIT_VERSION="\"$(GIT_VERSION)\""
CFLAGS += -DLOCAL_MOD="\"$(LOCAL_MOD)\""
CFLAGS += -Wall 
CFLAGS += -std=$(CSTD)
CFLAGS += -funsigned-char
CFLAGS += -g$(DEBUG)
CFLAGS += -mmcu=$(DEVICE)
CFLAGS += -O$(OPTIMIZE) -mcall-prologues
CFLAGS += $(addprefix -W,$(WARNINGS))
CFLAGS += $(addprefix -I,$(INCDIRS))
CFLAGS += $(addprefix -I,$(INCDIRS)/include)
CFLAGS += $(addprefix -D,$(DEFS))
CFLAGS += -MD
CFLAGS += -Wp,-M,-MP,-MT,$(*F).o,-MF,dep/$(@F).d
CFLAGS += -mrelax
CFLAGS += -Wl,-Map,$(PROJECT).map
CFLAGS += -ffunction-sections
CFLAGS += -Wl,-gc-sections
CFLAGS += -Waddr-space-convert

# Assembler flags
ASFLAGS += $(COMMON)
ASFLAGS += $(addprefix -D,$(ADEFS)) -Wa,-gstabs,-g$(DEBUG)
ALL_ASFLAGS = -mmcu=$(DEVICE) -I. -x assembler-with-cpp $(ASFLAGS)

# Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map,$(PROJECT).map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


SRCS = $(CSRC)
PROGS = lif mkcfg hardware/baudrate

all: version $(LIBS) build size $(PROGS) 

# =======================================
.PHONY: isp
isp:
	export fuses

.PHONY: arduino
arduino:
	export fuses=""
# =======================================
.PHONY: optiboot
optiboot:
	export OPTIBOOT=1
	make -C optiboot optiboot

.PHONY: optiboot
install_optiboot:
	export OPTIBOOT=1
	make -C optiboot install_optiboot

# =======================================
# Default target.
#Example way of creating a current year string for  a #define
# DATE="$(shell date +%Y)"
# .PHONY: date
# date:
#	@echo "#define _YEAR_ \"$(DATE)\"" >date.h

main.c:	

# =======================================
hardware/baudrate:  hardware/baudrate.c
	gcc hardware/baudrate.c -o hardware/baudrate -lm

# =======================================
.PHONY: term
term:   
	./term $(BAUD) $(PORT)

# =======================================
.PHONE: hogs
hogs:
	nm -n -P -v gpib.elf | sort -k 4n | tail -40	

# =======================================
.PHONY: sdcard
sdcard: install
	-rm -f sdcard/*\.lif
	cd sdcard; ./create_images.sh

# =======================================
.PHONY: release
release: all install
	# Save the results under release
	cp -p $(PROJECT).*   release/build
	cp -p sdcard/*\.lif  release/sdcard
	cp -p sdcard/*\.cfg  release/sdcard
	cp -p sdcard/*\.ini  release/sdcard

# =======================================
.PHONY: help
help:
	@echo
	@echo 'Building Commands'
	@echo "    make install           - builds and installs all command line utilities"
	@echo "    make sdcard            - builds all sdcard images and creates default hpdisk.cfg and amigo.cfg files"
	@echo "    make release           - builds all code and copies files to the release folder"
	@echo "    make clean             - cleans all generated files"
	@echo "    make                   - builds all code"
	@echo
	@echo 'Programming using an 6 wire ISP - installs optiboot'
	@echo "    make install_optiboot  - install optiboot boot loaded using an ISP"
	@echo "    make flash-isp         - build and flash the code using an ISP"
	@echo "    make flash-isp-release - flash the release code using an ISP"
	@echo "    make verify-isp        - verify code using an ISP"
	@echo "    make verify-isp-release- verify release code using an ISP"
	@echo
	@echo 'Programming using the built in optiboot programmer'
	@echo '    IMPORTANT - you MUST press RESET on the hp85disk board JUST BEFORE issuing these commands'
	@echo '        On your computer type in the make command without pressing Enter afterwards'
	@echo '        Then press RESET the button and next press Enter quickly afterwards'
	@echo "    make flash             - build and flash the code using built in optiboot programmer"
	@echo "    make flash-release     - flash the release code using built in optiboot programmer"
	@echo "    make verify            - verify code using built in optiboot programmer"
	@echo "    make verify-release    - verify release code using built in optiboot programmer"
	@echo
	@echo 'Programming using an 6 wire ISP - WITHOUT installing optiboot'
	@echo '    IMPORTANT - you will not be able to use non isp flashing modes later on'
	@echo '       Makes booting and flashing process slightly faster'
	@echo "    make flash-isp-noboot         - build and flash the code using an ISP"
	@echo "    make flash-isp-noboot-release - flash the release code using an ISP"
	@echo

# =======================================
# ISP flashing - NOTE: WE ALWAYS INSTALL optiboot
# We do NOT erase before flashing
# install_optiboot erases the chip
# install_optiboot sets our fuses!
flash-isp: isp all install_optiboot
	avrdude -c $(AVRDUDE_ISP) -P $(AVRDUDE_PORT) -p $(AVRDUDE_DEVICE) -D -F -B $(AVRDUDE_SPEED) -U flash:w:$(PROJECT).hex:i
	./term $(BAUD) $(PORT)
	#./miniterm $(BAUD) $(PORT)

flash-isp-fast: isp all install_optiboot
	avrdude -c $(AVRDUDE_ISP) -P $(AVRDUDE_PORT) -p $(AVRDUDE_DEVICE) -D -F -B 0.25 -D -U flash:w:$(PROJECT).hex:i
	./term $(BAUD) $(PORT)
	#./miniterm $(BAUD) $(PORT)

flash-isp-release: isp install_optiboot
	avrdude -c $(AVRDUDE_ISP) -P $(AVRDUDE_PORT) -p $(AVRDUDE_DEVICE) -D -F -B $(AVRDUDE_SPEED) -U flash:w:release/build/$(PROJECT).hex:i
	./term $(BAUD) $(PORT)
	#./miniterm $(BAUD) $(PORT)

verify-isp:
	avrdude -c $(AVRDUDE_ISP) -P $(AVRDUDE_PORT) -p $(AVRDUDE_DEVICE) -F -B $(AVRDUDE_SPEED) -U flash:v:$(PROJECT).hex:i

verify-isp-fast: 
	avrdude -c $(AVRDUDE_ISP) -P $(AVRDUDE_PORT) -p $(AVRDUDE_DEVICE) -F -B 0.25 -U flash:v:$(PROJECT).hex:i

# =======================================
# OPTIBOOT flashing using built in boot loader - arduino protocol
#    We Always disable eraseing
# You MUST press RESET and issue these commands very quickly afterwards
#    Suggestion on your computer type in the make command with pressing enter - press reset and then enter quickly after
# 
flash: arduino all
	# ./reset $(BAUD) $(PORT)
	# avrdude -c arduino -P $(PORT) -p $(AVRDUDE_DEVICE) -D -F -B $(AVRDUDE_SPEED)  -U flash:w:$(PROJECT).hex:i
	# ./miniterm $(BAUD) $(PORT)
	uploader/flasher $(BAUD) $(PORT) $(PROJECT).hex
	./term $(BAUD) $(PORT)

flash-release:	arduino 
	# ./reset $(BAUD) $(PORT)
	# avrdude -c arduino -P $(PORT) -p $(AVRDUDE_DEVICE) -D -F -B $(AVRDUDE_SPEED) -U flash:w:release/build/$(PROJECT).hex:i
	# ./miniterm $(BAUD) $(PORT)
	uploader/flasher $(BAUD) $(PORT) release/build/$(PROJECT).hex
	./term $(BAUD) $(PORT)

verify: 
	./reset $(BAUD) $(PORT)
	avrdude -c arduino -P $(PORT) -p $(AVRDUDE_DEVICE) -D -F -B $(AVRDUDE_SPEED)  -U flash:v:$(PROJECT).hex:i

verify-release:	
	./reset $(BAUD) $(PORT)
	avrdude -c arduino -P $(PORT) -p $(AVRDUDE_DEVICE) -D -F -B $(AVRDUDE_SPEED) -U flash:v:release/build/$(PROJECT).hex:i

# =======================================
# ISP flashing - NO optiboot!
# We ALWAYS erase the CHIP before flashing
flash-isp-noboot: isp all 
	avrdude -c $(AVRDUDE_ISP) -P $(AVRDUDE_PORT) -p $(AVRDUDE_DEVICE) -F -B $(AVRDUDE_SPEED) $(fuses) -U flash:w:$(PROJECT).hex:i
	./term $(BAUD) $(PORT)
	#./miniterm $(BAUD) $(PORT)

flash-isp-noboot-fast: isp all 
	avrdude -c $(AVRDUDE_ISP) avrdude -P $(AVRDUDE_PORT) -p $(AVRDUDE_DEVICE) -F -B 0.25 -D $(fuses) -U flash:w:$(PROJECT).hex:i
	./term $(BAUD) $(PORT)
	#./miniterm $(BAUD) $(PORT)

flash-isp-noboot-release:  isp all
	avrdude -c $(AVRDUDE_ISP) -P $(AVRDUDE_PORT) -p $(AVRDUDE_DEVICE) -F -B $(AVRDUDE_SPEED) $(fuses) -U flash:w:release/build/$(PROJECT).hex:i
	./term $(BAUD) $(PORT)


# =======================================

# If makefile changes, maybe the list of sources has changed, so update doxygens list
.PHONY: doxyfile.inc
doxyfile.inc:
	echo "INPUT         =  $(INCDIRS)" > doxyfile.inc
	echo "FILE_PATTERNS =  *.h *.c *.md" >> doxyfile.inc

.PHONY: mkcfg
mkcfg:
	make -C sdcard/mkcfg

.PHONY: lif
lif:   
	make -C lif

install: $(PROGS) optiboot 
	make -C lif install
	make -C sdcard/mkcfg install
	make -C optiboot optiboot
	install -s hardware/baudrate /usr/local/bin

warn:	clean
	make 2>&1 | grep -i warn >warnings.txt
	@cat warnings.txt

.PHONY: doxy
doxy:	doxyfile.inc $(SRCS) 
	doxygen Doxyfile

ifeq ($(OUTPUT),ihex)
build: elf hex lst sym lss 
hex: $(PROJECT).hex
else
ifeq ($(OUTPUT),binary)
build: elf bin lst sym
bin: $(PROJECT).bin
else
ifeq ($(OUTPUT),both)
build: elf hex bin lst sym
hex: $(PROJECT).hex
bin: $(PROJECT).bin
else
$(error "Invalid format: $(OUTPUT)")
endif
endif
endif

elf: $(PROJECT).elf
lst: $(PROJECT).lst 
sym: $(PROJECT).sym
lss: $(PROJECT).lss


# Display compiler version information.
version :
	@if [ ! -f "update.last" ]; then touch "update.last"; fi
	@$(CC) --version
	echo COBJ: $(COBJ)

# Create final output file (.hex or .bin) from ELF output file.
%.hex: %.elf
	@echo
	echo Making HEX file
	$(OBJCOPY) -O ihex $(HEX_FLASH_FLAGS)  $< $@


%.eep: %.elf
	echo Making EPP file
	-$(OBJCOPY) -O ihex $(HEX_EEPROM_FLAGS) $< $@ || exit 0


%.lss: %.elf
	echo Making LSS file
	avr-objdump -h -S -t $< > $@


%.bin: %.elf
	@echo
	echo Making BIN file
	$(OBJCOPY) -O binary $< $@

# Create extended listing file from ELF output file.
%.lst: %.elf
	@echo
	echo Making LST File
	$(OBJDUMP) -h -S -C $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	echo Making SYM File
	$(NM) -n $< > $@

# Display size of file.
.PHONY:	size
size: 
	@echo
	-$(SIZE) -C --mcu=$(DEVICE) $(PROJECT).elf
	-$(SIZE) -x -A --mcu=${DEVICE} $(PROJECT).elf
	-$(SIZE) -x --common -C --mcu=${DEVICE} $(PROJECT).elf
	-avr-nm -n -S $(PROJECT).elf | grep "__eeprom"
	#-avr-nm -n -S $(PROJECT).elf | grep "__noinit"
	-avr-nm -n -S $(PROJECT).elf | grep "__bss"
	-avr-nm -n -S $(PROJECT).elf | grep "__data"
	-avr-nm -n -S $(PROJECT).elf | grep "__heap"
	-avr-nm -n -S $(PROJECT).elf | grep "__brkval"

# Link: create ELF output file from object files.
%.elf:  $(AOBJ) $(COBJ) $(LIBS)
	@echo
	echo Linking ELF File
	$(CC) $(CFLAGS) $(AOBJ) $(COBJ) $(LIBS) --output $@


# Compile: create object files from C source files. ARM or Thumb(-2)
$(COBJ) : %.o : %.c
	@echo
	@echo $< :
	$(CC) -c $(CFLAGS) $< -o $@
	# $(CC) -c $(CFLAGS) $< -o $@

# Assemble: create object files from assembler source files. ARM or Thumb(-2)
$(AOBJ) : %.o : %.S
	@echo
	@echo $< :
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


## Compile
# asm from (hand coded) asm
%.s: %.S
	@echo
	@echo $< :
	$(CC) -S $(ALL_ASFLAGS) $< -o $@


# object from asm
.S.o :
	$(CC) $(ALL_ASFLAGS) -c $< -o $@



# Target: clean project.
clean:
	@echo
	rm -f -r $(COBJ) $(PROJECT)\.* dep/* | exit 0
	make -C lif clean
	make -C printf clean
	make -C sdcard/mkcfg clean
	make -C optiboot clean
	rm -f hardware/baudrate
	

# Include the dependency files.
#-include $(shell mkdir $(COBJDIR) 2>/dev/null) $(wildcard $(OBJDIR)/*.d)
## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

.PHONY: all clean distclean doxy


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

### Source files and search directory

FATFS_TESTS=1

FATFS= \
fatfs/mmc_hal.c \
fatfs/disk.c \
fatfs/posix.c \
fatfs/mmc.c \
fatfs/ff.c \
fatfs/option/syscall.c \
fatfs/option/unicode.c 
ifeq ($(FATFS_TESTS),1)
	FATFS += fatfs/fatfs_tests.c 
	FATFS += fatfs/fatfs_utils.c 
endif


GPIB   = \
gpib/gpib_hal.c \
gpib/gpib.c \
gpib/gpib_task.c \
gpib/gpib_tests.c \
gpib/ss80.c \
gpib/amigo.c \
gpib/printer.c

CSRC    = \
hardware/ram.c \
hardware/delay.c \
hardware/rs232.c \
hardware/spi.c \
hardware/rtc.c \
hardware/TWI_AVR8.c \
lib/timer_hal.c \
lib/timer.c \
lib/time.c \
lib/util.c \
main.c \
 $(FATFS) \
 $(GPIB)

CC = avr-gcc

ASRC    = \
 # fatfs/xitoa.o
VPATH   =

### Target device
DEVICE  = atmega1284p

### Optimization level (0, 1, 2, 3, 4 or s)
OPTIMIZE = s
# OPTIMIZE = s

### C Standard level (c89, gnu89, c99 or gnu99)
CSTD = gnu99

### Include dirs, library dirs and definitions
LIBS    =
LIBDIRS =
#INCDIRS =/share/embedded/GPIB/mike/mine
INCDIRS =.

#DEFS    = F_CPU=20000000 SDEBUG=9 SOFTWARE_PP=1 SPOLL=1 HP9134L=1
DEFS    = F_CPU=20000000 SDEBUG=10 SPOLL=1 HP9134L=1 $(DEVICE) \
	AMIGO AMIGO_HACK
ifeq ($(FATFS_TESTS),1)
	DEFS += FATFS_TESTS
endif
#DEFS    = F_CPU=20000000 SDEBUG=10 SPOLL=1 HP9134L=1 $(DEVICE) FATFS_TESTS
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


## Options common to compile, link and assembly rules
# COMMON = -mmcu=$(DEVICE) -Wl,--section-start,.data=0x801100,--defsym=__heap_start=0x803000,--defsym=__heap_end=0x807fff
# COMMON = -Wl,--section-start,.data=0x801100,--defsym=__heap_end=0x807fff
COMMON = -Wl,-u,vfprintf -lprintf_flt -lm

# Flags for C files
CFLAGS = $(COMMON)
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

# atmega1284p and atmega644 can use same fuses
fuses=-U lfuse:w:0xd6:m -U hfuse:w:0x99:m -U efuse:w:0xff:m

SRCS = $(CSRC)
SRCDIRS= . fatfs fatfs/option gpib hardware lib 



flash:  all
#
	# Program with avrdude using atmelice_isp
	# avrdude -P usb -p m1284p -c atmelice_isp -F -B0.25 $(fuses) -U flash:w:$(PROJECT).hex
	#
	# Program with avrdude using avrispmkii 
	# avrdude -P usb -p m1284p -c avrispmkII -F -B 2 $(fuses) -U flash:w:$(PROJECT).hex"
	#
	# Program with avrdude using dragon_isp
	# avrdude -P usb -p m1284p -c dragon_isp -F -B 1 $(fuses) -U flash:w:$(PROJECT).hex
	#
	# ===================================================
	# atmelice_isp
	# avrdude -c list 2>&1 | grep -ie atmelice
	#  atmelice         = Atmel-ICE (ARM/AVR) in JTAG mode
	#  atmelice_dw      = Atmel-ICE (ARM/AVR) in debugWIRE mode
	#  atmelice_isp     = Atmel-ICE (ARM/AVR) in ISP mode
	#  atmelice_pdi     = Atmel-ICE (ARM/AVR) in PDI mode
#ICE
	avrdude -P usb -p m1284p -c atmelice_isp -F -B0.25 $(fuses) -U flash:w:$(PROJECT).hex
# MKII
	# ===================================================
	

# Default target.
all: doxy version $(LIBS) build size $(PROGS)

# If makefile changes, maybe the list of sources has changed, so update doxygens list
doxyfile.inc:	
	echo "INPUT         =  $(SRCDIRS)" > doxyfile.inc
	echo "FILE_PATTERNS =  *.h *.c *.md" >> doxyfile.inc

doxy:	doxyfile.inc $(SRCS) 
	export PYTHONPATH="$PYTHONPATH:/share/embedded/testgen-0.11/extras"
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
size: 
	@echo
	$(SIZE) -C --mcu=$(DEVICE) $(PROJECT).elf
	$(SIZE) -x -A --mcu=${DEVICE} $(PROJECT).elf
	$(SIZE) -x --common -C --mcu=${DEVICE} $(PROJECT).elf
	-avr-nm -n -S $(PROJECT).elf | grep "__eeprom"
	-avr-nm -n -S $(PROJECT).elf | grep "__noinit"
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
	rm -f -r $(COBJ) $(PROGS) $(PROJECT)\.* dep/* | exit 0
	

# Include the dependency files.
#-include $(shell mkdir $(COBJDIR) 2>/dev/null) $(wildcard $(OBJDIR)/*.d)
## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

.PHONY: all clean distclean doxy

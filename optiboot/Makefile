# Makefile to program boot loader and flash chip
#
# These defaults are only assigned if they were not defined when the Makefile was called
PORT           ?= /dev/ttyUSB0
DEVICE         ?= atmega1284p
AVRDUDE_DEVICE ?= m1284p
AVRDUDE_ISP    ?= atmelice_isp
ISP_PORT       ?= usb
ISP_SPEED      ?= 0.25
LED            ?= C6

OPTIBOOT_HEX   ?= optiboot_$(DEVICE).hex

# FUSES for hp85disk
fuses_optiboot   ?= -U lfuse:w:0xd6:m -U hfuse:w:0xda:m -U efuse:w:0xff:m 

# Enable boot protect
fuses_lock       ?= -U lock:w:0x3d:m


optiboot: 
	make -f Makefile.optiboot optiboot atmega1284p AVR_FREQ=20000000L BAUD_RATE=115200 LED=$(LED) LED_START_FLASHES=2 UART=0 BIGBOOT=1

install_optiboot: optiboot boot_install
	@echo OPTIBOOT has been installed

# Erase the chip including lock bits
erase_chip:
	@echo ERASE CHIP
	avrdude -P $(ISP_PORT) -p $(AVRDUDE_DEVICE) -c $(AVRDUDE_ISP) -F -B $(ISP_SPEED) -e

# Fuse programming
# -u disables safe mode to permit updating fuses_optiboot
fuse_write:
	@echo FUSE WRITE
	avrdude -P $(ISP_PORT) -p $(AVRDUDE_DEVICE) -c $(AVRDUDE_ISP) -u -F -B $(ISP_SPEED) $(fuses_optiboot) 

# Install optiboot code 
# Flash boot loader code using isp
boot_write: 
	@echo BOOT WRITE
	avrdude -P $(ISP_PORT) -p $(AVRDUDE_DEVICE) -c $(AVRDUDE_ISP) -F -B $(ISP_SPEED) -U flash:w:$(OPTIBOOT_HEX):i  

# Install optiboot code 
# Flash boot loader code using isp
boot_lock: 
	@echo BOOT LOCK
	avrdude -P $(ISP_PORT) -p $(AVRDUDE_DEVICE) -c $(AVRDUDE_ISP) -F -B $(ISP_SPEED) $(fuses_lock)

# Install optiboot code with fuses_optiboot after erasing chip
boot_install:	erase_chip boot_write fuse_write boot_lock
	@echo BOOT INSTALLED

# Read chip status
info:
	avrdude -P $(ISP_PORT) -p $(AVRDUDE_DEVICE) -c $(AVRDUDE_ISP) -v

clean:	
	rm -f optiboot optiboot_$(DEVICE)\.*

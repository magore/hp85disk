#!/bin/bash
#
for i in $*
do
	if [ ! -f "$i" ]
	then
		continue
	fi
	sed -i -e "s/AVR_DIR_IN/GPIO_PIN_DIR_IN/" "$i"
	sed -i -e "s/AVR_DIR_OUT/GPIO_PIN_DIR_OUT/" "$i"
	sed -i -e "s/AVR_LATCH_LOW/GPIO_PIN_LATCH_LOW/" "$i"
	sed -i -e "s/AVR_LATCH_HI/GPIO_PIN_LATCH_HI/" "$i"
	sed -i -e "s/AVR_LATCH_RD/GPIO_PIN_LATCH_RD/" "$i"
	sed -i -e "s/AVR_PIN_RD/GPIO_PIN_RD/" "$i"
	sed -i -e "s/AVR_PULLUP/GPIO_PIN_LATCH_HI/" "$i"
	sed -i -e "s/AVR_NO_PULLUP/GPIO_PIN_LATCH_LOW/" "$i"
	sed -i -e "s/GPIO_MODE/GPIO_PIN_MODE/" "$i"
	sed -i -e "s/GPIO_RD/GPIO_PIN_RD/" "$i"
	sed -i -e "s/GPIO_WR/GPIO_PIN_WR/" "$i"
	sed -i -e "s/GPIO_LATCH_HI/GPIO_PIN_LATCH_HI/" "$i"
	sed -i -e "s/GPIO_LATCH_LOW/GPIO_PIN_LATCH_LOW/" "$i"
	sed -i -e "s/GPIO_LATCH_RD/GPIO_PIN_LATCH_RD/" "$i"
	sed -i -e "s/GPIO_DIR_OUT/GPIO_PIN_DIR_OUT/" "$i"
	sed -i -e "s/GPIO_DIR_IN/GPIO_PIN_DIR_IN/" "$i"
	sed -i -e "s/GPIO_HI/GPIO_PIN_HI/" "$i"
	sed -i -e "s/GPIO_LOW/GPIO_PIN_LOW/" "$i"
	sed -i -e "s/GPIO_OUT/GPIO_PIN_OUT/" "$i"
	sed -i -e "s/GPIO_MODE/GPIO_PIN_MODE/" "$i"

	# Ooops
	sed -i -e "s/GPIO_PIN_LATCH_READ/GPIO_PIN_LATCH_RD/" "$i"

#	sed -i -e "s/IO_HI/GPIO_PIN_HI/" "$i"
#	sed -i -e "s/IO_LOW/GPIO_PIN_LOW/" "$i"
#	sed -i -e "s/IO_RD/GPIO_PIN_RD/" "$i"
	sed -i -e "s/IO_FLOAT/GPIO_PIN_FLOAT/" "$i"
	sed -i -e "s/IO_LATCH_RD/GPIO_PIN_LATCH_RD/" "$i"
	sed -i -e "s/IO_JUST_RD/GPIO_PIN_TST/" "$i"
	sed -i -e "s/IO_JUST_LOW/GPIO_PIN_LATCH_LOW/" "$i"
	sed -i -e "s/IO_JUST_HI/GPIO_PIN_LATCH_HI/" "$i"

	sed -i -e "s/gpio_sfr_mode/gpio_pin_sfr_mode/" "$i"
	sed -i -e "s/gpio_rd/gpio_pin_rd/" "$i"
	sed -i -e "s/gpio_wr/gpio_pin_wr/" "$i"
	sed -i -e "s/gpio_out/gpio_pin_out/" "$i"
	sed -i -e "s/gpio16_dir/gpio16_pin_dir/" "$i"

	sed -i -e "s/GPIO16_MODE/GPIO16_PIN_MODE/" "$i"
	sed -i -e "s/GPIO16_DIR_IN/GPIO16_PIN_DIR_IN/" "$i"
	sed -i -e "s/GPIO16_LATCH_HI/GPIO16_PIN_LATCH_HI/" "$i"
	sed -i -e "s/GPIO16_LATCH_LOW/GPIO16_PIN_LATCH_LOW/" "$i"
	sed -i -e "s/GPIO16_DIR_OUT/GPIO16_PIN_DIR_OUT/" "$i"
	sed -i -e "s/GPIO16_HI/GPIO16_PIN_HI/" "$i"
	sed -i -e "s/GPIO16_LOW/GPIO16_PIN_LOW/" "$i"
	sed -i -e "s/GPIO16_OUT/GPIO16_PIN_OUT/" "$i"
	sed -i -e "s/GPIO16_FLOAT/GPIO16_PIN_FLOAT/" "$i"

done

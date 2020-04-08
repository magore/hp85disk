#!/bin/bash
#
# Mike Gore 2020
#   https://github.com/magore/hp85disk
#
# Works Under Ubuntu 18.04 or later and Ubuntu App under Windows 10
#
# Download github github project
# Install all required software for compiling and flashing firmware

# Become root
echo You need to become root to install the software
echo sudo will prompt you for your password
sudo bash

echo
echo installing software
apt-get update
apt-get install git aptitude make build-essential binutils gcc
aptitude --with-recommends install python3 python3-pip python-serial \
	minicom \
	avr-libc avra avrdude avrdude-doc avrp binutils-avr gcc-avr gdb-avr

pip3 install pySerial

echo
echo Downloading https://github.com/magore/hp85disk
if [ ! -d hp85disk ]
then
	git clone --branch V2 https://github.com/magore/hp85disk
else
	pushd hp85disk
		git pull
	popd
fi

pushd hp85disk
	git checkout V2
popd

echo
echo Done

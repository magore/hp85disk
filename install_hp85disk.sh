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

echo installing support software for building the hp85disk project
echo Note: sudo will prompt you for your password for software installation
sudo apt-get update
sudo apt-get install -y git aptitude make build-essential binutils gcc
sudo aptitude --with-recommends install -y python3 python3-pip python-serial minicom avr-libc avra avrdude avrdude-doc avrp binutils-avr gcc-avr gdb-avr


echo
echo installing pySerial
pip3 install pySerial

echo
echo Downloading hp85disk github master branch code
echo Downloading https://github.com/magore/hp85disk
if [ ! -d hp85disk ]
then
	git clone --branch master https://github.com/magore/hp85disk
else
	pushd hp85disk
		git pull
	popd
fi


pushd hp85disk
	git checkout master
popd

echo
echo
echo =============================================================================
echo The directory hp85disk contains the hp85disk software and installation scripts
echo Done

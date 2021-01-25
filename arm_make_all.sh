#!/bin/bash
echo "Starting..."

#PATH="${HOME}/opt/gcc-arm-none-eabi-6-2017-q2-update/bin/:$PATH"
PATH="${HOME}/opt/gnu-mcu-eclipse/arm-none-eabi-gcc/8.2.1-1.1-20190102-1122/bin/:$PATH"

#remember to install the libs: sudo apt-get install lib32z1 lib32ncurses5

# make all -j4
make all

#!/bin/bash

# MAIN SCRIPT TO BOOT OPERATING SYSTEM, HAS VERY POWERFUL SETTING'S WITH QEMU, PLEASE EDIT THEM TO YOUR DESIRE.

#[EXAMPLE]: -m 10G (10 Gigbytes of RAM)

# Exit on error.
set -e

echo "[INFO]: CLEANING PREVIOUS BUILD"
make clean

echo "[INFO]: BUILDING"
make all

echo "[INFO]: LAUNCHING QEMU"
qemu-system-x86_64 \
    -cdrom bin/caneos.iso \
    -machine q35 \
    -cpu max \
    -smp 16,cores=16,threads=1 \
    -m 15G \
    -vga std \
    -serial stdio \
    -rtc base=localtime \
    -d guest_errors \
    -audiodev sdl,id=audio0 \
    -machine pcspk-audiodev=audio0

# Clean build artifacts and clear the termianl upon exit.
make clean
clear
clear
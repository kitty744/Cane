#!/bin/bash

# QEMU Configuration - Accept command line arguments for flexible settings

# Default QEMU settings
DEFAULT_QEMU_SETTINGS="-machine q35 -cpu max -smp 16,cores=16,threads=1 -m 15G -vga std -serial stdio -rtc base=localtime -d guest_errors -audiodev sdl,id=audio0 -machine pcspk-audiodev=audio0"

# Use provided arguments, or fall back to defaults
if [ $# -eq 0 ]; then
    QEMU_ARGS="$DEFAULT_QEMU_SETTINGS"
else
    QEMU_ARGS="$*"
fi

echo "[INFO]: CLEANING PREVIOUS BUILD"
make clean

echo "[INFO]: BUILDING"
make all

echo "[INFO]: LAUNCHING QEMU"
echo "[INFO]: Using QEMU settings: $QEMU_ARGS"

qemu-system-x86_64 \
    -cdrom bin/caneos.iso \
    $QEMU_ARGS

# Clean build artifacts and clear the terminal upon exit.
make clean
clear

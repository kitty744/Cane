#!/bin/bash

# MEANT FOR TESTING THE OPERATING SYSTEMS COMPATABILITY ON LOWER-END MACHINES.

# Exit on error's.
set -e

echo "[INFO]: CLEANING PREVIOUS BUILD"
make clean

echo "[INFO]: BUILDING"
make all

echo "[INFO]: LAUNCHING QEMU"
qemu-system-x86_64 -cdrom bin/caneos.iso -m 512M -serial stdio -boot d

make clean
clear
clear
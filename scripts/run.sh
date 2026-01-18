#!/bin/bash

set -e

echo "Cleaning previous build..."
make clean

echo "Building CaneOS..."
make all

echo "Launching QEMU..."
qemu-system-x86_64 -cdrom bin/caneos.iso -m 512M -serial stdio -boot d

echo "Cleaning up..."
make clean

echo "Done!"

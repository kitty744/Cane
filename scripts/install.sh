#!/bin/bash

# Script to intstall missing dependencies.

# --- 1. DEPENDENCY CHECK & AUTO-INSTALLER ---
check_and_install() {
    # Core binaries needed for ValenOS
    DEPS=("qemu-system-x86_64" "nasm" "x86_64-elf-gcc" "grub-mkrescue" "xorriso" "kconfig-mconf" "coccinelle")
    MISSING=()

    for tool in "${DEPS[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            MISSING+=("$tool")
        fi
    done

    if [ ${#MISSING[@]} -ne 0 ]; then
        echo "[!] Missing dependencies: ${MISSING[*]}"
        echo "[?] Attempting to identify OS and install..."

        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            if command -v apt &> /dev/null; then
                echo "Detected Ubuntu/Debian. Installing..."
                sudo apt update && sudo apt install -y gcc-x86-64-elf-binutils nasm grub-common grub-pc-bin xorriso mtools qemu-system-x86 kconfig-frontends coccinelle
            elif command -v pacman &> /dev/null; then
                echo "Detected Arch Linux. Installing..."
                sudo pacman -S --needed gcc-x86_64-elf nasm grub libisoburn mtools qemu-full coccinelle
                yay -S kconfig-frontends
            fi
        elif [[ "$OSTYPE" == "darwin"* ]]; then
            echo "Detected macOS. Installing via Homebrew..."
            brew install x86_64-elf-gcc nasm qemu xorriso mtools coccinelle
            brew tap osx-cross/arm && brew install kconfig-frontends
        else
            echo "[ERROR] Unknown OS. Please install dependencies manually: ${MISSING[*]}"
            exit 1
        fi
    fi
}

# Run the check
check_and_install
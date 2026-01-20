# CaneOS

A modern x86_64 operating system built from scratch with comprehensive hardware compatibility and a retro GUI aesthetic.

## Overview

CaneOS is a complete operating system designed to run on both old and new hardware, featuring:

- **x86_64 Architecture** with proper protection rings
- **Multiboot2 Bootloader** compatibility
- **Comprehensive Memory Management** (PMM, VMM, Heap)
- **Enhanced I/O and Driver Support**
- **Retro GUI** inspired by classic Windows versions
- **Hardware Auto-Detection** for optimal performance

## Features

### Core Systems

- ✅ **Multiboot2 Bootloader** with GRUB integration
- ✅ **64-bit Higher Half Kernel** with proper virtual memory
- ✅ **Physical Memory Manager (PMM)** with bitmap allocation
- ✅ **Virtual Memory Manager (VMM)** with page mapping
- ✅ **Kernel Heap** with malloc/free support
- ✅ **Enhanced Printf** with extensive formatting options

## Architecture

### Protection Rings

- **Ring 0**: Kernel core, memory management, interrupt handling
- **Ring 1**: Direct hardware access layer
- **Ring 2**: Hardware abstraction and system APIs
- **Ring 3**: User applications and GUI

### Memory Layout

- **Physical Memory**: 1MB kernel load address
- **Virtual Memory**: Higher half at 0xFFFFFFFF80000000
- **Page Size**: 4KB with 2MB huge pages support
- **Heap Management**: Static buffer with future dynamic expansion

## Build Requirements

### Toolchain

- **x86_64-elf-gcc**: Cross-compiler for x86_64
- **nasm**: Assembly compiler
- **x86_64-elf-ld**: Linker
- **grub-mkrescue**: ISO creation
- **qemu-system-x86_64**: Emulation/testing

### Dependencies

```bash
# Ubuntu/Debian
sudo apt install gcc-x86-64-elf-binutils nasm grub-common qemu-system-x86

# Arch Linux
sudo pacman -S gcc-x86_64-elf nasm grub qemu

# macOS (with Homebrew)
brew install x86_64-elf-gcc nasm grub qemu
```

## Building and Running

### Quick Start

```bash
# Build and run with supercharged setting's.
make run

# Build and run with low setting's to test compatability.
make compat

# Clean build artifacts
make clean

# Build only (no QEMU)
make all
```

### Build Process

1. **Assembly**: NASM compiles boot.s with elf32 format
2. **Compilation**: GCC compiles all C files with kernel flags
3. **Linking**: LD creates the ELF kernel with proper memory layout
4. **ISO Creation**: GRUB creates bootable ISO with multiboot2 support
5. **Testing**: QEMU launches the OS with CD-ROM boot

## Project Structure

```
cane/
├── arch/x86_64/         # Architecture-specific code
├── kernel/              # Core kernel implementation
├── mm/                  # Memory management
├── lib/                 # Standard library
├── include/cane/        # Public headers
├── scripts/             # Build and utility scripts
├── Makefile             # Wildcard-based build system
├── linker.ld            # Kernel memory layout
├── .gitignore           # Version control exclusions
└── .editorconfig        # Editor configuration
```

## Development Status

### ✅ Completed

- Multiboot2 bootloader with GRUB integration
- 64-bit higher half kernel with proper paging
- Complete memory management system (PMM, VMM, Heap)
- Enhanced printf with extensive formatting options
- Automated memory management initialization
- Multiboot2 memory parsing
- Interrupt handling system
- **Interactive Command-line Shell** with full text editing capabilities
- PS/2 Keyboard driver with arrow key support
- VGA text mode display with cursor management

### 🔄 In Progress

- Implementing process management and multitasking
- Developing a comprehensive driver framework
- Enhancing the kernel's hardware abstraction layer

### 📋 Planned

- Comprehensive hardware drivers
- Retro GUI system
- User applications
- Advanced networking stack

## Printf Formatting Options

CaneOS includes an enhanced printf with extensive formatting:

```c
// Numbers
printf("%d", -42);        // Signed integer
printf("%u", 42);         // Unsigned integer
printf("%ld", 12345678);  // Long integer
printf("%llu", 1234567890); // Long long unsigned

// Bases
printf("%x", 255);        // Hexadecimal (lowercase)
printf("%X", 255);        // Hexadecimal (uppercase)
printf("%o", 8);          // Octal
printf("%b", 5);          // Binary

// Other
printf("%p", ptr);        // Pointer (0x format)
printf("%s", "hello");    // String
printf("%c", 'A');        // Character
printf("%%");             // Literal percent
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Test thoroughly with `./scripts/run.sh`
5. Submit a pull request

## License

This project is open source. See LICENSE file for details.

## Acknowledgments

- Inspired by Linux kernel architecture and design patterns
- Built with modern OS development best practices
- Compatible with existing hardware and software standards

---

**CaneOS** - Building the future of retro computing with modern technology.

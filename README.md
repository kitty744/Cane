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
- **kconfig-frontends**: Kernel configuration frontend

### Installing

```bash
# Install's all required dependencies.
make install
```

## Configuration

CaneOS uses Kconfig for kernel configuration management, similar to the Linux kernel.

### `make menuconfig`

Launch the interactive configuration menu to customize kernel settings:

```bash
make menuconfig
```

**Configuration File:**

- Settings are saved to `.config` in the project root
- The configuration is automatically used when building

**Usage Workflow:**

1. Run `make menuconfig` to configure your kernel
2. Save the configuration
3. Run `make run` to build and test with your settings

## Building and Running

### Make Targets

### `make install`

Install all dependencies required.

```bash
make install
```

#### `make run`

Build and run CaneOS with default QEMU settings:

- **Memory**: 2G RAM
- **CPU**: 5 cores
- **Graphics**: VGA standard mode
- **Audio**: SDL audio output
- **Machine**: Q35 chipset

```bash
make run
```

#### `make all`

Build the kernel and ISO only, without launching QEMU:

```bash
make all
```

#### `make clean`

Remove all build artifacts:

```bash
make clean
```

### Quick Start

```bash
# 1. Install dependencies
make install

# 2. Configure kernel settings (optional)
make menuconfig

# 3. Build and run with configured settings
make run

# 4. Build only (no QEMU)
make all

# 5. Clean build artifacts
make clean
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

## License

This project is open source. See LICENSE file for details.

## Acknowledgments

- Inspired by Linux kernel architecture and design patterns
- Built with modern OS development best practices
- Compatible with existing hardware and software standards

---

**CaneOS** - Building the future of retro computing with modern technology.

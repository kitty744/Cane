# Valen

![Valen](assets/logo/valen.png)

A modern x86_64 operating system built from scratch with comprehensive hardware compatibility and a retro GUI aesthetic.

## Overview

Valen is an operating system designed to run on both old and new hardware.

## Build Requirements

### Toolchain

- **x86_64-elf-gcc**: Cross-compiler for x86_64
- **nasm**: Assembly compiler
- **x86_64-elf-ld**: Linker
- **grub-mkrescue**: ISO creation
- **qemu-system-x86_64**: Emulation/testing
- **kconfig-frontends**: Kernel configuration frontend
- **coccinelle**: Debugging/Safety.

## Building and Running

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

## Documentation

Comprehensive documentation is available in the `docs/` directory to help contributors understand the codebase:

### Library Documentation

- **[STDIO Library](docs/code/lib/STDIO.md)** - VGA text mode output, serial communication, and formatted printing
- **[String Library](docs/code/lib/STRING.md)** - String manipulation and utility functions
- **[I/O Operations](docs/code/lib/IO.md)** - Hardware I/O port operations

### Development Documentation

- **[Project Structure](docs/code/structure/STRUCTURE.md)** - Overview of the codebase organization and architecture

### Memory Documentation

- **[Memory Management](docs/code/mm/MEM.md)** - Memory allocation and management functions

### Driver's Documentation

- **[Device Drivers](docs/code/drivers/DRIVERS.md)** - Hardware driver implementation details

### Kernel Documentation

- **[Boot Process](docs/code/kernel/BOOT.md)** - System startup and initialization sequence

## License

This project is open source. See LICENSE file for details.

## Acknowledgments

- Inspired by Linux kernel architecture and design patterns
- Built with modern OS development best practices
- Compatible with existing hardware and software standards

---

**Valen** - Building the future of retro computing with modern technology.

CC = x86_64-elf-gcc
AS = nasm
LD = x86_64-elf-ld
CFLAGS = -m64 -nostdlib -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone -mcmodel=kernel -Iinclude
ASFLAGS = -f elf64
LDFLAGS = -n -T linker.ld -z max-page-size=0x1000

SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Find all source files automatically
KERNEL_SRCS = $(shell find . -name '*.c' -o -name '*.s' | grep -v 'iso\|bin\|obj')
KERNEL_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(patsubst %.c,$(OBJDIR)/%.o,$(KERNEL_SRCS)))

KERNEL_BIN = $(BINDIR)/valen.bin
KERNEL_ISO = $(BINDIR)/valen.iso

-include .config

all: $(KERNEL_ISO)

$(KERNEL_ISO): $(KERNEL_BIN)
	mkdir -p isofiles/boot/grub
	cp $(KERNEL_BIN) isofiles/boot/valen.bin
	echo 'set timeout=0' > isofiles/boot/grub/grub.cfg
	echo 'set default=0' >> isofiles/boot/grub/grub.cfg
	echo 'menuentry "valen" {' >> isofiles/boot/grub/grub.cfg
	echo '    multiboot2 /boot/valen.bin' >> isofiles/boot/grub/grub.cfg
	echo '    boot' >> isofiles/boot/grub/grub.cfg
	echo '}' >> isofiles/boot/grub/grub.cfg
	grub-mkrescue -o $(KERNEL_ISO) isofiles

$(KERNEL_BIN): $(KERNEL_OBJS)
	mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) -f elf32 $(ASFLAGS) -o $@ $<

export $(shell [ -f .config ] && sed 's/=.*//' .config)
KCONFIG_MCONF := $(shell which kconfig-mconf || which mconf || echo kconfig-mconf)

install:
	@chmod +x ./scripts/install.sh
	./scripts/install.sh
run: all
	@chmod +x ./scripts/run.sh
	./scripts/run.sh
menuconfig:
	$(KCONFIG_MCONF) Kconfig
clean:
	rm -rf $(OBJDIR) $(BINDIR) isofiles

COCCI_DIR     := scripts/cocci
COCCI_SCRIPTS := $(wildcard $(COCCI_DIR)/*.cocci)
COCCI_TARGETS := $(filter %.c, $(KERNEL_SRCS))

coccinelle:
	@for script in $(COCCI_SCRIPTS); do \
		echo ">>> Applying: $$script"; \
		spatch --sp-file $$script $(COCCI_TARGETS) -I include/ --macro-file scripts/cocci/cocci_macros.h || true; \
	done

.PHONY: all clean coccinelle
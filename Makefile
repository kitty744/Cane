CC = x86_64-elf-gcc
AS = nasm
LD = x86_64-elf-ld
CFLAGS = -m64 -nostdlib -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone -mcmodel=kernel -Iinclude
ASFLAGS = -f elf64
LDFLAGS = -n -T linker.ld -z max-page-size=0x1000

SRCDIR = src
OBJDIR = obj
BINDIR = bin

KERNEL_SRCS = arch/x86_64/boot.s kernel/kernel.c lib/stdio.c lib/string.c
KERNEL_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(patsubst %.c,$(OBJDIR)/%.o,$(KERNEL_SRCS)))

KERNEL_BIN = $(BINDIR)/caneos.bin
KERNEL_ISO = $(BINDIR)/caneos.iso

all: $(KERNEL_ISO)

$(KERNEL_ISO): $(KERNEL_BIN)
	mkdir -p isofiles/boot/grub
	cp $(KERNEL_BIN) isofiles/boot/caneos.bin
	echo 'set timeout=0' > isofiles/boot/grub/grub.cfg
	echo 'set default=0' >> isofiles/boot/grub/grub.cfg
	echo 'menuentry "CaneOS" {' >> isofiles/boot/grub/grub.cfg
	echo '    multiboot2 /boot/caneos.bin' >> isofiles/boot/grub/grub.cfg
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

clean:
	rm -rf $(OBJDIR) $(BINDIR) isofiles

.PHONY: all clean

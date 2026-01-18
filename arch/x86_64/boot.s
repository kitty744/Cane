[BITS 32]
section .boot

; Define offset used to convert Virtual Addresses to Physical
KERNEL_VIRT_OFFSET equ 0xFFFFFFFF80000000

; Multiboot2 Header (must be 8-byte aligned and within first 32KB)
align 8
multiboot_header_start:
    dd 0xe85250d6                ; magic
    dd 0                         ; architecture 0 (protected mode i386)
    dd multiboot_header_end - multiboot_header_start
    dd 0x100000000 - (0xe85250d6 + 0 + (multiboot_header_end - multiboot_header_start))
    
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
multiboot_header_end:

global _start
global p4_table
global p3_table
global p2_table
extern kmain

_start:
    ; Setup Stack (Physical Address)
    mov esp, stack_top - KERNEL_VIRT_OFFSET

    ; Clear Page Tables
    mov edi, p4_table - KERNEL_VIRT_OFFSET
    mov ecx, 3072 ; (4096 bytes * 3 tables) / 4 bytes per stosd
    xor eax, eax
    cld
    rep stosd

    ; Setup P4 (PML4)
    mov eax, p3_table - KERNEL_VIRT_OFFSET
    or eax, 0b11 ; Present + Writable
    
    mov edi, p4_table - KERNEL_VIRT_OFFSET
    mov [edi], eax
    
    mov [edi + 511 * 8], eax

    ; Setup P3 (PDPT)
    mov eax, p2_table - KERNEL_VIRT_OFFSET
    or eax, 0b11 ; Present + Writable
    mov edi, p3_table - KERNEL_VIRT_OFFSET

    mov [edi], eax
    mov [edi + 510 * 8], eax

    ; Setup P2 (Page Directory) - Map first 1GB with 2MB pages
    mov edi, p2_table - KERNEL_VIRT_OFFSET
    xor ecx, ecx

.loop_p2:
    mov eax, 0x200000 ; 2MB
    mul ecx           
    or eax, 0b10000011 ; Present + Writable + Huge Page
    mov [edi + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne .loop_p2

    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Set Long Mode MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable Paging
    mov eax, p4_table - KERNEL_VIRT_OFFSET
    mov cr3, eax

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [gdt64.pointer_phys - KERNEL_VIRT_OFFSET]

    ; Far Jump to Long Mode
    jmp 0x08:(long_mode_start - KERNEL_VIRT_OFFSET)

[BITS 64]
section .text

long_mode_start:
    ; Jump to Virtual Address
    mov rax, .higher_half
    jmp rax

.higher_half:
    ; Reload GDT with Virtual Address
    mov rax, gdt64.pointer_virt
    lgdt [rax]

    ; Reset Data Segments
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Setup Virtual Stack
    mov rsp, stack_top
    
    ; Enter C Kernel
    call kmain
    
    cli
.hang: 
    hlt
    jmp .hang

section .rodata
align 8
gdt64:
    dq 0 ; Zero Entry
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)
.pointer_phys:
    dw $ - gdt64 - 1
    dq gdt64 - KERNEL_VIRT_OFFSET
.pointer_virt:
    dw $ - gdt64 - 1
    dq gdt64

section .bss
align 4096
p4_table: resb 4096
p3_table: resb 4096
p2_table: resb 4096
stack_bottom:
    resb 4096 * 16
stack_top:

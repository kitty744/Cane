[bits 64]
global gdt_flush

gdt_flush:
    lgdt [rdi]

    ; Set data segments to 0x10 (GDT entry 2)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Perform a far return to set CS to 0x08 (GDT entry 1)
    ; We push the Code Segment selector, then the return address.
    lea rax, [rel .reload_cs] 
    push 0x08                 ; New CS
    push rax                  ; New RIP
    retfq                     ; Far Return (pops RIP then CS)

.reload_cs:
    ret
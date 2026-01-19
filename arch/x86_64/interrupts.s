[bits 64]

extern page_fault_handler
extern keyboard_handler
extern generic_handler

global load_idt
global page_fault_isr
global keyboard_isr
global generic_isr

page_fault_isr:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; x86_64 pushes an error code onto the stack for Page Faults.
    ; It is located at [rsp + 120] after our pushes.
    mov rdi, [rsp + 120]
    call page_fault_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 8 ; Clean up the error code pushed by the CPU before returning
    iretq

;-----------------------------------------------------------------------------
; @brief Keyboard Interrupt Service Routine.
; Routes IRQ 1 (mapped to Vector 33 via I/O APIC).
;-----------------------------------------------------------------------------
keyboard_isr:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    call keyboard_handler

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    iretq

generic_isr:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    ; Call generic handler
    call generic_handler

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    iretq

load_idt:
    lidt [rdi]
    ret
[bits 64]
global switch_to

; void switch_to(task_context *prev, task_context *next)
switch_to:
    ; Save callee-saved registers from previous task
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15
    
    ; Save stack pointer if prev is not NULL
    test rdi, rdi
    jz .skip_save
    
    mov [rdi + 0], r15   ; Save R15 (offset 0)
    mov [rdi + 8], r14   ; Save R14 (offset 8)
    mov [rdi + 16], r13  ; Save R13 (offset 16)
    mov [rdi + 24], r12  ; Save R12 (offset 24)
    mov [rdi + 32], rbx  ; Save RBX (offset 32)
    mov [rdi + 40], rbp  ; Save RBP (offset 40)
    mov [rdi + 88], rsp  ; Save RSP (offset 88)
    
.skip_save:
    ; Load new context
    mov r15, [rsi + 0]   ; Load R15
    mov r14, [rsi + 8]   ; Load R14
    mov r13, [rsi + 16]  ; Load R13
    mov r12, [rsi + 24]  ; Load R12
    mov rbx, [rsi + 32]  ; Load RBX
    mov rbp, [rsi + 40]  ; Load RBP
    mov rsp, [rsi + 88]  ; Load RSP
    
    ; Restore callee-saved registers for new task
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    
    ; Jump to new task's RIP
    mov rax, [rsi + 136] ; Load RIP
    jmp rax

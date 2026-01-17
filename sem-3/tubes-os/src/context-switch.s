global process_context_switch

; void process_context_switch(struct Context ctx)
; 
; Context structure layout:
;   Offset 0x00: edi
;   Offset 0x04: esi
;   Offset 0x08: ebp
;   Offset 0x0C: esp (user stack)
;   Offset 0x10: ebx
;   Offset 0x14: edx
;   Offset 0x18: ecx
;   Offset 0x1C: eax
;   Offset 0x20: gs
;   Offset 0x24: fs
;   Offset 0x28: es
;   Offset 0x2C: ds
;   Offset 0x30: eip
;   Offset 0x34: eflags
;   Offset 0x38: page_directory_virtual_addr

section .text
process_context_switch:
    ; Step 1: Save base address of function argument ctx
    lea  ecx, [esp + 0x04] 

    ; Step 2: Setup iret stack (push in reverse order: SS, ESP, EFLAGS, CS, EIP)
    ; Push SS - user data segment selector (0x20 | 0x3 = 0x23)
    push 0x23 

    ; Push ESP - user stack pointer from ctx->cpu.stack.esp
    mov  eax, [ecx + 0xc] 
    push eax 

    ; Push EFLAGS from ctx->eflags
    mov  eax, [ecx + 0x34]
    push eax 

    ; Push CS - user code segment selector (0x18 | 0x3 = 0x1B)
    mov  eax, 0x18 | 0x3
    push eax 

    ; Push EIP from ctx->eip
    mov  eax, [ecx + 0x30]
    push eax 

    ; Step 3: Load all registers from ctx
    ; Load segment registers
    mov ds, [ecx + 0x2c] 
    mov es, [ecx + 0x28]
    mov fs, [ecx + 0x24]
    mov gs, [ecx + 0x20]

    ; Load stack base pointer
    mov ebp, [ecx + 0x8]

    ; Load general purpose registers
    mov edx, [ecx + 0x14]
    mov ebx, [ecx + 0x10]

    ; Load index registers
    mov esi, [ecx + 0x4] 
    mov edi, [ecx + 0]

    ; Step 4: Cleanup - load remaining registers
    ; Load eax first, then ecx last (since ecx is our pointer)
    mov eax, [ecx + 0x1c] 
    mov ecx, [ecx + 0x18]

    ; Step 5: Jump to process with iret
    ; iret will pop: EIP, CS, EFLAGS, ESP, SS and switch to user mode
    iret

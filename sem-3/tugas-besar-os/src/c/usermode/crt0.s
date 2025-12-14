global _start
extern main

section .text
; global main

_start:
	call main
	; mov ebx, eax
	; mov eax, 14
	; int 0x30
	jmp $
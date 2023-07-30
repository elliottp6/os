; tell linker to put this into the assembly section
section .asm

; 64-bit code
[BITS 64]

; imports
extern interrupt_handlers

; exports
global interrupt_wrappers

; number of total interrupts in x86_64
%define NUM_INTERRUPT_TABLE_ENTRIES 256
%define IRQ_0_7_COMMAND_PORT 0x20

; macro which builds an interrupt service routine
; TODO: note that we are NOT yet saving the SIMD registers, which could be clobbered by the interrupt handler
; ... we'll implement this later on
%macro write_interrupt_wrapper 1
    global int%1 ; export this as int0, int1, int2, ...
    int%1: ; label
        push rax
        push rcx
        push rdx
        push rsi
        push rdi
        push r8
        push r9
        push r10
        push r11
        mov rdi, %1 ; interrupt # as 1st arg for interrupt handler
        call qword [interrupt_handlers + %1 * 8]
        mov al, 0x20
        out IRQ_0_7_COMMAND_PORT, al ; acknowledge the IRQ
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
%endmacro

; create NUM_INTERRUPT_TABLE_ENTRIES instances of our interrupt service routine
%assign i 0
%rep NUM_INTERRUPT_TABLE_ENTRIES
    write_interrupt_wrapper i
    %assign i i+1
%endrep

; tell linker to put the rest of this file into the data section
section .data

; writes a 64-bit pointer to the ISR label
%macro write_interrupt_wrapper_pointer 1
    dq int%1
%endmacro

; table of 64-bit function pointers to the interrupt service routines
interrupt_wrappers:
%assign i 0
%rep NUM_INTERRUPT_TABLE_ENTRIES
    write_interrupt_wrapper_pointer i
    %assign i i+1
%endrep

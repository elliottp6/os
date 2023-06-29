; tell assembler where this code will be loaded into memory (so we know absolute location of labels).
; Bootloaders are loaded at 0x7C00 on x86_64 systems.
; Memory below this point is reserved for interrupt vectors, BIOS, BASIC, etc.
; 0x08000 is where first usable OS memory begins,
; so the bootloader will be loaded at 0x7C00 (which gives it 1KB of memory to work with)
ORG 0x7C00 

; tell assembler to generate 16-bit machine code
BITS 16

; entry point for 16-bit real mode
main16:
    call enable_a20_line
    call detect_long_mode
    jc .fail
    mov si, message_yes_long_mode
    call print
    jmp $
.fail:
    mov si, message_no_long_mode
    call print
    jmp $

; real-mode function
; enable a20 line using the FAST A20 option
; CAUTION: not all systems support FAST A20, and it may do something else unpredictable to the system instead
; this is fine for qemu purposes since it does support fast a20
; historical 8086 has a quirk called "memory wraparound" that some programs relied on, so the 20th bit of physical memory was latched to zero
; we MUST enable this so we have access to all of memory
enable_a20_line:
    in al, 0x92 ; read from port 0x92
    or al, 2
    out 0x92, al ; write to port 0x92
    ret

; real-mode function
enter_long_mode:
    ; TODO
    ret

; real-mode function
; carry flag is cleared if long-mode is supported, otherwise it is set
detect_long_mode:
    call detect_cpuid ; see if CPUID is supported
    jc .no_long_mode
    mov eax, 0x80000000 ; 'A' register holds arg for CPUID. 
    cpuid
    cmp eax, 0x80000001 ; check for the existence of extended functions
    jb .no_long_mode
    mov eax, 0x80000001 ; call extended function which tells us if long mode exists
    cpuid
    test edx, 1 << 29 ; LM bit is bit 29 in the D register
    jz .no_long_mode
    clc ; success, so clear carry and return
    ret
.no_long_mode
    stc ; failure, so set carry and return
    ret

; real-mode function
; carry flag is cleared if CPUID is supported, otherwise it is set
; this function clobbers EAX, ECX and CF
detect_cpuid:
    pushfd ; copy FLAGS into EAX and ECX
    pop eax
    mov ecx, eax
    xor eax, 1 << 21 ; flip bit 21 in EAX, put EAX into FLAGS, then move FLAGS back into EAX
    push eax
    popfd
    pushfd
    pop eax
    push ecx ; restore FLAGS from ECX
    popfd
    xor eax, ecx ; if EAX == ECX, then we were not able to flip the CPUID bit, so jump to .no_cpuid
    jz .no_cpuid
    clc ; Success: return w/ a clear carry flag
    ret
.no_cpuid:
    stc ; Failure: return w/ a set carry flag
    ret

; real-mode print routine
; si register holds a pointer to the null-terminated message
print:
    mov bx, 0 ; bx = counter
.loop:
    lodsb ; load si[b] into al (uses the DS:SI combo for segmented memory access)
    cmp al, 0 ; check for null terminator
    je .done ; if zero/null, jump to .done
    call print_char
    jmp .loop ; otherwise, print next char
.done:
    ret

; real-mode print_char routine
; al register holds the character to be printed
print_char:
    mov ah, 0eh ; command VIDEO TELETYPE OUTPUT (display char on screen & advance cursor)
    int 0x10 ; calls BIOS interrupt (lookup Ralf Brown's interrupt list) to print character
    ret

; static data
message_no_long_mode: db "ERROR: CPU does not support long mode.", 0x0A, 0x0D, 0 ; message-CR-LF-NULL
message_yes_long_mode: db "SUCCESS: CPU supports long mode.", 0x0A, 0x0D, 0

; padding & 2-byte boot-sector signature (to bring this binary up to 512 bytes)
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature

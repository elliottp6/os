%define KERNEL_ADDRESS 0x8000
%define KERNEL_SIZE_IN_SECTORS 1
%define CODE_SEG     0x08
%define DATA_SEG     0x10

[BITS 32]
main32:
    ; print 'K' character, so we know we've entered the kernel OK
    mov ebx,0xb8000    ; The video address
    mov al,'K'         ; The character to be print
    mov ah,0xF0        ; The color: white(F) on black(0)
    mov [ebx],ax

    ; if long mode is not supported, inform user and halt
    call detect_long_mode
    jc failed_to_support_long_mode

    ; print 'L' character, to indicate that long mode is supported
    mov ebx,0xb8000    ; The video address
    mov al,'L'         ; The character to be print
    mov ah,0xF0        ; The color: white(F) on black(0)
    mov [ebx],ax

    ; OPTIONAL: setup a new, larger stack
    ;mov ebp, STACK32_ADDRESS
    ;mov esp, ebp

    ; ok, now it's time to enter long mode!
    jmp $

; print 'F' character, to indicate that long-mode is not supported
failed_to_support_long_mode:
    mov ebx,0xb8000    ; The video address
    mov al,'F'         ; The character to be print
    mov ah,0xF0        ; The color: white(F) on black(0)
    mov [ebx],ax
    jmp $

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
    .no_long_mode:
    stc ; failure, so set carry and return
    ret

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

; long mode main
[BITS 64]
main64:
    ; Blank out the screen to a blue color.
    mov edi, 0xB8000
    mov rcx, 500                      ; Since we are clearing uint64_t over here, we put the count as Count/4.
    mov rax, 0x1F201F201F201F20       ; Set the value to set the screen to: Blue background, white foreground, blank spaces.
    rep stosq                         ; Clear the entire screen. 
 
    ; Display "Hello World!"
    mov edi, 0x00b8000              
    mov rax, 0x1F6C1F6C1F651F48    
    mov [edi],rax
    mov rax, 0x1F6F1F571F201F6F
    mov [edi + 8], rax
    mov rax, 0x1F211F641F6C1F72
    mov [edi + 16], rax
    jmp $

; padding to make this file 16-byte aligned (which allows it to mix w/ C object files in the same text section)
times 512-($ - $$) db 0

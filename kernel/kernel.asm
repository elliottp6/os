%define CODE_SEG     0x08
%define DATA_SEG     0x10

[BITS 32]
main32:
    ; print '&' character, so we know we've entered the kernel OK
    mov ebx,0xb8000    ; The video address
    mov al,'&'         ; The character to be print
    mov ah,0x0F        ; The color: white(F) on black(0)
    mov [ebx],ax

    ; ok, now it's time to enter long mode!
    jmp $

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

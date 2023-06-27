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
    mov si, message_hello_world ; say hi
    call print
    jmp $ ; jump to self ($ = self) which is an infinite loop

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
message_hello_world: db 'Hello world', 0 ; null-terminated string

; padding & 2-byte boot-sector signature (to bring this binary up to 512 bytes)
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature

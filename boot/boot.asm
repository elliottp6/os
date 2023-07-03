ORG 0x7C00

%define STACK_ADDRESS 0x8000
%define CODE_SEG     0x08
%define DATA_SEG     0x10
%define KERNEL_SIZE_IN_SECTORS 1
%define KERNEL_ADDRESS 0x10000

[BITS 16]
main16:
    ; setup real mode segment registers (note that we cannot do this in a routine, b/c the 'ret' instruction requires the stack)
    cli ; disable interrupts (must do this before we mess w/ stack pointer, or else an interrupt could corrupt the stack)
    xor ax, ax ; clear ax
    mov ss, ax ; stack segment = 0
    mov ds, ax ; data segment = 0
    mov es, ax ; extra segment = 0
    mov fs, ax ; (OS's generally use this for thread-specific memory) = 0
    mov gs, ax ; (same here) = 0
    mov ax, STACK_ADDRESS ; stack pointer
    mov sp, ax
    
    ; ok, now we have a flat memory model. enable the A20 physical line so we have access to all memory
    call enable_a20_line

    ; say hello
    mov esi, message_entering_protected_mode
    call print_16

    ; enter protected mode
    lgdt[gdt_descriptor] ; load GDT
    mov eax, cr0 ; turn on 1st bit of cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:main32

; GDT (global descriptor table)
gdt_start:
gdt_null: ; null segment
    dd 0x0
    dd 0x0
gdt_code: ; code segment, offset 0x8, linked to CS
    dw 0xffff
    dw 0 ; base first 0-15 bits
    db 0 ; base 16-32 bits
    db 0x9A ; access byte
    db 11001111b ; high 4 bit flats & low 4 bit flags
    db 0 ; base 24-31 bits
gdt_data: ; data segment, offset 0x10, linked to DS, SS, ES, FS, GS
    dw 0xffff
    dw 0 ; base first 0-15 bits
    db 0 ; base 16-32 bits
    db 0x92 ; access byte
    db 11001111b ; high 4 bit flats & low 4 bit flags
    db 0 ; base 24-31 bits
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size
    dd gdt_start ; offset

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

; si register holds a pointer to the null-terminated message
print_16:
    mov bx, 0 ; bx = counter
    .print_loop_16:
    lodsb ; load si[b] into al (uses the DS:SI combo for segmented memory access)
    cmp al, 0 ; check for null terminator
    je .done_printing_16 ; if zero/null, jump to .done
    call print_char_16
    jmp .print_loop_16 ; otherwise, print next char
    .done_printing_16:
    ret

; al register holds the character to be printed
print_char_16:
    mov ah, 0eh ; command VIDEO TELETYPE OUTPUT (display char on screen & advance cursor)
    int 0x10 ; calls BIOS interrupt (lookup Ralf Brown's interrupt list) to print character
    ret

[BITS 32]
main32:
    ; setup protected mode segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; print '%' character, so we know we've entered protected mode OK
    mov ebx,0xb8000    ; The video address
    mov al,'%'         ; The character to be print
    mov ah,0x0F        ; The color: white(F) on black(0)
    mov [ebx],ax

    ; ok, now load the kernel and run it
    mov eax, 1 ; LBA 1 (logical block address 1, which is just past the bootloader)
    mov ecx, KERNEL_SIZE_IN_SECTORS ; load entire kernel
    mov edi, KERNEL_ADDRESS ; where to load kernel
    call ata_lba_read ; loads kernel into memory
    jmp CODE_SEG:KERNEL_ADDRESS ; jump to kernel

ata_lba_read:
    ; send the highest 8 bits of the LBA to the HD controller
    mov ebx, eax ; backup LBA (logical block address)
    shr eax, 24
    or eax, 0xE0 ; select the master drive
    mov dx, 0x1F6 ; this is the port that use to talk to the HD controller
    out dx, al ; set port
    
    ; send the total sectors to read to port 0x1F2
    mov eax, ecx 
    mov dx, 0x1F2
    out dx, al

    ; send more bits of the LBA
    mov eax, ebx ; restoring LBA to EAX
    mov dx, 0x1F3
    out dx, al

    ; send more bits of the LBA
    mov dx, 0x1F4
    mov eax, ebx; restore the backup LBA
    shr eax, 8
    out dx, al

    ; send upper 16 bits of the LBA
    mov dx, 0x1F5
    mov eax, ebx ; restor the backup LBA
    shr eax, 16
    out dx, al

    ; ???
    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    ; read all sectors into memory
.next_sector:
    push ecx

.try_again:
    ; check if we need to read
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again

    ; need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw ; rep = repeat 'ecx' times: insw = read a word from port 'dx' and store into where 'edi' points to???
    pop ecx ; restore ecx (total # of sectors to read)
    loop .next_sector ; loop while decrementing ecx

    ; done
    ret

; string table
message_entering_protected_mode: db "About to enter protected mode...", 0x0A, 0x0D, 0 ; message-CR-LF-NULL
message_entered_protected_mode: db "Entered protected mode!", 0x0A, 0x0D, 0

; padding & 2-byte boot-sector signature (to bring this binary up to 512 bytes)
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature

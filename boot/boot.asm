; BIOS will load us here, so tell assembler how to set label addresses appropriately
; since bootloader is 512 bytes, it will live from 0x7C00 to 0x7E00
ORG 0x7C00

; put the bootloader's stack right above the bootloader code
; 0x7F00 has 0x100 (256 bytes) of growth before it will overlap the bootloder code
; this is plenty (we only use a few bytes of stack right now)
%define STACK_ADDRESS 0x7F00

; load the kernel right above the bootloader's stack
%define KERNEL_ADDRESS 0x8000

; define KERNEL_SECTORS (defined externally by makefile, since the size of the kernel cannot be known until after compilation)
%include "bin/kernel_sectors.inc"

; in protected mode, we'll use gdt_entry_1 for code, and gdt_entry_2 for data
; these are the offsets into the GDT
%define CODE_SEG 0x08
%define DATA_SEG 0x10

[BITS 16]
boot16:
    ; disable interrupts, so interrupt handlers don't cause corruption while we're manipulating critical things like the stack pointer etc.
    cli
    
    ; clear real mode segment registers to give us a flat 64KB memory model
    xor ax, ax ; clear ax
    mov ss, ax ; stack segment = 0
    mov ds, ax ; data segment = 0
    mov es, ax ; extra segment = 0
    mov fs, ax ; (OS's generally use this for thread-specific memory) = 0
    mov gs, ax ; (same here) = 0

    ; setup our stack
    mov ax, STACK_ADDRESS
    mov sp, ax
    
    ; enable the A20 physical line so we have access to all memory
    call enable_a20_line

    ; load GDT into the GDTR (global descriptor table register), so that protected mode will know where to find it
    lgdt[gdt_descriptor] 

    ; tell user we're about to enter protected mode
    mov esi, message_entering_protected_mode
    call print_16

    ; enter protected mode by turning on 1st bit of cr0
    mov eax, cr0 
    or eax, 1
    mov cr0, eax

    ; set the the CS register to the gtd table offset for the gdt code entry, and jump to main32
    jmp CODE_SEG:boot32

; size and offset information for GDT
gdt_descriptor:
    dw gdt_end - gdt_entry0 - 1 ; size
    dd gdt_entry0 ; offset

; GDT (global descriptor table). this tells the CPU about memory segs in protected mode.
; full details @ https://wiki.osdev.org/Global_Descriptor_Table
gdt_entry0:
    dd 0, 0 ; a segment selector that points to entry0 is a "null segment selector", and will cause general-protection-exception (#GP) if memory is accessed.
gdt_entry1: ; we'll use this segment for code. Set base = 0, limit = 0xFFFFF in pages which is 1048575 * 4096 = 4GB. Set access bytes to (1st = 1 for valid segment, 2nd/3rd = 0 for kernel privilege level, 4th 1 for code/data seg, 5th 1 for execution-enabled, 6th direction bit which 0 means segment grows upwards, RW bit is readable/writable bit which is ON which means we can read if code seg or write if data seg (write access never allowed for code segs, read access never allowed for data segs), finally last is accessed-bit which is left at 0 and used by CPU). Set flags to 1100 (top bit means we're specifying limit in terms of pages, second-from-top bit means we're defining a 32-bit protected mode segment, finally 0 for long-mode bit and 0 for reserved bit).
    dw 0xffff, 0 ; 0-15 bits of limit = 0xffff (i.e. unlimited), 0-15 bits of base = 0 (i.e. do not do any offset in physical memory)
    db 0, 10011010b, 11001111b, 0 ; 16-32 bits of base = 0, access byte, 4-bit flags + 16-19 bits of limit), 24-31 bits of base = 0
gdt_entry2: ; we'll use this segment for data, same as entries for code except for the access byte, which disallows execution
    dw 0xffff, 0
    db 0, 10010010b, 11001111b, 0
gdt_end:

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
boot32:
    ; setup protected mode segment registers to point to our data segemnt entry in the GDT
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; print 'P' character, so we know we've entered protected mode OK
    mov ebx,0xb8000    ; The video address
    mov al,'P'         ; The character to be print
    mov ah,0xF0        ; The color: white(F) on black(0)
    mov [ebx],ax

    ; ok, now load the kernel from disk
    mov eax, 1 ; LBA 1 (logical block address 1, which is just past the bootloader)
    mov ecx, KERNEL_SECTORS
    mov edi, KERNEL_ADDRESS
    call ata_lba_read ; loads kernel into memory

    ; jump to kernel, along with arguments from the bootloader
    push KERNEL_SECTORS ; # of sectors that the kernel takes up (a 32-bit int since this is 32-bit code)
    jmp KERNEL_ADDRESS ; jump to kernel

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
message_entering_protected_mode: db "Entering protected mode...", 0x0A, 0x0D, 0 ; message-CR-LF-NULL

; padding & 2-byte boot-sector signature (to bring this binary up to 512 bytes)
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature

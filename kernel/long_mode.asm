; tell assembler where this code will be loaded into memory (so we know absolute location of labels)
; Bootloaders are loaded at 0x7C00 on x86_64 systems
; Everything below 0x7C00 is reserved by the BIOS.
; This means that 0x7C00 to 0x7E00 is 512-bytes for the stage 1 bootloader (this file).
; Above this we can do WHATEVER WE WANT.
; Put the stack at 0x9000 growing down. This gives us some space to load the stage 2 bootsector.
; Put the long mode page tables at 0xA000, which will be 16KB worth of data.
; We can load the OS itself at a much higher address, 0x10000 will not clobber anything else.
ORG 0x7C00

; constants
%define STACK_ADDRESS 0x9000
%define LONG_MODE_PAGE_TABLE_ADDRESS 0xA000
%define PAGE_PRESENT    (1 << 0)
%define PAGE_WRITE      (1 << 1)
%define CODE_SEG     0x0008
%define DATA_SEG     0x0010

; entry point for 16-bit real mode
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
    sti ; enable interrupts

    ; ok, now we have a flat memory model. enable the A20 physical line so we have access to all memory
    call enable_a20_line

    ; see if long mode is supported. if not, the procedure will inform user and then loop forever
    call require_long_mode

    ; build the long mode page map from LONG_MODE_PAGE_TABLE_ADDRESS
    mov edi, LONG_MODE_PAGE_TABLE_ADDRESS ; edi argument tells 'enter_long_mode' where to put page data
    call build_long_mode_2MB_page_table

    ; enter long mode
    mov edi, LONG_MODE_PAGE_TABLE_ADDRESS
    jmp enter_long_mode

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
; es:edi must point to a 16KB PML4E buffer
enter_long_mode:
    ; disable IRQs (interrupt requests) TODO: why is this different from cli/sti?
    mov al, 0xFF
    out 0xA1, al
    out 0x21, al
    nop ; no-ops (why???)
    nop
    lidt [IDT] ; load a zero-length interrupt-descriptor-table, which means any NMI (non-maskable-interrupt) will cause a triple fault (a non-recoverable fault, which reboots the CPU, or in qemu it will dump w/ the instruction pointer @ instruction that caused the first exception.)

    ; must enable PAE and PGE on CR4 before we can enter long mode
    mov eax, 10100000b ; set PAE (physical address extension) and PGE (page global enable) bit [page table entries marked as global will be cached in the TLB across different address spaces]
    mov cr4, eax

    ; CR3 register must point to the PML4 (page map level 4)
    mov edx, edi
    mov cr3, edx

    ; enable long-mode by turning on the LME bit in the EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x00000100
    wrmsr

    ; enable paging & protection
    mov ebx, cr0
    or ebx, 0x80000001
    mov cr0, ebx

    ; load 64-bit global descriptor table
    lgdt [GDT.Pointer]

    ; start executing 64-bit code
    jmp CODE_SEG:main64

; es:edi must point to page-aligned 16KB buffer (for the PML4, PDPT, PD and a PT)
; ss:esp must point to memory that can be used as a small stack
; this creates a page map with a total system memory of 2MB
; (i.e. a full page table pointing to physical memory, but with only a single 0 entry for the PML4, PDPT and PD tables)
build_long_mode_2MB_page_table:
    ; zero-out the entire 16KB buffer
    push di ; backup DI (otherwise, clobbered by rep stosd)
    mov ecx, 0x1000 ; set ECX to 4096
    xor eax, eax ; clear EAX
    cld ; clear direction-flag
    rep stosd ; repeat-while-equal: store contents of eax into [edi], inc/dec edi each time by 4 bytes each time (4096 loops = 16KB)
    pop di ; restore DI
    
    ; write 1st entry of page map level 4 [PML4 9 bits (47-39) for 512 entries (PML4E) [512GB each]]
    lea eax, [es:di + 0x1000] ; put address of PDPT into EAX
    or eax, PAGE_PRESENT | PAGE_WRITE ; page present & writable
    mov [es:di], eax ; store value of EAX into first PML4E

    ; write 1st entry of page directory pointer table [PDPT 9 bits (bits 38-30) for 512 entries (PDPE) [1GB each]]
    lea eax, [es:di + 0x2000] ; put address of PD into EAX
    or eax, PAGE_PRESENT | PAGE_WRITE
    mov [es:di + 0x1000], eax ; store value of EAX into first PDPT

    ; write 1st entry of page directory [PD 9 bits (29-21) for 512 entries (PDE) [2MB each]]
    lea eax, [es:di + 0x3000] ; put address of PT into EAX
    or eax, PAGE_PRESENT | PAGE_WRITE
    mov [es:di + 0x2000], eax ; store value of EAX into first PDE

    ; write all entries of page table [PT 9 bits (20-12) for 512 entries (PE) [4K each]; leaving 12 bits (11-0) for 4096 byte pages]
    ; this maps 1:1 virtual-to-physical memory from 0 to 2MB
    push di ; save DI
    lea di, [di + 0x3000] ;  point DI to the page table
    mov eax, PAGE_PRESENT | PAGE_WRITE ; EAX starts pointing to memory address 0 w/ flags
    .page_table_loop:
    mov [es:di], eax ; first page table entry points to memory address 0
    add eax, 0x1000 ; next one points 4096 bytes later
    add di, 8 ; each page table entry is an 8 byte pointer
    cmp eax, 0x200000 ; 2MB
    jb .page_table_loop ; jump if eax is below 2MB
    pop di ; restore DI
    ret

; will cause the machine to halt
require_long_mode:
    call detect_long_mode
    jc .failed_to_support_long_mode
    ret
    .failed_to_support_long_mode:
    mov si, message_no_long_mode
    call print
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

; si register holds a pointer to the null-terminated message
print:
    mov bx, 0 ; bx = counter
    .print_loop:
    lodsb ; load si[b] into al (uses the DS:SI combo for segmented memory access)
    cmp al, 0 ; check for null terminator
    je .done_printing ; if zero/null, jump to .done
    call print_char
    jmp .print_loop ; otherwise, print next char
    .done_printing:
    ret

; al register holds the character to be printed
print_char:
    mov ah, 0eh ; command VIDEO TELETYPE OUTPUT (display char on screen & advance cursor)
    int 0x10 ; calls BIOS interrupt (lookup Ralf Brown's interrupt list) to print character
    ret

; long mode main
[BITS 64]
main64:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Blank out the screen to a blue color.
    mov edi, 0xB8000
    mov rcx, 500                      ; Since we are clearing uint64_t over here, we put the count as Count/4.
    mov rax, 0x1F201F201F201F20       ; Set the value to set the screen to: Blue background, white foreground, blank spaces.
    rep stosq                         ; Clear the entire screen. 

    ; load kernel
    ;mov eax, 1 ; LBA 1 (logical block address 1, which is just past the bootloader)
    ;mov ecx, 1 ; 1 sectors (entire kernel)
    ;mov edi, 0x100000 ; loads kernel into 1MB position in memory
    ;call ata_lba_read ; loads kernel into memory
    ;push word CODE_SEG
    ;push qword 0x100000
    ;retf
    ;jmp CODE_SEG:0x100000 ; jump to kernel '_start'
 
    ; Display "Hello World!"
    ;mov edi, 0x00b8000              
    ;mov rax, 0x1F6C1F6C1F651F48    
    ;mov [edi],rax
    ;mov rax, 0x1F6F1F571F201F6F
    ;mov [edi + 8], rax
    ;mov rax, 0x1F211F641F6C1F72
    ;mov [edi + 16], rax
    jmp $

; zero-length IDT structure
ALIGN 4
IDT:
    .Length       dw 0
    .Base         dd 0

; Global Descriptor Table
GDT:
.Null:
    dq 0x0000000000000000             ; Null Descriptor - should be present.
.Code:
    dq 0x00209A0000000000             ; 64-bit code descriptor (exec/read).
    dq 0x0000920000000000             ; 64-bit data descriptor (read/write).
ALIGN 4
    dw 0                              ; Padding to make the "address of the GDT" field aligned on a 4-byte boundary
.Pointer:
    dw $ - GDT - 1                    ; 16-bit Size (Limit) of GDT.
    dd GDT                            ; 32-bit Base Address of GDT. (CPU will zero extend to 64-bit) 

; string table
message_no_long_mode: db "ERR: no long mode.", 0x0A, 0x0D, 0 ; message-CR-LF-NULL

; padding & 2-byte boot-sector signature (to bring this binary up to 512 bytes)
times 510-($ - $$) db 0 ; fill 510 - (size = (location - origin)) to bring us to 510 bytes
dw 0xAA55 ; signature

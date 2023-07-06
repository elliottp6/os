; constants for kernel address, size, and GDT offsets for code & data
; ORG 0x8000 (don't need this here b/c we're linking this file using linker.ld)
%define KERNEL_ADDRESS 0x8000
%define KERNEL_SIZE_IN_SECTORS 1
%define CODE_SEG 0x08
%define DATA_SEG 0x10

; constants for page map
%define LONG_MODE_PAGE_TABLE_ADDRESS 0xA000
%define PAGE_PRESENT (1 << 0)
%define PAGE_WRITE (1 << 1)

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

    ; build the long-mode page map
    mov edi, LONG_MODE_PAGE_TABLE_ADDRESS ; edi argument tells 'enter_long_mode' where to put page data
    call build_long_mode_2MB_page_table

    ; enter long mode
    mov edi, LONG_MODE_PAGE_TABLE_ADDRESS
    jmp enter_long_mode

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

    ; select the 64-bit code segment while jumping to 64-bit main
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
extern main
main64:
    ; ??
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; OPTIONAL: setup a new, larger stack
    ;mov ebp, STACK32_ADDRESS
    ;mov esp, ebp

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

    ; enter into the C code
    ; TODO: we need to rebuild our cross-compiler to generate 64-bit code, or else this call fails!
    ;call main

    ; after C code runs, display 'system shutdown', then loop forever
    mov edi, 0x00b8000              
    mov rax, 0x1F5C1F6C1F651F48    
    mov [edi],rax
    mov rax, 0x1F6F1F571F201F6F
    mov [edi + 8], rax
    mov rax, 0x1F211F641F6C1F72
    mov [edi + 16], rax
    jmp $

; zero-length IDT structure
ALIGN 4
IDT:
    .Length       dw 0
    .Base         dd 0

; Long-Mode Global Descriptor Table
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

; padding to make this file 16-byte aligned (which allows it to mix w/ C object files in the same text section)
times 512-($ - $$) db 0

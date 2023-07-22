%define CODE_SEG 0x08
%define DATA_SEG 0x10
%define PAGE_FLAG_PRESENT (1 << 0)
%define PAGE_FLAG_WRITE (1 << 1)
%define KERNEL_ADDRESS 0x100000
%define KERNEL_STACK_ADDRESS 0x200000
%define KERNEL_STACK_SIZE 4096
%define LONG_MODE_PAGE_MAP_ADDRESS 0xA000

; ENABLE THESE FOR 4096-size pages w/ 2MB of initial memory acccess:
;%define PAGE_FLAG_HUGE 0
;%define PAGE_SIZE 4096
;%define PT_OFFSET 0x3000

; OR: ENABLE THESE FOR 2MB-size pages w/ 1GB of initial memory access:
%define PAGE_FLAG_HUGE (1 << 7)
%define PAGE_SIZE 0x200000
%define PT_OFFSET 0x2000

global start32 ; tell linker where to find this entry point
extern main ; allows start.asm to call into main.c

[BITS 32]
start32:
    ; note that stack has the KERNEL_SECTORS 32-bit int from the bootloader on the stack
    ; we'll retrieve this later in start64

    ; print 'K' character, so we know we've entered the kernel OK
    mov ebx,0xb8000    ; The video address
    mov al,'K'         ; The character to be print
    mov ah,0xF0        ; The color: white(F) on black(0)
    mov [ebx],ax

    ; if long mode is not supported, inform user and halt
    call detect_long_mode
    jc failed_to_support_long_mode

    ; build the long-mode page map
    mov edi, LONG_MODE_PAGE_MAP_ADDRESS ; edi argument tells 'enter_long_mode' where to put page data
    call build_long_mode_page_map

    ; enter long mode
    mov edi, LONG_MODE_PAGE_MAP_ADDRESS
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

    ; must enable PGE, PAE and PSE (for 2MB pages) on CR4 before we can enter long mode
    ; enable PGE (page global enable), PAE (physical address extension) and PSE (page size extension, on = 2MB huge pages)
    mov eax, 10110000b
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
    jmp CODE_SEG:start64

; es:edi must point to page-aligned 16KB buffer (for the PML4, PDPT, PD and a PT)
; ss:esp must point to memory that can be used as a small stack
; this creates an identity page map
build_long_mode_page_map:
    ; zero-out the entire 16KB buffer
    push di ; backup DI (otherwise, clobbered by rep stosd)
    mov ecx, 0x1000 ; set ECX to 4096
    xor eax, eax ; clear EAX
    cld ; clear direction-flag
    rep stosd ; repeat-while-equal: store contents of eax into [edi], inc/dec edi each time by 4 bytes each time (4096 loops = 16KB)
    pop di ; restore DI
    
    ; write level 3 pagetable
    lea eax, [es:di + 0x1000] ; put address of PDPT into EAX
    or eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE ; page present & writable
    mov [es:di], eax ; store value of EAX into first PML4E

    ; write level 2 pagetabel
    lea eax, [es:di + 0x2000] ; put address of PD into EAX
    or eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    mov [es:di + 0x1000], eax ; store value of EAX into first PDPT
 
    ; write level 1 pagetable
    lea eax, [es:di + 0x3000] ; put address of PT into EAX
    or eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    mov [es:di + 0x2000], eax ; store value of EAX into first PDE

    ; write level 0 pagetable
    ; note that if we're using 2MB pages, then this will actually write the level 1 pagetable (and we won't need the full 16KB, just 12KB)
    push di ; save DI
    lea di, [di + PT_OFFSET] ;  point DI to the PT_OFFSET
    mov eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_HUGE; EAX starts pointing to memory address 0 w/ flags
.page_table_loop:
    mov [es:di], eax ; first page table entry points to memory address 0
    add eax, PAGE_SIZE ; next one points to PAGE_SIZE later physical address
    add di, 8 ; each page table entry is an 8 byte pointer
    cmp eax, PAGE_SIZE * 512 ; stop @ 512 pages
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
start64:
    ; ??
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Blank out the screen to a blue color, to indicate that we're in long mode and about to enter the kernel's main C function
    mov edi, 0xB8000                  ; address of vga text mode buffer
    mov rcx, 500                      ; Since we are clearing uint64_t over here, we put the count as Count/4.
    mov rax, 0x1F201F201F201F20       ; Set the value to set the screen to: Blue background, white foreground, blank spaces.
    rep stosq                         ; Clear the entire screen. 

    ; setup a 1MB stack @ 2MB
    mov eax, KERNEL_STACK_ADDRESS
    mov ebp, eax ; set stack base pointer
    mov esp, ebp ; set stack pointer

    ; TODO: alternatively, setup a new 1MB stack right after the kernel
    ;pop rax ; get the KERNEL_SECTORS that we pushed from the bootloader
    ;add rsp, 4 ; adjust stack b/c when we pushed it was only 32-bits not 64-bits
    ;mov eax, eax ; blank our the higher 32-bits
    ;mul rax, 512 ; convert # sectors into actual bytes
    ;add eax, KERNEL_ADDRESS ; convert into address
    ;mov ebp, eax ; set base stack pointer
    ;mov esp, ebp ; stack stack pointer

    ; re-enable interrupts
    sti

    ; enter into the C code
    call main

    ; suspend the machine (ready for power off)
suspend:
    cli
    hlt
    jmp suspend

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

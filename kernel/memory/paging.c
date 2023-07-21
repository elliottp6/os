#include <stdint.h>
#include "paging.h"

#define PAGE_FLAG_PRESENT 1
#define PAGE_FLAG_WRITE 2
#define PAGE_SIZE 4096
#define PAGE_BITS 12
#define PAGE_TABLE_BITS 9
#define PAGE_TABLE_ENTRIES 512
#define PAGE_TABLE_MASK 511
#define PAGE_TABLE_MAX_MEMORY 0x10000000 // 256 MB

typedef struct page_table {
    uint64_t entries[PAGE_TABLE_ENTRIES];
} page_table_t;

static size_t shift_right_plus_remainder( size_t x, size_t shift ) {
    size_t mask = (1 << shift) - 1;
    return (x >> shift) | ((x & mask) > 0);
}

void paging_init_kernel_page_tables() {
    // calculate how many total pages we need for each page table level
    //size_t kernel_pages = shift_right_plus_remainder( PAGE_TABLE_MAX_MEMORY, PAGE_BITS );
    //size_t table0_length = shift_right_plus_remainder( kernel_pages, PAGE_TABLE_BITS ); // PT (page table)

    /*
    size_t kernel_pages = (PAGE_TABLE_MAX_MEMORY >> PAGE_BITS) + 
           table0_entries = kernel_pages >> PAGE_TABLE_BITS,    
           table1_entries = table0_entries >> PAGE_TABLE_BITS,  // PD (page directory)
           table2_entries = table1_entries >> PAGE_TABLE_BITS,  // PDPT (page directory pointer table)
           table3_entries = table2_entries >> PAGE_TABLE_BITS;  // PML4 (page map level 4)
    */
    // 

    // 

    /*
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
    */
}

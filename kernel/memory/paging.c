#include <stdint.h>
#include "paging.h"
#include "../text/string.h"
#include "../text/vga_text.h"

#define PAGE_FLAG_PRESENT 1
#define PAGE_FLAG_WRITE 2
#define PAGE_BITS 12
#define PAGE_SIZE (1 << PAGE_BITS)
#define PAGETABLE_BITS 9
#define PAGETABLE_ENTRIES (1 << PAGETABLE_BITS)
#define PAGEMAP_LEVELS 4
#define PAGEMAP_MAX_MEMORY 0x40000000 // 1GB

typedef struct pagetable {
    uint64_t entries[PAGETABLE_ENTRIES];
} pagetable_t;

static size_t shift_right_round_up( size_t x, size_t shift ) {
    size_t mask = (1 << shift) - 1, remainder = x & mask;
    return (x >> shift) | (remainder > 0);
}

// note: *num_pagetables_per_level must point to a an array of size_t w/ length PAGEMAP_LEVELS
static size_t get_pagemap_dimensions( size_t memory_size, size_t *num_pages, size_t *num_pagetables_per_level ) {
    // calculate # of pages needed to map memory from 0 to PAGEMAP_MAX_MEMORY
    *num_pages = shift_right_round_up( memory_size, PAGE_BITS );

    // calculate # of pagetables needed at each level in the pagemap hierarchy
    size_t pagemap_size = 0;
    for( size_t i = 0; i < PAGEMAP_LEVELS; i++ ) {
        num_pagetables_per_level[i] = shift_right_round_up( i > 0 ? num_pagetables_per_level[i - 1] : *num_pages, PAGETABLE_BITS );
        pagemap_size+= num_pagetables_per_level[i] * sizeof( pagetable_t );
    }

    // return # of bytes required for the entire pagemap
    return pagemap_size;
}

// TODO: this can probably be removed later, but for now it was useful just to double check the get_pagemap_dimensions function
// later on, we really should write proper tests, and have them run in POST
static void paging_print_pagemap_dimensions( size_t num_pages, size_t *num_pagetables_per_level, size_t pagemap_size ) {
    // print out dimensions
    vga_text_print( "pagemap dimensions for memory_size ", 0x17 );
    vga_text_print( string_int64_to_temp( (int64_t)PAGEMAP_MAX_MEMORY ), 0x17 );
    vga_text_print( "\n  # pages: ", 0x17 );
    vga_text_print( string_int64_to_temp( (int64_t)num_pages ), 0x17 );
    for( size_t i = 0; i < PAGEMAP_LEVELS; i++ ) {
        vga_text_print( "\n  # tables for level ", 0x17 );
        vga_text_print( string_int64_to_temp( (int64_t)i ), 0x17 );
        vga_text_print( ": ", 0x17 );
        vga_text_print( string_int64_to_temp( (int64_t)num_pagetables_per_level[i] ), 0x17 ); 
    }
    vga_text_print( "\n  pagemap size: ", 0x17 );
    vga_text_print( string_int64_to_temp( (int64_t)pagemap_size ), 0x17 );
    vga_text_print( "\n", 0x17 );
}

void paging_init_kernel_pagemap() {
    // get pagemap dimensions
    size_t num_pages,
           num_pagetables_per_level[PAGEMAP_LEVELS],
           pagemap_size = get_pagemap_dimensions( PAGEMAP_MAX_MEMORY, &num_pages, num_pagetables_per_level );

    // print this data out to the user
    paging_print_pagemap_dimensions( num_pages, num_pagetables_per_level, pagemap_size );

    // the initial pagemap 

    // TODO: allocate the pagemap

    // TODO: populate the pagemap

    // TODO: switch the CR3 register to point to the pagemap

    // TODO: flush the TLB
    //cli
    //static inline void __native_flush_tlb_single(unsigned long addr) {
    //   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
    //}
	//	sti

    // return the # of bytes needed for the entire pagemap
    //return pagemap_size;

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

#include <stdint.h>
#include "paging.h"
#include "../text/string.h"
#include "../text/vga_text.h"

#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE (1 << 1)
#define PAGE_FLAG_HUGE (1 << 7)
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
    vga_text_print( string_from_int64( (int64_t)PAGEMAP_MAX_MEMORY ), 0x17 );
    vga_text_print( "\n  # pages: ", 0x17 );
    vga_text_print( string_from_int64( (int64_t)num_pages ), 0x17 );
    for( size_t i = 0; i < PAGEMAP_LEVELS; i++ ) {
        vga_text_print( "\n  # tables for level ", 0x17 );
        vga_text_print( string_from_int64( (int64_t)i ), 0x17 );
        vga_text_print( ": ", 0x17 );
        vga_text_print( string_from_int64( (int64_t)num_pagetables_per_level[i] ), 0x17 ); 
    }
    vga_text_print( "\n  pagemap size: ", 0x17 );
    vga_text_print( string_from_int64( (int64_t)pagemap_size ), 0x17 );
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
    /* TODO: test to make sure entire address space is accessible
    // test accessing last byte of memory (1GB)
    char *p = (char*)0x40000000 - 1;
    vga_text_print( "reading last byte of memory (address 1GB - 1): ", 0x17 );
    vga_text_print( string_int64_to_temp( (int64_t)*p ), 0x17 );
    vga_text_print( "\n", 0x17 );
    */

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
}

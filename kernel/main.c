#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "text/string.h"
#include "text/vga_text.h"
#include "memory/paging.h"
#include "memory/kernel_heap.h"

void panic( const char* details ) {
    // print messages
    vga_text_print( "System panic!\n", 0x4F );
    if( NULL != details ) vga_text_print( details, 0x4F );

    // suspend CPU
    while( true ) { asm ( "cli\n" "hlt\n" ); }
}

static void divide_by_zero() {
    asm (
        "xor %rax, %rax\n" // set rax to 0
        "xor %rdx, %rdx\n" // set rdx to 0
        "div %rdx\n" // divide rax / rdx, which will cause the 
    );
}

void main() {
    // clear background to blue, and display welcome message
    vga_text_clear( 0x17 );
    vga_text_print( "Welcome to the 64-bit kernel!\n", 0x17 );

    // TODO: convert this into a legit test
    // test 'string_int64_to_temp'
    vga_text_print( "testing string_int64_to_temp for -1054: ", 0x17 );
    vga_text_print( string_int64_to_temp( -1054 ), 0x17 );
    vga_text_print( "\n", 0x17 );

    // TODO: convert this into a legit test
    // test accessing last byte of memory (1GB)
    char *p = (char*)0x40000000 - 1;
    vga_text_print( "reading last byte of memory (address 1GB - 1): ", 0x17 );
    vga_text_print( string_int64_to_temp( (int64_t)*p ), 0x17 );
    vga_text_print( "\n", 0x17 );

    // initialize the kernel heap
    kernel_heap_init();

    // test the kernel heap. 1st object should be 40 bytes away from the heap's 2MB start, b/c we need 32 bytes for the heap's header, and then 8 bytes for the block's size.
    void *obj = kernel_heap_alloc( 8 );
    if( (int64_t)obj != (int64_t)(0x200000 + 40) ) panic( "kernel_heap_alloc: 1st allocated object must be 8 bytes after the heap's header" );
    vga_text_print( "kernel_heap_alloc: 1st object allocated @ ", 0x17 );
    vga_text_print( string_int64_to_temp( (int64_t)obj ), 0x17 );
    vga_text_print( "\n", 0x17 );

    // TODO: do kernel main stuff
    // divide_by_zero();
    // panic( "attempted to divide by 0\n" );

    // machine is now ready for power off
    vga_text_print( "Exiting kernel & suspending CPU. Machine is now ready to be powered off.\n", 0x06 );
}

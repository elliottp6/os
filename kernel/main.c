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

    // test the kernel heap
    // TODO: major bug here! the heap returns NULL here!
    int64_t *ptr = (int64_t*)kernel_heap_alloc( 8 );
    vga_text_print( "1st object allocated on the kernel heap @ ", 0x17 );
    vga_text_print( string_int64_to_temp( (int64_t)ptr ), 0x17 );
    vga_text_print( "\n", 0x17 );

    // TODO: do kernel main stuff
    // divide_by_zero();
    // panic( "attempted to divide by 0\n" );

    // machine is now ready for power off
    vga_text_print( "Exiting kernel & suspending CPU. Machine is now ready to be powered off.\n", 0x06 );
}

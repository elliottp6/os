#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "text/string.h"
#include "text/vga_text.h"
#include "memory/kernel_heap.h"

static void panic( const char* details ) {
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
    vga_text_print( "Welcome to the kernel!\n", 0x17 );

    // test 'string_int64_to_temp'
    vga_text_print( "testing string_int64_to_temp for -1054: ", 0x17 );
    vga_text_print( string_int64_to_temp( -1054 ), 0x17 );
    vga_text_print( "\n", 0x17 );

    // access the very last byte of memory that is mapped by the page table
    char *p = (char*)0x200000 - 1;
    vga_text_print( "reading memory that is not mapped by page table: ", 0x17 );
    vga_text_print( string_int64_to_temp( (int64_t)*p ), 0x17 );
    vga_text_print( "\n", 0x17 );

    // setup a much larger page table
    // TODO

    // initialize the kernel heap
    // TODO: we must first expand out paging to address a larger amount of memory
    // kernel_heap_init();

    // TODO: do kernel main stuff
    // divide_by_zero();
    // panic( "attempted to divide by 0\n" );

    // machine is now ready for power off
    vga_text_print( "Exiting kernel & suspending CPU. Machine is now ready to be powered off.\n", 0x06 );
}

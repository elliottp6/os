#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "text/string.h"
#include "text/vga_text.h"
#include "memory/paging.h"
#include "memory/kernel_heap.h"
#include "interrupt/interrupt_table.h"

void panic( const char* details ) {
    // print messages
    vga_text_print( "System panic!\n", 0x4F );
    if( NULL != details ) vga_text_print( details, 0x4F );

    // suspend CPU
    while( true ) { asm ( "cli\n" "hlt\n" ); }
}

void main() {
    // clear background to blue, and display welcome message
    vga_text_clear( 0x17 );
    vga_text_print( "Welcome to the 64-bit kernel!\n", 0x17 );

    // initialize the interrupt table
    interrupt_table_init();

    // run string tests
    string_run_tests();

    // initialize the kernel heap (this also runs heap tests)
    kernel_heap_init();

    // machine is now ready for power off
    vga_text_print( "Exiting kernel & suspending CPU. Machine is now ready to be powered off.\n", 0x06 );
}

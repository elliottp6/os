#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "buffer/string.h"
#include "drivers/vga_text.h"
#include "memory/paging.h"
#include "memory/kernel_heap.h"
#include "interrupt/interrupt_table.h"
#include "drivers/ps2_keyboard.h"

static void suspend() {
    while( true ) { asm ( "cli\n" "hlt\n" ); }
}

void panic( const char* details ) {
    // print messages
    if( NULL != details ) {
        vga_text_print( "System panic: ", 0x4F );
        vga_text_print( details, 0x4F );
    } else {
        vga_text_print( "System panic!\n", 0x4F );
    }

    // suspend CPU
    suspend();
}

void main() {
    // clear background to blue, and display welcome message
    vga_text_clear( 0x17 );
    vga_text_print( "Welcome to the 64-bit kernel!\n", 0x17 );

    // run string tests
    string_run_tests();

    // initialize the kernel heap (this also runs heap tests)
    kernel_heap_init();

    // initialize the interrupt table
    interrupt_table_init();

    // now that we have interrupts & IRQs working, we can enable the keyboard driver
    ps2_keyboard_init();

    // main loop
    while( true ) {
        interrupt_table_wait_for_interrupt();
    }

    // machine is now ready for power off
    vga_text_print( "Exiting kernel & suspending CPU. Machine is now ready to be powered off.\n", 0x06 );
}

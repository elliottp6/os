#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "text/vga_text.h"

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

    // TODO: do kernel main stuff
    // divide_by_zero();
    // panic( "attempted to divide by 0\n" );

    // machine is now ready for power off
    vga_text_print( "Exiting kernel & suspending CPU. Machine is now ready to be powered off.\n", 0x06 );
}

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "text/vga_text.h"

void main() {
    // clear background to blue, and display welcome message
    vga_text_clear( 0x17 );
    vga_text_print( "Welcome to the kernel!\n", 0x17 );

    // TODO: do kernel main stuff
    // panic( "attempted to divide by 0\n" );

    // machine is now ready for power off
    vga_text_print( "Exiting kernel & suspending CPU. Machine is now ready to be powered off.\n", 0x06 );
}

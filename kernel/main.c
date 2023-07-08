#include <stddef.h>
#include <stdint.h>
#include "main.h"
#include "text/vga_text.h"

void main() {
    vga_text_clear( 0x17 );
    vga_text_print( "We are now in kernel main!\nIs this on the next line now?\n", 0x17 );
}

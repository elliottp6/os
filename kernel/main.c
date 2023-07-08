#include <stddef.h>
#include <stdint.h>
#include "main.h"
#include "terminal.h"

void main() {
    terminal_clear( 0x17 );
    terminal_print( "We are now in kernel main!\nIs this on the next line now?\n", 0x17 );
}

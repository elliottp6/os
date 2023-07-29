#include "io.h"

uint8_t io_read_byte( uint16_t port ) { // see https://www.osdev.org/howtos/2/
   uint8_t ret;
   asm( "inb %%dx, %%al": "=a" (ret): "d" (port) );
   return ret;
}

void io_write_byte( uint16_t port, uint8_t value ) {
    asm( "outb %%al, %%dx" :: "d" (port), "a" (value) );
}

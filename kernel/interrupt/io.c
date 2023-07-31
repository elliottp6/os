#include "io.h"

#define PIC1_COMMAND_PORT 0x20
#define PIC2_COMMAND_PORT 0xA0
#define PIC1_DATA_PORT (PIC1_COMMAND_PORT + 1)
#define PIC2_DATA_PORT (PIC2_COMMAND_PORT + 1)
#define PIC_COMMAND_END_OF_INTERRUPT 0x20

void io_disable_irqs() {
    io_write_byte( PIC1_DATA_PORT, 0xFF );
    io_write_byte( PIC2_DATA_PORT, 0xFF );
}

void io_enable_irqs() {
    io_write_byte( PIC1_DATA_PORT, 0 );
    io_write_byte( PIC2_DATA_PORT, 0 );
}

void io_acknowledge_irq() {
    io_write_byte( PIC1_COMMAND_PORT, PIC_COMMAND_END_OF_INTERRUPT );
    io_write_byte( PIC2_COMMAND_PORT, PIC_COMMAND_END_OF_INTERRUPT );
}

uint8_t io_read_byte( uint16_t port ) { // see https://www.osdev.org/howtos/2/
   uint8_t ret;
   asm( "inb %%dx, %%al": "=a" (ret): "d" (port) );
   return ret;
}

void io_write_byte( uint16_t port, uint8_t value ) {
    asm( "outb %%al, %%dx" :: "d" (port), "a" (value) );
}

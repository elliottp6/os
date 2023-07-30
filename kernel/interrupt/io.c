#include "io.h"

#define IRQ_0_7_COMMAND_PORT 0x20
#define IRQ_0_7_DISABLE_PORT 0x21
#define IRQ_8_15_COMMAND_PORT 0xA0
#define IRQ_8_15_DISABLE_PORT 0xA1

void io_disable_irqs() {
    io_write_byte( IRQ_0_7_DISABLE_PORT, 0xFF );
    io_write_byte( IRQ_8_15_DISABLE_PORT, 0xFF );
}

void io_enable_irqs() {
    io_write_byte( IRQ_0_7_DISABLE_PORT, 0 );
    io_write_byte( IRQ_8_15_DISABLE_PORT, 0 );
}

void io_acknowledge_irq() {
    io_write_byte( IRQ_0_7_COMMAND_PORT, 0x20 ); // we only need to send to the 0-7 port, b/c they're daisy-chained together
}

uint8_t io_read_byte( uint16_t port ) { // see https://www.osdev.org/howtos/2/
   uint8_t ret;
   asm( "inb %%dx, %%al": "=a" (ret): "d" (port) );
   return ret;
}

void io_write_byte( uint16_t port, uint8_t value ) {
    asm( "outb %%al, %%dx" :: "d" (port), "a" (value) );
}

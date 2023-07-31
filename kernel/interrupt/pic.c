#include "io.h"
#include "pic.h"

#define PIC1_COMMAND_PORT 0x20
#define PIC2_COMMAND_PORT 0xA0
#define PIC1_DATA_PORT (PIC1_COMMAND_PORT + 1)
#define PIC2_DATA_PORT (PIC2_COMMAND_PORT + 1)
#define PIC_COMMAND_END_OF_INTERRUPT 0x20

// must remap PIC because some IRQs would otherwise map to interrupts reserved for CPU exceptions
void pic_remap_and_enable_irqs() {
    io_write_byte( PIC1_COMMAND_PORT, 0x11 ); // ICW1: tells PIC to expect 3 commands
    io_write_byte( PIC2_COMMAND_PORT, 0x11 );
    io_write_byte( PIC1_DATA_PORT, 0x20 ); // ICW2: set base interrupt vector
    io_write_byte( PIC2_DATA_PORT, 0x28 );
    io_write_byte( PIC1_DATA_PORT, 0x04 ); // ICW3: IRQ2 -> connection to PIC2
    io_write_byte( PIC2_DATA_PORT, 0x02 ); 
    io_write_byte( PIC1_DATA_PORT, 0x01 ); // ICW4
    io_write_byte( PIC2_DATA_PORT, 0x01 );
    pic_enable_irqs();
}

void pic_enable_irqs() {
    io_write_byte( PIC1_DATA_PORT, 0 );
    io_write_byte( PIC2_DATA_PORT, 0 );
}

void pic_disable_irqs() {
    io_write_byte( PIC1_DATA_PORT, 0xFF );
    io_write_byte( PIC2_DATA_PORT, 0xFF );
}

void pic_acknowledge_irq() {
    io_write_byte( PIC1_COMMAND_PORT, PIC_COMMAND_END_OF_INTERRUPT );
    io_write_byte( PIC2_COMMAND_PORT, PIC_COMMAND_END_OF_INTERRUPT );
}

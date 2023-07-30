#pragma once

#include <stdint.h>

void io_disable_irqs();
void io_enable_irqs();
void io_acknowledge_irq();

uint8_t io_read_byte( uint16_t port );
void io_write_byte( uint16_t port, uint8_t value );

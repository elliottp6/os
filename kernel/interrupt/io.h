#pragma once

#include <stdint.h>

uint8_t io_read_byte( uint16_t port );
void io_write_byte( uint16_t port, uint8_t value );

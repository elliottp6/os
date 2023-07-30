#pragma once

#include <stddef.h>

void interrupt_table_init();
void interrupt_table_set_callback( size_t i, void(*callback)() );
void interrupt_table_handler( uint64_t interrupt );

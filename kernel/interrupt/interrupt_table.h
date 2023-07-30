#pragma once

#include <stddef.h>
#include <stdint.h>

// C interrupt handlers must be declared here, so our assembly code handlers can invoke them
#define INTERRUPT_TABLE_LENGTH 256
typedef void *(interrupt_handler)(uint64_t);
extern interrupt_handler *c_interrupt_handlers[INTERRUPT_TABLE_LENGTH];

// interrupt handler API
void interrupt_table_init();
void interrupt_table_set_handler( size_t i, interrupt_handler *handler );

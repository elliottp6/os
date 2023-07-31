#pragma once

#include <stddef.h>
#include <stdint.h>

// common interrupts
#define INTERRUPT_INDEX_DIVIDE_BY_ZERO 0
#define INTERRUPT_INDEX_BREAKPOINT 3
#define INTERRUPT_INDEX_INVALID_OPCODE 6
#define INTERRUPT_INDEX_CLOCK 8
#define INTERRUPT_INDEX_KEYBOARD 9

// C interrupt handlers must be declared here, so our assembly code handlers can invoke them
#define INTERRUPT_TABLE_LENGTH 256
typedef void *(interrupt_handler)(uint64_t);
extern interrupt_handler *c_interrupt_handlers[INTERRUPT_TABLE_LENGTH];

// interrupt handler API
void interrupt_table_init();
void interrupt_table_set_handler( size_t i, interrupt_handler *handler );
void interrupt_table_wait_for_interrupt();

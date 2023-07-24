#include <stdint.h>
#include "../memory/buffer.h"
#include "interrupt_table.h"

#define NUM_INTERRUPT_TABLE_ENTRIES 256 // x86_64 always has 256 of these

typedef struct interrupt_table_descriptor {
    uint16_t interrupt_table_size_minus_1;
    uint64_t interrupt_table_location;
} __attribute__((packed)) interrupt_table_descriptor_t;

typedef struct interrupt_table_entry { // aka interrupt descriptor. struct fields definitions from https://wiki.osdev.org/Interrupt_Descriptor_Table
    uint16_t offset_bits_0_15; // offset is address of ISR or interrupt service routine
    uint16_t code_segment_selector; // for GDT/LDT
    uint8_t stack_table_offset; // 3 bits
    uint8_t type_attribute_flags;
    uint16_t offset_bits_16_31;
    uint32_t offset_bits_32_63;
    uint32_t reserved_leave_as_0;
} interrupt_table_entry_t;

typedef struct interrupt_table {
    interrupt_table_entry_t entries[NUM_INTERRUPT_TABLE_ENTRIES];
} interrupt_table_t;

// define the table & the table descriptor
interrupt_table_descriptor_t interrupt_table_descriptor;
interrupt_table_t interrupt_table; // aka IDT (interrupt descriptor table)

// TODO: check if this is correct
typedef struct interrupt_frame {
    uint64_t rdi; // registers saved by CPU
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t r8;
    uint64_t r9;
    uint64_t rax;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t interrupt_number; // interrupt number and error code (if applicable)
    uint64_t error_code;
    uint64_t rip; // saved instruction pointer and code segment
    uint64_t cs;
    uint64_t rflags; // flags register
    uint64_t rsp; // stack pointer
    uint64_t ss; // stack segment
} interrupt_frame_t;

static void load_interrupt_table( interrupt_table_descriptor_t *descriptor ) {
    asm (
        "lidt %[descriptor]\n"
        :
        : [descriptor] "m" (*descriptor)
    );
}

void interrupt_table_init() {
    // initialize interrupt_table_descriptor
    interrupt_table_descriptor.interrupt_table_size_minus_1 = sizeof( interrupt_table_t ) - 1;
    interrupt_table_descriptor.interrupt_table_location = (uint64_t)&interrupt_table;

    // clear interrupt table (we now it's evenly divisible by sizeof( uint64_t ), so we can clear the buffer in 64-bit chunks)
    buffer_clear_qwords( (uint64_t*)&interrupt_table, sizeof( interrupt_table_t ) / sizeof( uint64_t ) );

    // TODO

    // load it
    // TODO: load_interrupt_table( &interrupt_table_descriptor );
}

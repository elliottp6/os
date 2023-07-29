#include <stdbool.h>
#include <stdint.h>
#include "interrupt_table.h"
#include "../memory/buffer.h"
#include "../text/vga_text.h" // for printing
#include "../main.h" // for panic

#define NUM_INTERRUPT_TABLE_ENTRIES 256 // x86_64 always has 256 of these
#define KERNEL_CODE_SELECTOR 0x08 // defined in boot.asm

typedef struct interrupt_table_descriptor {
    uint16_t interrupt_table_size_minus_1;
    uint64_t interrupt_table_location;
} __attribute__((packed)) interrupt_table_descriptor_t;

typedef struct interrupt_table_entry { // aka interrupt descriptor. struct fields definitions from https://wiki.osdev.org/Interrupt_Descriptor_Table
    uint16_t isr_address_bit0_15; // address of interrupt service routine
    uint16_t code_segment_selector; // for GDT/LDT
    uint8_t stack_table_offset; // 3 bits
    uint8_t type_attribute_flags;
    uint16_t isr_address_bit16_31;
    uint32_t isr_address_bit32_63;
    uint32_t reserved_leave_as_0;
} interrupt_table_entry_t;

typedef struct interrupt_table {
    interrupt_table_entry_t entries[NUM_INTERRUPT_TABLE_ENTRIES];
} interrupt_table_t;

// define the table & the table descriptor
interrupt_table_descriptor_t interrupt_table_descriptor;
interrupt_table_t interrupt_table; // aka IDT (interrupt descriptor table)

static void load_interrupt_table( interrupt_table_descriptor_t *descriptor ) {
    asm(
        "lidt %[descriptor]\n"
        :
        : [descriptor] "m" (*descriptor)
    );
}

static bool are_interrupts_enabled() {
    uint64_t flags;
    asm( "pushfq; popq %0" : "=r" (flags) );
    return flags & (1 << 9);
}

static void enable_interrupts() {
    asm( "sti\n" );
}

static void disable_interrupts() {
    asm( "cli\n" );
}

void interrupt_table_set( size_t i, void *interrupt_handler_wrapper ) {
    // disable interrupts
    bool interrupts_enabled = are_interrupts_enabled();
    disable_interrupts();

    // get function addrss 
    uint64_t handler_address = (uint64_t)interrupt_handler_wrapper;
    interrupt_table_entry_t *entry = &interrupt_table.entries[i];    

    // set isr address
    entry->isr_address_bit0_15 = (uint16_t)handler_address;
    entry->isr_address_bit16_31 = (uint16_t)(handler_address >> 16);
    entry->isr_address_bit32_63 = (uint32_t)(handler_address >> 32);

    // isr executes using the kernel code segment selector
    entry->code_segment_selector = KERNEL_CODE_SELECTOR;

    // bits: Present = 1, DPL (descriptor privilege level) = 11 (ring 3), storage segment = 0 (must be 0 for interrupt & trap gates), gate type = 11 (3 = interrupt gate), reserved = 0
    entry->type_attribute_flags = 0xEE;

    // enable interrupts (if they were enabled before calling this function)
    if( interrupts_enabled ) enable_interrupts();
}

static uint8_t inb( uint16_t port ) { // taken from https://www.osdev.org/howtos/2/ (note used yet, but might as well define this here for future reference since we've defined outb)
   uint8_t ret;
   asm( "inb %%dx, %%al": "=a" (ret): "d" (port) );
   return ret;
}

static void outb( uint16_t port, uint8_t value ) { // taken from https://www.osdev.org/howtos/2/
    asm( "outb %%al, %%dx" :: "d" (port), "a" (value) );
}

static void handle_divide_by_zero() {
    panic( "divided by zero\n" );
}

INTERRUPT_TABLE_BUILD_WRAPPER( handle_divide_by_zero );

static void cause_divide_by_zero() {
    asm( "\
        xor %rax, %rax  \n\t\
        xor %rdx, %rdx  \n\t\
        div %rdx        \n\t\
    ");
}

// globals modified by interrupt services routines (ISRs) should be volatile, see http://www.gammon.com.au/interrupts
volatile static bool breakpoint_was_handled;

static void handle_breakpoint() {
    breakpoint_was_handled = true;
}

INTERRUPT_TABLE_BUILD_WRAPPER( handle_breakpoint );

static void cause_breakpoint() {
    asm( "int $3" );
}

static void handle_invalid_opcode() {
    panic( "invalid opcode\n" );
}

INTERRUPT_TABLE_BUILD_WRAPPER( handle_invalid_opcode );

static void cause_invalid_opcode() {
    asm( "ud2" );
}

void interrupt_table_init() {
    // interrupts should be disabled prior to calling this function
    if( are_interrupts_enabled() ) panic( "interrupt_table_init: interrupts must be disabled before calling this function\n" );

    // initialize interrupt_table_descriptor
    interrupt_table_descriptor.interrupt_table_size_minus_1 = sizeof( interrupt_table_t ) - 1;
    interrupt_table_descriptor.interrupt_table_location = (uint64_t)&interrupt_table;

    // clear interrupt table (we now it's evenly divisible by sizeof( uint64_t ), so we can clear the buffer in 64-bit chunks)
    buffer_clear_qwords( (uint64_t*)&interrupt_table, sizeof( interrupt_table_t ) / sizeof( uint64_t ) );

    // load the interrupt table
    load_interrupt_table( &interrupt_table_descriptor );

    // wire up a few simple interrupts
    interrupt_table_set( 0, handle_divide_by_zero_wrapper );
    interrupt_table_set( 3, handle_breakpoint_wrapper );
    interrupt_table_set( 6, handle_invalid_opcode_wrapper );

    // enable interrupts
    enable_interrupts();

    // check that interrupts are now enabled
    if( !are_interrupts_enabled() ) panic( "interrupt_table_init: failed to enable interrupts\n" );

    // test the breakpoint interrupt
    // note that a software interrupt will stop execution @ 'cause_breakpoint' to run the handler
    breakpoint_was_handled = false;
    cause_breakpoint();

    // verify that the handle_breakpoint function was called
    // b/c this process was halted, we do not need locks (there's no concurrent access). 'volatile' is sufficient to ensure we're access the variable's value directly from memory, and not cached in a register.
    if( !breakpoint_was_handled ) panic( "interrupt_table_init: breakpoint interrupt test failed\n" );
}

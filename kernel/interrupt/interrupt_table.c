#include <stdbool.h>
#include <stdint.h>
#include "io.h"
#include "interrupt_table.h"
#include "../memory/buffer.h"
#include "../text/string.h" // for printing integers
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

// assembly interrupt routines
extern void* interrupt_service_routine_pointer_table[NUM_INTERRUPT_TABLE_ENTRIES];

static void load_interrupt_table( interrupt_table_descriptor_t *descriptor ) {
    asm( "lidt %[descriptor]" :: [descriptor] "m" (*descriptor) );
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

static void interrupt_table_set( size_t i, void *interrupt_routine ) {
    // get entry pointer
    interrupt_table_entry_t *entry = &interrupt_table.entries[i]; 

    // isr executes using the kernel code segment selector
    entry->code_segment_selector = KERNEL_CODE_SELECTOR;

    // set ISR (interrupt service routine) address
    uint64_t address = (uint64_t)interrupt_routine;
    entry->isr_address_bit0_15 = (uint16_t)address;
    entry->isr_address_bit16_31 = (uint16_t)(address >> 16);
    entry->isr_address_bit32_63 = (uint32_t)(address >> 32);

    // set these both to 0
    entry->reserved_leave_as_0 = 0;
    entry->stack_table_offset = 0;

    // bits: Present = 1, DPL (descriptor privilege level) = 11 (ring 3), storage segment = 0 (must be 0 for interrupt & trap gates), gate type = 11 (3 = interrupt gate), reserved = 0
    entry->type_attribute_flags = 0xEE;
}

static void cause_divide_by_zero() {
    asm( "\
        xor %rax, %rax  \n\t\
        xor %rdx, %rdx  \n\t\
        div %rdx        \n\t\
    ");
}

static void cause_breakpoint() {
    asm( "int $3" );
}

static void cause_invalid_opcode() {
    asm( "ud2" );
}

// TODO: consider adding a "stack frame" argument
void interrupt_handler( uint64_t interrupt ) {
    vga_text_print( "interrupt #", 0x17 );
    vga_text_print( string_from_int64( (int64_t)interrupt ), 0x17 );
    vga_text_print( "\n", 0x17 );
    panic( "there was an interrupt!\n" );
    //breakpoint_was_handled = true;
    //acknowledge_irq();
}

void interrupt_table_init() {
    // interrupts should be disabled prior to calling this function
    if( are_interrupts_enabled() ) panic( "interrupt_table_init: interrupts must be disabled before calling this function\n" );

    // initialize interrupt_table_descriptor
    interrupt_table_descriptor.interrupt_table_size_minus_1 = sizeof( interrupt_table_t ) - 1;
    interrupt_table_descriptor.interrupt_table_location = (uint64_t)&interrupt_table;

    // set all of the interrupts to the routines we wrote in interrupt_routines.asm
    for( int i = 0; i < NUM_INTERRUPT_TABLE_ENTRIES; i++ ) {
        interrupt_table_set( i, interrupt_service_routine_pointer_table[i] );
    }

    // load the interrupt table
    load_interrupt_table( &interrupt_table_descriptor );

    // enable interrupts
    enable_interrupts();
    if( !are_interrupts_enabled() ) panic( "interrupt_table_init: failed to enable interrupts\n" );

    // test an interrupt
    //cause_divide_by_zero();

    // test the breakpoint interrupt
    // note that a software interrupt will stop execution @ 'cause_breakpoint' to run the handler
    //breakpoint_was_handled = false;
    //cause_breakpoint();

    // verify that the breakpoint_handler function was called
    // b/c this process was halted, we do not need locks (there's no concurrent access). 'volatile' is sufficient to ensure we're access the variable's value directly from memory, and not cached in a register.
    //if( !breakpoint_was_handled ) panic( "interrupt_table_init: breakpoint interrupt test failed\n" );

    // now enable IRQs so we can recieve hardware interrupts (e.g. keypresses)
    // io_enable_irqs();
}

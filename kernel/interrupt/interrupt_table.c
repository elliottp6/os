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
#define TRACE_INTERRUPTS

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

// define table, table descriptor, interrupt routines & callbacks
static interrupt_table_descriptor_t interrupt_table_descriptor;
static interrupt_table_entry_t interrupt_table[NUM_INTERRUPT_TABLE_ENTRIES]; // aka IDT (interrupt descriptor table)
extern void* interrupt_service_routine_pointer_table[NUM_INTERRUPT_TABLE_ENTRIES]; // written in assembly
static void(*interrupt_callbacks[NUM_INTERRUPT_TABLE_ENTRIES])(); // calls backs written in C

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

void interrupt_table_set_callback( size_t i, void(*callback)() ) {
    if( i > NUM_INTERRUPT_TABLE_ENTRIES ) panic( "interrupt_table_set_callback: invalid interrupt index\n" );
    interrupt_callbacks[i] = callback;
}

static void interrupt_table_set_routine( size_t i, void *interrupt_routine ) {
    // get entry pointer
    interrupt_table_entry_t *entry = &interrupt_table[i]; 

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

// TODO: add a stack frame argument
void interrupt_table_handler( uint64_t interrupt ) {
    // print interrupt number
    #ifdef TRACE_INTERRUPTS
    vga_text_print( "interrupt ", 0x17 );
    vga_text_print( string_from_int64( (int64_t)interrupt ), 0x17 );
    vga_text_print( "\n", 0x17 );
    #endif
    
    // execute callback function
    void (*callback)() = interrupt_callbacks[interrupt];
    if( callback ) {
        callback();
    }

    // probably not all interrupts require this, but it doesn't hurt
    io_acknowledge_irq();
}

static void divide_by_zero_callback() {
    panic( "divided by zero\n" );
}

static void cause_divide_by_zero() {
    asm( "\
        xor %rax, %rax  \n\t\
        xor %rdx, %rdx  \n\t\
        div %rdx        \n\t\
    ");
}

static void invalid_opcode_callback() {
    panic( "invalid opcode\n" );
}

static void cause_invalid_opcode() {
    asm( "ud2" );
}

static volatile bool breakpoint_hit;

static void breakpoint_callback_test() {
    breakpoint_hit = true;
}

static void breakpoint_callback() {
    panic( "breakpoint\n" );
}

static void cause_breakpoint() {
    asm( "int $3" );
}

void interrupt_table_init() {
    // interrupts should be disabled prior to calling this function
    // TODO: should also check that maskable IRQs are disabled?
    if( are_interrupts_enabled() ) panic( "interrupt_table_init: interrupts must be disabled before calling this function\n" );

    // initialize interrupt table descriptor
    interrupt_table_descriptor.interrupt_table_size_minus_1 = sizeof( interrupt_table ) - 1;
    interrupt_table_descriptor.interrupt_table_location = (uint64_t)&interrupt_table;

    // clear all of the interrupt callbacks (they're each a qword, so this should work)
    buffer_clear_qwords( (uint64_t*)interrupt_callbacks, NUM_INTERRUPT_TABLE_ENTRIES );

    // set all of the interrupts to the routines we wrote in interrupt_routines.asm
    for( int i = 0; i < NUM_INTERRUPT_TABLE_ENTRIES; i++ ) {
        interrupt_table_set_routine( i, interrupt_service_routine_pointer_table[i] );
    }

    // set a few default callbacks to system panic
    interrupt_table_set_callback( 0, divide_by_zero_callback );
    interrupt_table_set_callback( 3, breakpoint_callback_test );
    interrupt_table_set_callback( 6, invalid_opcode_callback );

    // load the interrupt table
    load_interrupt_table( &interrupt_table_descriptor );

    // enable interrupts & IRQs
    enable_interrupts();
    io_enable_irqs();
    if( !are_interrupts_enabled() ) panic( "interrupt_table_init: failed to enable interrupts\n" );

    // test breakpoint interrupt
    breakpoint_hit = false;
    cause_breakpoint();
    if( !breakpoint_hit ) panic( "interrupt_table_init: test failed for breakpoint interrupt" );

    // replace the test breakpoint callback w/ the real callback (the real one just panics)
    interrupt_table_set_callback( 3, breakpoint_callback );
}

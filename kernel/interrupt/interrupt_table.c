#include <stdbool.h>
#include <stdint.h>
#include "pic.h"
#include "interrupt_table.h"
#include "../buffer/buffer.h"
#include "../buffer/string.h" // for printing integers
#include "../drivers/vga_text.h" // for printing
#include "../main.h" // for panic

#define KERNEL_CODE_SELECTOR 0x08 // defined in boot.asm
#define TRACE_UNHANDLED_INTERRUPTS

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

// define table descriptor, table, and interrupt wrappers (written in ASM) which call interrupt handlers (written in C)
static interrupt_table_descriptor_t interrupt_table_descriptor;
static interrupt_table_entry_t interrupt_table[INTERRUPT_TABLE_LENGTH]; // aka IDT (interrupt descriptor table)
extern void* interrupt_wrappers[INTERRUPT_TABLE_LENGTH]; // actual interrutp service routines written in assembly, which just call their corresponding C interrupt handler
interrupt_handler *interrupt_handlers[INTERRUPT_TABLE_LENGTH]; // handlers written in C

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

static void interrupt_table_set_wrapper( size_t i, void *interrupt_wrapper ) {
    // get entry pointer
    interrupt_table_entry_t *entry = &interrupt_table[i]; 

    // isr executes using the kernel code segment selector
    entry->code_segment_selector = KERNEL_CODE_SELECTOR;

    // set ISR (interrupt service routine) address
    uint64_t address = (uint64_t)interrupt_wrapper;
    entry->isr_address_bit0_15 = (uint16_t)address;
    entry->isr_address_bit16_31 = (uint16_t)(address >> 16);
    entry->isr_address_bit32_63 = (uint32_t)(address >> 32);

    // set these both to 0
    entry->reserved_leave_as_0 = 0;
    entry->stack_table_offset = 0;

    // bits: Present = 1, DPL (descriptor privilege level) = 11 (ring 3), storage segment = 0 (must be 0 for interrupt & trap gates), gate type = 11 (3 = interrupt gate), reserved = 0
    entry->type_attribute_flags = 0xEE;
}

void interrupt_table_set_handler( size_t i, interrupt_handler *handler ) {
    if( i >= INTERRUPT_TABLE_LENGTH ) panic( "interrupt_table_set_handler: invalid interrupt index\n" );
    interrupt_handlers[i] = handler;
}

// TODO: add a stack frame argument
static void trace_interrupt_handler( uint64_t interrupt ) {
    // print interrupt number
    #ifdef TRACE_UNHANDLED_INTERRUPTS
    vga_text_print( "interrupt ", 0x17 );
    vga_text_print( string_from_int64( (int64_t)interrupt ), 0x17 );
    vga_text_print( "\n", 0x17 );
    #endif
}

static void empty_interrupt_handler( uint64_t interrupt ) {}

static void divide_by_zero_handler( uint64_t interrupt ) {
    panic( "divided by zero\n" );
}

static void cause_divide_by_zero() {
    asm( "\
        xor %rax, %rax  \n\t\
        xor %rdx, %rdx  \n\t\
        div %rdx        \n\t\
    ");
}

static void invalid_opcode_handler( uint64_t interrupt ) {
    panic( "invalid opcode\n" );
}

static void cause_invalid_opcode() {
    asm( "ud2" );
}

static volatile bool breakpoint_hit;

static void breakpoint_handler_test( uint64_t interrupt ) {
    breakpoint_hit = true;
}

static void breakpoint_handler( uint64_t interrupt ) {
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

    // set all of the interrupts to the routines we wrote in interrupt_routines.asm
    // also, set all of the handlers to the default handler
    for( int i = 0; i < INTERRUPT_TABLE_LENGTH; i++ ) {
        interrupt_table_set_wrapper( i, interrupt_wrappers[i] );
        interrupt_table_set_handler( i, (interrupt_handler*)trace_interrupt_handler );
    }

    // ok, now set some specific handlers
    interrupt_table_set_handler( INTERRUPT_INDEX_DIVIDE_BY_ZERO, (interrupt_handler*)divide_by_zero_handler );
    interrupt_table_set_handler( INTERRUPT_INDEX_BREAKPOINT, (interrupt_handler*)breakpoint_handler_test );
    interrupt_table_set_handler( INTERRUPT_INDEX_INVALID_OPCODE, (interrupt_handler*)invalid_opcode_handler );
    interrupt_table_set_handler( INTERRUPT_INDEX_CLOCK, (interrupt_handler*)empty_interrupt_handler );

    // load the interrupt table
    load_interrupt_table( &interrupt_table_descriptor );

    // enable interrupts & IRQs
    enable_interrupts();
    pic_remap_and_enable_irqs();
    if( !are_interrupts_enabled() ) panic( "interrupt_table_init: failed to enable interrupts\n" );

    // test breakpoint interrupt
    breakpoint_hit = false;
    cause_breakpoint();
    if( !breakpoint_hit ) panic( "interrupt_table_init: test failed for breakpoint interrupt" );

    // replace the test breakpoint handler w/ the real handler (the real one just panics)
    interrupt_table_set_handler( INTERRUPT_INDEX_BREAKPOINT, (interrupt_handler*)breakpoint_handler );
}

void interrupt_table_wait_for_interrupt() {
    asm( "hlt" );
}

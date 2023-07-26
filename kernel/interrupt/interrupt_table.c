#include <stdint.h>
#include "../memory/buffer.h"
#include "interrupt_table.h"
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

typedef struct interrupt_frame {
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
} interrupt_frame_t;

/*
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
*/

static void load_interrupt_table( interrupt_table_descriptor_t *descriptor ) {
    asm (
        "lidt %[descriptor]\n"
        :
        : [descriptor] "m" (*descriptor)
    );
}

static void interrupt_table_set( size_t i, void *interrupt_service_routine ) {
    // get function addrss 
    uint64_t isr_address = (uint64_t)interrupt_service_routine;
    interrupt_table_entry_t *entry = &interrupt_table.entries[i];    

    // set isr address
    entry->isr_address_bit0_15 = (uint16_t)isr_address;
    entry->isr_address_bit16_31 = (uint16_t)(isr_address >> 16);
    entry->isr_address_bit32_63 = (uint32_t)(isr_address >> 32);

    // isr executes using the kernel code segment selector
    entry->code_segment_selector = KERNEL_CODE_SELECTOR;

    // bits: Present = 1, DPL (descriptor privilege level) = 11 (ring 3), storage segment = 0 (must be 0 for interrupt & trap gates), gate type = 11 (3 = interrupt gate), reserved = 0
    entry->type_attribute_flags = 0xEE;
}

// saves registers prior to ISR body
#define INTERRUPT_SERVICE_ROUTINE_PROLOGUE asm("\
    pushq   %rax\n\t\
    movq    %es, %rax\n\t\
    pushq   %rax\n\t\
    movq    %ds, %rax\n\t\
    pushq   %rax\n\t\
    pushq   %rbx\n\t\
    pushq   %rcx\n\t\
    pushq   %rdx\n\t\
    pushq   %rbp\n\t\
    pushq   %rdi\n\t\
    pushq   %rsi\n\t\
    pushq   %r8\n\t\
    pushq   %r9\n\t\
    pushq   %r10\n\t\
    pushq   %r11\n\t\
    pushq   %r12\n\t\
    pushq   %r13\n\t\
    pushq   %r14\n\t\
    pushq   %r15\n\t\
    movq    $0x10, %rdi\n\t\
    movq    %rdi, %es\n\t\
    movq    %rdi, %ds\n\t\
");

// restored registers after ISR body and returns using IRETQ
#define INTERRUPT_SERVICE_ROUTINE_EPILOGUE asm("\
    popq    %r15\n\t\
    popq    %r14\n\t\
    popq    %r13\n\t\
    popq    %r12\n\t\
    popq    %r11\n\t\
    popq    %r10\n\t\
    popq    %r9\n\t\
    popq    %r8\n\t\
    popq    %rsi\n\t\
    popq    %rdi\n\t\
    popq    %rbp\n\t\
    popq    %rdx\n\t\
    popq    %rcx\n\t\
    popq    %rbx\n\t\
    popq    %rax\n\t\
    movq    %rax, %ds\n\t\
    popq    %rax\n\t\
    movq    %rax, %es\n\t\
    popq    %rax\n\t\
    leave\n\t\
    iretq\n\t\
");

__attribute__((naked)) static void naked_handle_divide_by_zero() {
    INTERRUPT_SERVICE_ROUTINE_PROLOGUE
    // TODO: call C function which does the real work here
    INTERRUPT_SERVICE_ROUTINE_EPILOGUE
}

// TODO: we could also try this approach, which would likely require us to specify different compilation flags for the ISR C file
//__attribute__((interrupt)) static void handle_divide_by_zero( interrupt_frame_t *frame ) {}

void interrupt_table_init() {
    // initialize interrupt_table_descriptor
    interrupt_table_descriptor.interrupt_table_size_minus_1 = sizeof( interrupt_table_t ) - 1;
    interrupt_table_descriptor.interrupt_table_location = (uint64_t)&interrupt_table;

    // clear interrupt table (we now it's evenly divisible by sizeof( uint64_t ), so we can clear the buffer in 64-bit chunks)
    buffer_clear_qwords( (uint64_t*)&interrupt_table, sizeof( interrupt_table_t ) / sizeof( uint64_t ) );

    // ok, let's define a handler for "divide by zero", so we can test our table


    // load it
    // TODO: load_interrupt_table( &interrupt_table_descriptor );
}

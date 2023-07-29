#pragma once

#include <stddef.h>

void interrupt_table_init();
void interrupt_table_set( size_t i, void *interrupt_handler_wrapper );

// TODO: we need to save the AVX registers as well (or, we can compile our kernel to ensure we never use them)
// which registers are guaranteed to be preserved in a gcc C function call? see https://stackoverflow.com/questions/18024672/what-registers-are-preserved-through-a-linux-x86-64-function-call
#define INTERRUPT_TABLE_BUILD_WRAPPER( c_function ) \
    __attribute__((naked)) static void c_function##_wrapper() { \
        asm("\
            pushq   %rax            \n\t\
            pushq   %rcx            \n\t\
            pushq   %rdx            \n\t\
            pushq   %rsi            \n\t\
            pushq   %rdi            \n\t\
            pushq   %r8             \n\t\
            pushq   %r9             \n\t\
            pushq   %r10            \n\t\
            pushq   %r11            \n\t\
            call    "#c_function"   \n\t\
            popq    %r11            \n\t\
            popq    %r10            \n\t\
            popq    %r9             \n\t\
            popq    %r8             \n\t\
            popq    %rdi            \n\t\
            popq    %rsi            \n\t\
            popq    %rdx            \n\t\
            popq    %rcx            \n\t\
            popq    %rax            \n\t\
            iretq                   \n\t\
        "); \
    }

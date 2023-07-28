#pragma once

#include <stddef.h>

void interrupt_table_init();
void interrupt_table_set( size_t i, void *interrupt_handler_wrapper );

// TODO: finish
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

// must put a single-line comment here or else for some reason the below multiline comment messes up the macro
/*
    ;pushq   %rax            \n\t\
            ;movq    %es, %rax       \n\t\
            ;pushq   %rax            \n\t\
            ;movq    %ds, %rax       \n\t\
            ;pushq   %rax            \n\t\
            ;pushq   %rbx            \n\t\
            ;pushq   %rcx            \n\t\
            ;pushq   %rdx            \n\t\
            ;pushq   %rbp            \n\t\
            ;pushq   %rdi            \n\t\
            ;pushq   %rsi            \n\t\
            ;pushq   %r8             \n\t\
            ;pushq   %r9             \n\t\
            ;pushq   %r10            \n\t\
            ;pushq   %r11            \n\t\
            ;pushq   %r12            \n\t\
            ;pushq   %r13            \n\t\
            ;pushq   %r14            \n\t\
            ;pushq   %r15            \n\t\
            ;movq    $0x10, %rdi     \n\t\
            ;movq    %rdi, %es       \n\t\
            ;movq    %rdi, %ds       \n\t\
            ;call    "#c_function"   \n\t\
            ;popq    %r15            \n\t\
            ;popq    %r14            \n\t\
            ;popq    %r13            \n\t\
            ;popq    %r12            \n\t\
            ;popq    %r11            \n\t\
            ;popq    %r10            \n\t\
            ;popq    %r9             \n\t\
            ;popq    %r8             \n\t\
            ;popq    %rsi            \n\t\
            ;popq    %rdi            \n\t\
            ;popq    %rbp            \n\t\
            ;popq    %rdx            \n\t\
            ;popq    %rcx            \n\t\
            ;popq    %rbx            \n\t\
            ;popq    %rax            \n\t\
            ;movq    %rax, %ds       \n\t\
            ;popq    %rax            \n\t\
            ;movq    %rax, %es       \n\t\
            ;popq    %rax            \n\t\
            ;leave                   \n\t\
*/

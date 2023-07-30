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


/*
// this is the frame that the CPU pushes to the stack before calling the interrupt
typedef struct interrupt_frame {
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
} interrupt_frame_t;
*/

/*
// this is a larger frame, after we push our extra regs to the stack
// TODO: this needs to sync up with what our ISR EPILOGUE/PROLOGUE actually does
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




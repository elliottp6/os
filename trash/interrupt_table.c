
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




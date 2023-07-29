#include <stdint.h>
#include "ps2_keyboard.h"

// PS/2 port controller, see https://wiki.osdev.org/%228042%22_PS/2_Controller
#define PS2_PORT 0x64 // PS/2 command register
#define KEYBOARD_INPUT_PORT 0x60 // PS/2 data input port
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE //
#define KEY_RELEASED_MASK 0x80 // bitmask for a single bit of the scancode
#define KEY_PRESS_OR_RELEASE_INTERRUPT 0x21
#define CAPSLOCK_SCANCODE 0x3A

// PS/2 keyboard actually have 3 sets of scancodes
// for now, we just implement the first set (set #1)
// see https://wiki.osdev.org/PS/2_Keyboard
/*static uint8_t scancode_to_ascii_1[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0d, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`', 
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'
};*/

void ps2_keyboard_init() {
    /*
    // register the interrupt callback for keypresses
    idt_register_interrupt_callback( ISR_KEYBOARD_INTERRUPT, classic_keyboard_handle_interrupt );
    
    // initial state of keyboard should be capslock disabled
    keyboard_set_capslock( &classic_keyboard, KEYBOARD_CAPS_LOCK_OFF );

    // enable the 1st PS/2 port
    io_write_byte( PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT );
    */
}

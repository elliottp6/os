#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../interrupt/io.h"
#include "../interrupt/interrupt_table.h"
#include "../text/vga_text.h" // for debug output
#include "../main.h" // for panic
#include "ps2_keyboard.h"

// PS/2 port controller, see https://wiki.osdev.org/%228042%22_PS/2_Controller
#define PS2_PORT 0x64 // PS/2 command register
#define KEYBOARD_INPUT_PORT 0x60 // PS/2 data input port
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE //
#define KEY_RELEASED_MASK 0x80 // bitmask for a single bit of the scancode
#define KEY_STATE_INTERRUPT 0x21
#define CAPSLOCK_SCANCODE 0x3A

// PS/2 keyboard actually have 3 sets of scancodes
// for now, we just implement the first set (set #1)
// see https://wiki.osdev.org/PS/2_Keyboard
/*static uint8_t scancode_to_ascii_table_1[] = {
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

static uint8_t scancode_to_char( uint8_t scancode, uint8_t *table, uint8_t table_length, bool capslock ) {
    // decode character
    uint8_t c = scancode < table_length ? table[scancode] : 0;

    // make lowercase if capslock is off
    return c + 32 * (!capslock & (c >= 'A') & (c <= 'Z'));
}

static void key_state_handler( uint64_t interrupt ) {
    //panic( "keypress\n" );
    vga_text_print( "k", 0x17 );

    // TODO: we don't get any more keypress/release interrupts... but that's probably b/c we have to do something else here!
}

void ps2_keyboard_init() {
    // set interrupt to handle keypress
    interrupt_table_set_handler( KEY_STATE_INTERRUPT, (interrupt_handler*)key_state_handler );

    // enable the 1st PS/2 port
    io_write_byte( PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT );
}

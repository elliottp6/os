#include <stdint.h>
#include "main.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

static uint16_t *vga_text_mode_buffer = (uint16_t*)0xB8000;

static uint16_t terminal_make_char( char c, char color ) { return (color << 8) | c; }

static void terminal_putchar( int x, int y, char c, char color ) {
    vga_text_mode_buffer[(y * VGA_WIDTH) + x] = terminal_make_char( c, color );
}

void main() {
    terminal_putchar( 0, 0, 'A', 15 );
}

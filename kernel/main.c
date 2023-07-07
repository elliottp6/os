#include <stddef.h>
#include <stdint.h>
#include "main.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

static size_t strlen( const char* p ) {
    size_t i = 0;
    while( p[i] ) i++;
    return i;
}

uint16_t *vga_text_mode_buffer = (uint16_t*)0xB8000,
          terminal_row = 0,
          terminal_col = 0;

static uint16_t terminal_make_char( char c, char color ) {
    return (color << 8) | c;
}

static void terminal_putchar( int x, int y, char c, char color ) {
    vga_text_mode_buffer[(y * VGA_WIDTH) + x] = (color << 8) | c ; // terminal_make_char( c, color );
}

static void terminal_backspace() {
    if( 0 == terminal_col ) {
        if( 0 == terminal_row ) return;
        terminal_row--;
        terminal_col = VGA_WIDTH;
    }
    terminal_putchar( --terminal_col, terminal_row, ' ', 15 );
}

static void terminal_writechar( char c, char color ) {
    // handle newline 
    if( '\n' == c ) { terminal_row++; terminal_col = 0; return; }

    // handle backspace
    if( 8 == c ) { terminal_backspace(); return; }

    // handle regular character
    terminal_putchar( terminal_col, terminal_row, c, color );
    terminal_col++;
    if( terminal_col >= VGA_WIDTH ) { terminal_col = 0; terminal_row++; }
}

static void print( char* str, size_t len ) {
    terminal_putchar( 0, 0, 'B', 15 );
    terminal_putchar( 1, 0, 'I', 15 );
    //size_t len = strlen( str );
    //for( int i = 0; i < len; i++ ) terminal_writechar( str[i], color );
}

// TODO: ok, it's clear that passing pointers is a huge problem
// it's not a stack size problem yet (though our stack is still only 256 bytes)
// the problem is likely where C stores the static data segment
// we might also have our kernel page tables in the wrong place
void main() {
    print( NULL, 0 );
}

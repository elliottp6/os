#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "string.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_SIZE (VGA_WIDTH * VGA_HEIGHT)

typedef struct vga_text_cell {
    char character, attribute; // ascii character, top 3 bits for background, bottom 4 bits for foreground
} vga_text_cell;

// globals
static vga_text_cell *vga_text = (vga_text_cell*)0xB8000;
static uint32_t terminal_x = 0, terminal_y = 0;

static vga_text_cell make_cell( char character, char attribute ) {
    vga_text_cell result;
    result.character = character;
    result.attribute = attribute;
    return result;
}

static void set_cell( int x, int y, vga_text_cell cell ) {
    vga_text[(y * VGA_WIDTH) + x] = cell;
}

static void backspace() {
    // if we're at the beginning of a line
    if( 0 == terminal_x ) {
        // and there's no prior line, then nothing to do
        if( 0 == terminal_y ) return;

        // otherwise, move Y position up to the prior line
        terminal_y--;

        // and reset X position to the end of the prior line
        terminal_x = VGA_WIDTH; // <-- TODO: not sure if this is right, or if we should look for first non-whitespace character?
    }

    // blank out the current position
    set_cell( --terminal_x, terminal_y, make_cell( ' ', 0xF ) );
}

static void write_cell( vga_text_cell cell ) {
    // handle newline 
    if( '\n' == cell.character ) { terminal_y++; terminal_x = 0; return; }

    // handle backspace
    if( 8 == cell.character ) { backspace(); return; }

    // handle regular character
    set_cell( terminal_x, terminal_y, cell );
    terminal_x++;
    if( terminal_x >= VGA_WIDTH ) { terminal_x = 0; terminal_y++; }
}

void vga_text_print( const char* str, char color ) {
    size_t len = strlen( str );
    for( int i = 0; i < len; i++ ) write_cell( make_cell( str[i], color ) );
}

void vga_text_clear( char color ) {
    //
    terminal_x = terminal_y = 0;

    // 
    vga_text_cell cell;
    cell.attribute = color;
    cell.character = ' ';
    for( int i = 0; i < VGA_SIZE; i++ ) vga_text[i] = cell;
}

#pragma once

void vga_text_print( const char* str, char color );
void vga_text_clear( char color );
void panic( const char* details ); // probably shouldn't go here, but meh we'll move it somewhere else later

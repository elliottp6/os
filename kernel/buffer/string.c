#include <stdbool.h>
#include "string.h"
#include "../main.h"

// temporary buffer just enough to hold a 64-bit integer (up to 19 digits), plus a sign and a null terminator
#define TEMP_LENGTH 21
static char temp[TEMP_LENGTH];

char *string_from_int64( int64_t n ) {
    // always treat integers as negative (b/c range of negative integers is 1 greater than positive integers)
    bool is_positive = n >= 0;
    if( is_positive ) n = -n;

    // write null terminator
    temp[TEMP_LENGTH - 1] = 0;

    // write digits right-to-left
    size_t i = TEMP_LENGTH - 1;
    do temp[--i] = '0' - (n % 10); while( n/=10 );

    // add negative sign
    if( !is_positive ) temp[--i] = '-';

    // return address @ leftmost position
    return &temp[i];
}

size_t string_length( const char* p ) {
    size_t i = 0;
    while( p[i] ) i++;
    return i;
}

int string_compare( const char* a, const char* b ) {
    for(; (*a != 0) | (*b != 0); a++, b++ ) {
        if( *a != *b ) return *a < *b;
    }
    return 0;
}

bool string_equal( const char* a, const char* b ) {
    return 0 == string_compare( a, b );
}

void string_run_tests() {
    char *result = string_from_int64( -1054 );
    if( !string_equal( result, "-1054" ) ) {
        panic( "string_run_tests failed: expected -1054" );
    }
}

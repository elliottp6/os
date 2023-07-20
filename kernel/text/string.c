#include <stdbool.h>
#include "string.h"

// temporary buffer just enough to hold a 64-bit integer (up to 19 digits), plus a sign and a null terminator
#define TEMP_LENGTH 21
static char temp[TEMP_LENGTH];

size_t string_length( const char* p ) {
    size_t i = 0;
    while( p[i] ) i++;
    return i;
}

char *string_int64_to_temp( int64_t n ) {
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

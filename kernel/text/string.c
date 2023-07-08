#include "string.h"

size_t strlen( const char* p ) {
    size_t i = 0;
    while( p[i] ) i++;
    return i;
}

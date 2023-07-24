#include "buffer.h"

void buffer_set_qwords( uint64_t *buffer, uint64_t value, size_t count ) {
    while( count > 0 ) {
        *buffer = value;
        buffer++;
        count--;
    }
}

void buffer_clear_qwords( uint64_t *buffer, size_t count ) { 
    buffer_set_qwords( buffer, 0, count );
}

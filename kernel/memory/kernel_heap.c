#include "kernel_heap.h"
#include "freelist_heap.h"

void kernel_heap_init() {
    freelist_heap_init( KERNEL_HEAP_START, KERNEL_HEAP_SIZE );
}

void *kernel_heap_alloc( size_t object_size ) {
    freelist_heap_alloc( KERNEL_HEAP_START, object_size );
}

void kernel_heap_free( void *object ) {
    freelist_heap_free( KERNEL_HEAP_START, object );
}

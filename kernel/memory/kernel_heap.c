#include "kernel_heap.h"
#include "freelist_heap.h"

void kernel_heap_init() {
    freelist_heap_init( (void*)KERNEL_HEAP_START, KERNEL_HEAP_SIZE );
}

void *kernel_heap_alloc( size_t object_size ) {
    return freelist_heap_alloc( (void*)KERNEL_HEAP_START, object_size );
}

void kernel_heap_free( void *object ) {
    freelist_heap_free( (void*)KERNEL_HEAP_START, object );
}

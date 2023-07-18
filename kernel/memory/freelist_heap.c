#include "freelist_heap.h"

// alignment must be a power of 2 (or else undefined behavior)
static size_t upalign_offset( void *address, size_t alignment_minus_1 ) {
    uintptr_t current = (size_t)address;
    uintptr_t next = (current + alignment_minus_1) & ~alignment_minus_1;
    return next - current;
}

void freelist_heap_init( freelist_heap *heap, void* start, size_t size ) {
    // set fields for heap
    heap->start = start;
    heap->size = size;

    // check if we can fit a root block in heap w/ data_size >= 1
    size_t start_offset = upalign_offset( start, sizeof( size_t ) - 1 );
    size_t data_offset = start_offset + sizeof( freelist_block );
    if( size <= data_offset ) {
        heap->root = NULL;
        return;
    }

    // initialize root block
    freelist_block *root = heap->root = (freelist_block*)(start + start_offset);
    circular_list_init( (circular_list_node_t*)root );
    root->data_size = size - data_offset; // this is always >= 1
}

static freelist_block *find_free_block( freelist_heap *heap ) {
    return NULL;
}

void* freelist_heap_allocate( freelist_heap* heap, size_t size ) {
    return NULL;
}

void freelist_heap_free( freelist_heap* heap, void* ptr ) {

}

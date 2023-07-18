#include "freelist_heap.h"

// alignment must be a power of 2 (or else undefined behavior)
static size_t offset_for_forward_align( void *address, size_t alignment_minus_1 ) {
    uintptr_t current = (size_t)address;
    uintptr_t next = (current + alignment_minus_1) & ~alignment_minus_1;
    return next - current;
}

void freelist_heap_init( freelist_heap *heap, void* start, size_t size ) {
    // set fields for heap
    heap->start = start;
    heap->size = size;

    // check if we can fit a root block in heap w/ data_size >= 1
    size_t start_offset = offset_for_forward_align( start, sizeof( size_t ) - 1 );
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

static bool block_is_big_enough( circular_list_node_t *block_node, void *data_size_ptr ) {
    freelist_block *block = (freelist_block*)block_node;
    size_t data_size = *(size_t*)data_size_ptr;
    return block->data_size >= data_size;
}

static freelist_block *find_big_enough_block( freelist_heap *heap, size_t data_size ) {
    if( NULL == heap->root ) return NULL;
    return (freelist_block*)circular_list_find( (circular_list_node_t*)heap->root, block_is_big_enough, &data_size );
}

void* freelist_heap_allocate( freelist_heap* heap, size_t size ) {
    // see if we can find a block that's big enough
    freelist_block *block = find_big_enough_block( heap, size );
    if( NULL == block ) return NULL;

    // see if we should split this block
    // TODO

    // return pointer to the block's data
    return block + sizeof( freelist_block );
}

void freelist_heap_free( freelist_heap* heap, void* ptr ) {
    // TODO
}

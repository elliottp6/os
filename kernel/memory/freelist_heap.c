#include "freelist_heap.h"

// a memory block should be at least big enough to hold the block header and a size_t x
#define MIN_BLOCK_SIZE (sizeof(freelist_block) + sizeof(size_t))

// alignment must be a power of 2 (or else undefined behavior)
static size_t offset_for_forward_align( size_t x, size_t alignment_minus_1 ) {
    size_t next = (x + alignment_minus_1) & ~alignment_minus_1;
    return next - x;
}

void freelist_heap_init( freelist_heap *heap, void* start, size_t size ) {
    // set fields for heap
    heap->start = start;
    heap->size = size;
    heap->root = NULL;

    // check if we have enough space for root block
    size_t offset = offset_for_forward_align( (size_t)start, sizeof( size_t ) - 1 );
    if( size < offset + MIN_BLOCK_SIZE ) return;

    // initialize root block
    freelist_block *root = heap->root = (freelist_block*)(start + offset);
    circular_list_init( (circular_list_node_t*)root );
    root->block_size = size - offset;
}

static bool block_is_big_enough( circular_list_node_t *block_node, void *min_block_size ) {
    freelist_block *block = (freelist_block*)block_node;
    return block->block_size >= *(size_t*)min_block_size;
}

static freelist_block *find_big_enough_block( freelist_heap *heap, size_t min_block_size ) {
    if( NULL == heap->root ) return NULL;
    return (freelist_block*)circular_list_find( (circular_list_node_t*)heap->root, block_is_big_enough, &min_block_size );
}

void* freelist_heap_allocate( freelist_heap* heap, size_t size ) {
    // ensure that 'min_block_size' includes the block header, and is aligned to the pointer size
    size_t min_block_size = offset_for_forward_align( size + sizeof( freelist_block ), sizeof( size_t ) - 1 );

    // see if we can find a block that's big enough
    freelist_block *block = find_big_enough_block( heap, min_block_size );
    if( NULL == block ) return NULL;

    // split block (if possible)
    size_t free_space_size = block->block_size - min_block_size;
    if( free_space_size >= MIN_BLOCK_SIZE ) {
        // shrink block to its minimum size
        block->block_size = min_block_size;

        // insert new block w/ the extra free space
        freelist_block *new_block = block + min_block_size;
        new_block->block_size = free_space_size;
        circular_list_insert_after( (circular_list_node_t*)heap->root, (circular_list_node_t*)new_block );
    }

    // remove block from the free list
    circular_list_remove( (circular_list_node_t*)block );

    // return pointer to the block's data
    return block + sizeof( freelist_block );
}

void freelist_heap_free( freelist_heap* heap, void* ptr ) {
    // TODO
}

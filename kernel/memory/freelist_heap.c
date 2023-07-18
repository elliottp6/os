#include "freelist_heap.h"

typedef freelist_used_block_t used_block_t;
typedef freelist_free_block_t free_block_t;
typedef freelist_heap_t heap_t;
typedef circular_list_node_t node_t;

#define MIN_FREE_BLOCK_SIZE (sizeof( free_block_t ) + sizeof( size_t ))

// alignment must be a power of 2 (or else undefined behavior)
static size_t offset_for_forward_align( size_t x, size_t alignment_minus_1 ) {
    size_t next = (x + alignment_minus_1) & ~alignment_minus_1;
    return next - x;
}

void freelist_heap_init( heap_t *heap, void *start, size_t size ) {
    // initialize heap
    heap->start = start;
    heap->size = size;
    heap->root.block_size = 0;
    circular_list_init( (node_t*)&heap->root );

    // check if we have enough space for root free block
    size_t offset = offset_for_forward_align( (size_t)start, sizeof( size_t ) - 1 );
    if( size < offset + MIN_FREE_BLOCK_SIZE ) return;

    // insert first free block
    free_block_t *free_block = (free_block_t*)(start + offset);
    circular_list_insert_after( (node_t*)&heap->root, (node_t*)free_block );
    free_block->block_size = size - offset;
}

static bool free_block_is_big_enough( node_t *free_block_node, void *min_free_block_size ) {
    free_block_t *free_block = (free_block_t*)free_block_node;
    return free_block->block_size >= *(size_t*)min_free_block_size;
}

static free_block_t *find_big_enough_free_block( heap_t *heap, size_t min_free_block_size ) {
    return (free_block_t*)circular_list_find( (node_t*)&heap->root, free_block_is_big_enough, &min_free_block_size );
}

void *freelist_heap_allocate( heap_t *heap, size_t size ) {
    // min_block_size is size w/ the used block header, aligned to the pointer size
    size_t min_block_size = offset_for_forward_align( size + sizeof( used_block_t ), sizeof( size_t ) - 1 );

    // see if we can find a free block that's big enough
    free_block_t *free_block = find_big_enough_free_block( heap, min_block_size );
    if( NULL == free_block ) return NULL;

    // if there's enough free space in the block: split it
    size_t free_space_size = free_block->block_size - min_block_size;
    if( free_space_size >= MIN_FREE_BLOCK_SIZE ) {
        // shrink block to its minimum size
        free_block->block_size = min_block_size;

        // initialize new free block right after this block
        free_block_t *new_free_block = free_block + min_block_size;
        new_free_block->block_size = free_space_size;

        // replace block with the new free block
        circular_list_replace( (node_t*)free_block, (node_t*)new_free_block );
    } else {
        // remove block from the free list
        circular_list_remove( (node_t*)free_block );
    }

    // convert free_block into used_block
    used_block_t* used_block = (used_block_t*)free_block;
    used_block->block_size = free_block->block_size;

    // return pointer to the block's data
    return used_block + sizeof( used_block_t );
}

void freelist_heap_free( heap_t* heap, void* ptr ) {
    // get block & block_size
    //used_block_t *used_block = (used_block_t*)(ptr - sizeof(used_block_t));
    //size_t block_size = used_block->block_size;

    // TODO: insert into freelist at correct memory location

    // TODO: defragement adjacent blocks
}

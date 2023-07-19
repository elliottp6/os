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

void *freelist_heap_allocate( heap_t *heap, size_t size ) {
    // min_block_size is size w/ the used block header, aligned to the pointer size
    size_t min_block_size = offset_for_forward_align( size + sizeof( used_block_t ), sizeof( size_t ) - 1 );

    // see if we can find a free block that's big enough
    free_block_t *free_block = (free_block_t*)circular_list_find( (node_t*)&heap->root, free_block_is_big_enough, &min_block_size );
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
    return used_block + sizeof(used_block_t);
}

static void try_merge_left( free_block_t *left, free_block_t *right ) {
    // cannot merge with root block
    if( 0 == left->block_size || 0 == right->block_size ) return;

    // right must touch left to be mergeable
    if( right != left + left->block_size ) return;

    // merge right block into left
    left->block_size+= right->block_size;
    circular_list_remove( (node_t*)right );
}

void freelist_heap_free( heap_t *heap, void* ptr ) {
    // get used block & block_size
    used_block_t *used_block = (used_block_t*)(ptr - sizeof(used_block_t));
    size_t block_size = used_block->block_size;

    // turn this into a free block
    free_block_t *free_block = (free_block_t*)used_block;
    free_block->block_size = block_size;

    // insert into freelist at correct memory location
    // WARNING: this is an O(n) linear search across the heap (the most expensive operation we have in the freelist_heap)
    node_t *root = &heap->root.node;
    node_t *node = root->next;
    while( node != root && (uintptr_t)node < (uintptr_t)free_block ) node = node->next;
    circular_list_insert_before( node, (node_t*)free_block );

    // defragment heap by trying to merge this block with its right and left blocks
    try_merge_left( free_block, (free_block_t*)free_block->node.next );
    try_merge_left( (free_block_t*)free_block->node.prior, free_block );
}

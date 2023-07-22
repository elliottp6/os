#include <stdbool.h>
#include <stdint.h>
#include "circular_list.h"
#include "freelist_heap.h"
#include "../main.h"

typedef circular_list_node_t node_t;

typedef struct used_block {
    size_t block_size;
} used_block_t;

typedef struct free_block {
    node_t node;
    size_t block_size;
} free_block_t;

typedef struct heap {
    size_t size;
    free_block_t root;
} heap_t;

// align must be a power of 2 (or else undefined behavior)
static size_t offset_for_upalign( size_t value, size_t align ) {
    size_t next = (value + align - 1) & ~(align - 1);
    return next - value;
}

void freelist_heap_init( void *heap_start, size_t heap_size ) {
    // initialize heap
    heap_t *heap = (heap_t*)heap_start;
    heap->size = heap_size;
    heap->root.block_size = 0;
    circular_list_init( (node_t*)&heap->root );

    // check if we have enough space for root free block after the heap header (and, aligned to the pointer size)
    size_t offset = offset_for_upalign( (size_t)heap + sizeof( heap_t ), sizeof( size_t ) );
    if( heap_size < offset + sizeof( free_block_t ) ) return;

    // insert first free block
    free_block_t *free_block = (free_block_t*)(heap_start + offset);
    circular_list_insert_after( (node_t*)&heap->root, (node_t*)free_block );
    free_block->block_size = heap_size - offset;
}

static bool free_block_is_big_enough( node_t *free_block_node, void *min_free_block_size ) {
    free_block_t *free_block = (free_block_t*)free_block_node;
    return free_block->block_size >= *(size_t*)min_free_block_size;
}

void *freelist_heap_alloc( void *heap_start, size_t object_size ) {
    // object_size must be aligned to pointer size
    object_size+= offset_for_upalign( object_size, sizeof( size_t ) );

    // define minimum block size
    size_t min_block_size = sizeof( used_block_t ) + object_size;
    if( min_block_size < sizeof( free_block_t ) ) min_block_size = sizeof( free_block_t );

    // see if we can find a free block that's big enough
    heap_t *heap = (heap_t*)heap_start;
    free_block_t *free_block = (free_block_t*)circular_list_find( (node_t*)&heap->root, free_block_is_big_enough, &min_block_size );
    if( NULL == free_block ) {
        panic( "no free blocks in heap\n" );
        return NULL;
    }

    // if there's enough free space in the block: split it
    size_t free_space_size = free_block->block_size - min_block_size;
    if( free_space_size >= sizeof( free_block_t ) ) {
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

    // return pointer to the used block's data
    return used_block + sizeof( used_block_t );
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

void freelist_heap_free( void *heap_start, void* ptr ) {
    // get used block & block_size
    used_block_t *used_block = (used_block_t*)(ptr - sizeof(used_block_t));
    size_t block_size = used_block->block_size;

    // turn this into a free block
    free_block_t *free_block = (free_block_t*)used_block;
    free_block->block_size = block_size;

    // insert into freelist at correct memory location
    // WARNING: this is an O(n) linear search across the heap (the most expensive operation we have in the freelist_heap)
    heap_t *heap = (heap_t*)heap_start;
    node_t *root = &heap->root.node;
    node_t *node = root->next;
    while( node != root && (uintptr_t)node < (uintptr_t)free_block ) node = node->next;
    circular_list_insert_before( node, (node_t*)free_block );

    // defragment heap by trying to merge this block with its right and left blocks
    try_merge_left( free_block, (free_block_t*)free_block->node.next );
    try_merge_left( (free_block_t*)free_block->node.prior, free_block );
}

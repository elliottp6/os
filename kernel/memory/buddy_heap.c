#include <stdint.h>
#include <stdbool.h>
#include "bit_tree.h"
#include "buddy_heap.h"

static circular_list_node_t *node_to_block( void *start, size_t bucket, size_t node ) {
    return start + ((node - (1 << bucket) + 1) << (MAX_ALLOC_LOG2 - bucket));
}

static size_t node_from_block( void *start, size_t bucket, circular_list_node_t *block ) {
    return (((void*)block - start) >> (MAX_ALLOC_LOG2 - bucket)) + (1 << bucket) - 1;
}

// returns index of smallest bucket that can fit the requested size
static size_t min_bucket_that_fits( size_t request_size ) {
    size_t bucket = BUCKET_COUNT - 1, size = MIN_ALLOC;
    while( size < request_size ) { bucket--; size*=2; }
    return bucket;
}

void buddy_heap_init( buddy_heap_t *heap, void *start, size_t size ) {
    // set start & end pointers for heap
    heap->start = start;
    heap->end = start + size;

    // clear bit tree
    bit_tree_clear( heap->bit_tree, NODE_CHUNKS );

    // initialize buckets
    circular_list_node_t *buckets = heap->buckets;
    for( int i = 0; i < BUCKET_COUNT; i++ ) circular_list_init( &buckets[i] );

    // push free blocks
    // TODO
}

void* buddy_heap_allocate( buddy_heap_t* heap, size_t size ) {
    // determine bucket
    size+= HEADER_SIZE;
    if( size > MAX_ALLOC ) return NULL;
    size_t bucket = min_bucket_that_fits( size );
     
    // find smallest free block that is large enough
    circular_list_node_t *buckets = heap->buckets, *free_block;
    size_t free_bucket = bucket;
    while( !(free_block = circular_list_pop_next( &buckets[free_bucket] )) ) {
        if( 0 == free_bucket ) return NULL; // if root (largest) bucket fails, we're done
        free_bucket--; // try next-larger bucket
    }

    // mark block as used
    size_t node_index = node_from_block( heap->start, bucket, free_block );
    bit_tree_flip_parent_value( heap->bit_tree, node_index );

    // split block if it's too large
    while( free_bucket < bucket ) {
        // mark child block as used
        bit_tree_flip_value( heap->bit_tree, node_index );

        // move to left child
        node_index = bit_tree_get_left_child_index( node_index );
        bucket++;

        // insert right child into free list
        size_t right_child = node_index + 1;
        circular_list_node_t *right_block = node_to_block( heap->start, bucket, right_child );
        circular_list_insert_after( &buckets[bucket], right_block );
    }
    
    // must skip past the node's header
    return free_block + HEADER_SIZE;
}

void buddy_heap_free( buddy_heap_t* heap, void* ptr ) {
    // TODO
}

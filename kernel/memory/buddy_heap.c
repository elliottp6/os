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
    while( size < request_size ) { bucket--; size<<=1; }
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
    // determine bucket;
    if( size + HEADER_SIZE > MAX_ALLOC ) return NULL;
    size_t desired_bucket = min_bucket_that_fits( size + HEADER_SIZE );
    
    // find smallest free block that is large enough
    circular_list_node_t *buckets = heap->buckets, *block;
    size_t bucket = desired_bucket;
    while( !(block = circular_list_pop_next( &buckets[bucket] )) ) {
        if( 0 == bucket ) return NULL; // if root (largest) bucket fails, we're done
        bucket--; // try next-larger bucket
    }

    // mark block as used
    size_t node = node_from_block( heap->start, desired_bucket, block );
    bit_tree_flip_parent_value( heap->bit_tree, node );

    // split block (if it's too large)
    while( bucket < desired_bucket ) {
        // mark child block as used
        bit_tree_flip_value( heap->bit_tree, node );

        // move to left child
        node = bit_tree_get_left_child_index( node );
        bucket++;

        // insert right child into free list
        size_t right_child_node = node + 1;
        circular_list_node_t *right_block = node_to_block( heap->start, bucket, right_child_node );
        circular_list_insert_after( &buckets[bucket], right_block );
    }
    
    // usable space is right after the block's header (which contains its bucket)
    *(size_t*)block = bucket;
    return block + HEADER_SIZE;
}

void buddy_heap_free( buddy_heap_t* heap, void* ptr ) {
    // ignore null (or panic?)
    if( NULL == ptr ) return;

    // get block, bucket and node
    circular_list_node_t *block = (circular_list_node_t*)(ptr - HEADER_SIZE);
    size_t bucket = *(size_t*)block;
    size_t node = node_from_block( heap->start, bucket, block );

    // merge blocks
    while( 0 != node ) {
        // TODO
    }

    // add bucket to the free list
    // TODO
}

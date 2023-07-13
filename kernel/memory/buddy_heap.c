#include <stdint.h>
#include <stdbool.h>
#include "circular_list.h"
#include "buddy_heap.h"

#define HEADER_SIZE 8 // 8-byte header for each allocated block
#define MIN_ALLOC_LOG2 4 // minimum allocation size is 16 bytes (8 byte header + 8 byte object), which keeps us 8-byte aligned, and also matches the sizeof( circular_list_node_t ).
#define MIN_ALLOC ((size_t)1 << MIN_ALLOC_LOG2)
#define MAX_ALLOC_LOG2 31 // maximum allocation size is 2GB
#define MAX_ALLOC ((size_t)1 << MAX_ALLOC_LOG2)
#define BUCKET_COUNT (MAX_ALLOC_LOG2 - MIN_ALLOC_LOG2 + 1) // one bucket for each free list of pow2 size
#define NODE_COUNT (1 << (BUCKET_COUNT - 1)) // # of nodes in linearized binary tree of bits

typedef struct heap {
    uint8_t *start, *end, node_bits[NODE_COUNT / 8];
    circular_list_node_t buckets[BUCKET_COUNT]; // root list node for each bucket. 0th bucket is largest (MAX_ALLOC).
} heap_t;

// 
static uint8_t *node_to_ptr( uint8_t *start, size_t bucket, size_t node ) {
    return start + ((node - (1 << bucket) + 1) << (MAX_ALLOC_LOG2 - bucket));
}

static size_t node_from_ptr( uint8_t *start, size_t bucket, uint8_t *ptr ) {
    return ((ptr - start) >> (MAX_ALLOC_LOG2 - bucket)) + (1 << bucket) - 1;
}

static size_t node_get_parent( size_t node ) {
    return (node - 1) / 2;
}

static int node_get_bit( uint8_t *node_bits, size_t node ) {
    return (node_bits[node / 8] >> (node % 8)) & 1;
}

static void node_flip_bit( uint8_t *node_bits, size_t node ) {
    node_bits[node / 8] ^= 1 << (node % 8);
}

static int node_get_parent_bit( uint8_t *node_bits, size_t node ) {
    return node_get_bit( node_bits, node_get_parent( node ) );
}

static void node_flip_parent_bit( uint8_t *node_bits, size_t node ) {
    node_flip_bit( node_bits, node_get_parent( node ) );
}

// returns index of smallest bucket that can fit the requested size
static size_t min_bucket_that_fits( size_t request_size ) {
    size_t bucket = BUCKET_COUNT - 1, size = MIN_ALLOC;
    while( size < request_size ) { bucket--; size*=2; }
    return bucket;
}

void buddy_heap_init( void *heap, size_t size ) {
    
}

void* buddy_heap_allocate( void* heap, size_t size ) {
    //heap_t *h = (heap_t*)heap;

    // if requesting too much memory, go ahead and fail
    //if( size + HEADER_SIZE > MAX_ALLOC ) return NULL;

    // 
    return NULL;
}

void buddy_heap_free( void* heap, void* p ) {

}

/*
#define MIN_BLOCK_SIZE 32 // smallest than 32-bytes isn't worth it due to overhead
#define NUM_BLOCK_SIZES 10 // blocks up to 32KB (32,64...16KB,32KB)

typedef struct block_header {
    struct block_header *prior, *next;
} block_header_t;

typedef struct heap_header { // note: we could make the free_blocks array dynamic since for smaller heaps we don't need NUM_BLOCK_SIZES, but for now it's easier to leave it as a static size
    block_header_t *free_blocks[NUM_BLOCK_SIZES];
} heap_header_t;

// heap_header contains free pointers for each pow2 size block
// [0] => free MIN_BLOCK_SIZE byte block
// [1] => free (MIN_BLOCK_SIZE << 1) byte block
// [2] => free (MIN_BLOCK_SIZE << 2) byte block
// ...
void buddy_heap_init( void *heap, size_t size ) {
    // heap header lives @ the start of the heap
    heap_header_t *heap_header = (heap_header_t*)heap;

    // blocks live past the heap header
    heap+= sizeof( heap_header_t );
    size-= sizeof( heap_header_t );

    // populate free list (large..small blocks)
    for( size_t i = NUM_BLOCK_SIZES - 1; i >= 0; i++ ) {
        // size of this block
        size_t block_size = MIN_BLOCK_SIZE << i;

        // if no space for a free block here, set to NULL
        if( block_size > size ) {
            heap_header->free_blocks[i] = NULL;
            continue;
        }

        // initialize block
        block_header_t *block_header = (block_header_t*)heap;
        block_header->prior = NULL;
        block_header->next = NULL;
        heap_header->free_blocks[i] = block_header;

        // move position in heap
        heap+= block_size;
        size-= block_size;
    }
}

// (1) if we have block of size, use block
//     if we have block of > 2*size, split that block and goto 1
//     otherwise, ERROR_NOMEM, or just PANIC
// (2) fix block forward pointers (each block points to next & prev block of same size)
// (3) fix root block table
void* buddy_heap_allocate( void* heap, size_t size ) {
    // adjust object size to include header
    size+= sizeof( block_header_t );
    size_t twice_size = size << 1;

    // heap header lives @ the start of the heap
    // we can just case it directly to a list of free blocks for faster access
    block_header_t *free_blocks = (block_header_t*)heap;

    // look for the smallest free block that will contain this object w/ its header
    for( size_t i = 0, block_size = MIN_BLOCK_SIZE; i < NUM_BLOCK_SIZES; i++, block_size<<=1 ) {
        block_header_t *block_header = &free_blocks[i];
        
        // skip block if it's missing, or if it's too small
        if( (NULL == block_header) | (size > block_size) ) continue;

        // split block if it's twice as big as required
        if( block_size >= twice_size ) {
            // backup block header
            //block_header_t block_header_backup = *block_header;

            // point header at the next block of this size
            //block_header = block_header->next;
            
            // ui
            // TODO

            // insert this into the list for smaller blocks
            // TODO

            // repeat the last loop iteration
            i-=2;
            block_size>>=2;
            continue;
        }

        // allocate the block
        // TODO
    }

    // not enough free contiguous space to allocate
    return NULL;
}

void buddy_heap_free( void* heap, void* p ) {
    // get block header for the object
    // block_header_t *block_header = (block_header_t*)(p - sizeof( block_header_t ));

    // remove block by fixing up pointers of the prior & next blocks
    // TODO

    // update the free_blocks list to point here IF this block is earlier in the list??
    // BUT: what to do about the older free block? I guess this block could point to that block, forming our free list, but right now even nonfree blocks are part of that list.
    // need to reconsider design a bit
    // TODO

    // check if neighboring blocks can be merged? Makes sense since we split in allocate, so we should merge in free
    // TODO
}
*/

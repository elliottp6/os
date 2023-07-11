#include "heap.h"

#define MIN_BLOCK_SIZE 32 // smallest than 32-bytes isn't worth it due to overhead
#define NUM_BLOCK_SIZES 10 // blocks up to 32KB (32,64...16KB,32KB)

typedef struct block_header {
    struct block_header *prior, *next;
    uint8_t side;
} block_header_t;

typedef struct heap_header { // note: we could make the free_blocks array dynamic since for smaller heaps we don't need NUM_BLOCK_SIZES, but for now it's easier to leave it as a static size
    block_header_t *free_blocks[NUM_BLOCK_SIZES];
} heap_header_t;

// heap_header contains free pointers for each pow2 size block
// [0] => free MIN_BLOCK_SIZE byte block
// [1] => free (MIN_BLOCK_SIZE << 1) byte block
// [2] => free (MIN_BLOCK_SIZE << 2) byte block
// ...
void heap_init( void *heap, size_t size ) {
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
void* heap_allocate( void* heap, size_t size ) {
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
            block_header_t block_header_backup = *block_header;

            // point header at the next block of this size
            block_header = block_header->next;
            
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

void heap_free( void* heap, void* p ) {
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

#include "heap.h"

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
void heap_init( void *heap, size_t size ) {
    // grab header
    heap_header_t *heap_header = (heap_header_t*)heap;

    // skip past the heap header
    heap+= sizeof( heap_header_t );
    size -= sizeof( heap_header_t );

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

/*
alloc:
(1) if we have block of size, use block
    if we have block of > 2*size, split that block and goto 1
    otherwise, ERROR_NOMEM, or just PANIC
(2) fix block forward pointers (each block points to next & prev block of same size)
(3) fix root block table

free:
(1) address - N bytes to get block header
(2) set prev block to point to next block of same size (i.e. removing from list)
(3) if this block is earlier in list than root block's pointer, update root block's pointer
*/
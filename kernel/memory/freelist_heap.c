#include <stdbool.h>
#include <stdint.h>
#include "circular_list.h"
#include "freelist_heap.h"
#include "../buffer/string.h" // print freelist_heap_print
#include "../drivers/vga_text.h" // "
#include "../main.h" // for panic

//#define TRACE
#define PANIC_ON_OUT_OF_MEMORY

typedef circular_list_node_t node_t;

typedef struct used_block {
    size_t size;
} used_block_t;

typedef struct free_block {
    node_t node;
    size_t size;
} free_block_t;

typedef struct heap {
    size_t size;
    free_block_t root;
} heap_t;

// align must be a power of 2 (or else undefined behavior)
static size_t upalign( size_t value, size_t align ) {
    return (value + align - 1) & ~(align - 1);
}

void freelist_heap_init( void *heap_start, size_t heap_size ) {
    // heap_start must be aligned with size_t
    if( (size_t)heap_start != upalign( (size_t)heap_start, sizeof( size_t ) ) ) {
        panic( "heap-start must 8-byte aligned\n" );
    }

    // initialize heap
    heap_t *heap = (heap_t*)heap_start;
    heap->size = heap_size;
    heap->root.size = 0;
    circular_list_init( &heap->root.node );

    // check if we have enough space for 1st free block
    if( heap_size < sizeof( heap_t ) + sizeof( free_block_t ) ) return;

    // insert first free block
    free_block_t *free_block = (free_block_t*)(heap_start + sizeof( heap_t ) );
    circular_list_insert_after( &heap->root.node, &free_block->node );
    free_block->size = heap_size - sizeof( heap_t );

    #ifdef TRACE
    vga_text_print( "freelist_heap_init: free_block @ ", 0x17 );
    vga_text_print( string_from_int64( *(int64_t*)free_block ), 0x17 );
    vga_text_print( " with size ", 0x17 );
    vga_text_print( string_from_int64( (int64_t)free_block->size ), 0x17 );
    vga_text_print( " root->next ", 0x17 );
    vga_text_print( string_from_int64( (int64_t)(heap->root.node.next) ), 0x17 );
    vga_text_print( "\n", 0x17 );
    #endif
}

static bool free_block_is_big_enough( node_t *free_block_node, void *min_free_block_size ) {
    free_block_t *free_block = (free_block_t*)free_block_node;

    #ifdef TRACE
    vga_text_print( "free_block_is_big_enough: checking free block @ ", 0x17 );
    vga_text_print( string_from_int64( (int64_t)free_block ), 0x17 );
    vga_text_print( " of size ", 0x17 );
    vga_text_print( string_from_int64( (int64_t)free_block->size ), 0x17 );
    vga_text_print( " vs min of ", 0x17 );
    vga_text_print( string_from_int64( *(int64_t*)min_free_block_size ), 0x17 );
    vga_text_print( "\n", 0x17 );
    #endif

    return free_block->size >= *(size_t*)min_free_block_size;
}

void *freelist_heap_alloc( void *heap_start, size_t object_size ) {
    // object_size must be aligned to pointer size
    object_size = upalign( object_size, sizeof( size_t ) );

    // define minimum block size
    size_t min_block_size = sizeof( used_block_t ) + object_size;
    if( min_block_size < sizeof( free_block_t ) ) min_block_size = sizeof( free_block_t );

    // see if we can find a free block that's big enough
    heap_t *heap = (heap_t*)heap_start;
    free_block_t *free_block = (free_block_t*)circular_list_find( &heap->root.node, free_block_is_big_enough, &min_block_size );
    if( NULL == free_block ) {
        #ifdef PANIC_ON_OUT_OF_MEMORY
        panic( "freelist_heap_alloc: no sufficiently-large free blocks in heap\n" );
        #endif
        return NULL;
    }
    
    #ifdef TRACE
    else {
        vga_text_print( "freelist_heap_alloc: found free_block @ ", 0x17 );
        vga_text_print( string_from_int64( (int64_t)free_block ), 0x17 );
        vga_text_print( "\n", 0x17 );
    }
    #endif

    // if there's enough free space in the block: split it
    size_t free_space_size = free_block->size - min_block_size;
    if( free_space_size >= sizeof( free_block_t ) ) {
        // shrink block to its minimum size
        free_block->size = min_block_size;

        // initialize new free block right after this block
        free_block_t *new_free_block = (void*)free_block + min_block_size;
        new_free_block->size = free_space_size;

        // replace block with the new free block
        circular_list_replace( &free_block->node, &new_free_block->node );
    } else {
        // remove block from the free list
        circular_list_remove( &free_block->node );
    }

    // convert free_block into used_block
    used_block_t* used_block = (used_block_t*)free_block;
    used_block->size = free_block->size;

    // return pointer to the used block's data
    return (void*)used_block + sizeof( used_block_t );
}

static void try_merge_left( free_block_t *left, free_block_t *right ) {
    // blocks must be different and touching to be mergeable
    if( left == right || (void*)left + left->size != (void*)right ) return;

    // merge right block into left
    left->size+= right->size;
    circular_list_remove( &right->node );
}

void freelist_heap_free( void *heap_start, void* ptr ) {
    // get used block & block_size
    used_block_t *used_block = (used_block_t*)(ptr - sizeof(used_block_t));
    size_t block_size = used_block->size;

    // turn this into a free block
    free_block_t *free_block = (free_block_t*)used_block;
    free_block->size = block_size;

    // insert into freelist at correct memory location
    // WARNING: this is an O(n) linear search across the heap (the most expensive operation we have in the freelist_heap)
    heap_t *heap = (heap_t*)heap_start;
    node_t *root = &heap->root.node;
    node_t *node = root->next;
    while( node != root && (void*)node < (void*)free_block ) node = node->next;
    circular_list_insert_before( node, &free_block->node );

    // defragment heap by trying to merge this block with its right and left blocks
    try_merge_left( free_block, (free_block_t*)node );
    try_merge_left( (free_block_t*)free_block->node.prior, free_block );
}

size_t freelist_heap_free_block_count( void *heap_start ) {
    heap_t *heap = (heap_t*)heap_start;
    return circular_list_length( &heap->root.node ) - 1;
}

static void print_free_block( node_t *node, void *closure ) {
    free_block_t *block = (free_block_t*)node;
    vga_text_print( "[", 0x17 );
    vga_text_print( string_from_int64( block->size ), 0x17 );
    vga_text_print( "]", 0x17 );
}

void freelist_heap_print( void *heap_start ) {
    heap_t *heap = (heap_t*)heap_start;
    circular_list_foreach( &heap->root.node, print_free_block, NULL );
}

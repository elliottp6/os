#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "circular_list.h"

// TODO: all of this can be moved into the implementation file
// we can instead just put the 'freelist_heap' header object
// directly into the beginning of the heapspace itself
typedef struct freelist_used_block {
    size_t block_size;
} freelist_used_block_t;

typedef struct freelist_free_block {
    circular_list_node_t node;
    size_t block_size;
} freelist_free_block_t;

typedef struct freelist_heap {
    void *start;
    size_t size;
    freelist_free_block_t root;
} freelist_heap_t;

void freelist_heap_init( freelist_heap_t *heap, void *start, size_t size );
void *freelist_heap_allocate( freelist_heap_t *heap, size_t size );
void freelist_heap_free( freelist_heap_t *heap, void *ptr );

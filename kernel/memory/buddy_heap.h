#pragma once
#include <stddef.h>
#include "circular_list.h"

// TODO: rename these defines to eliminate conflicts
#define HEADER_SIZE 8 // 8-byte header for each allocated block
#define MIN_ALLOC_LOG2 4 // minimum allocation size is 16 bytes (8 byte header + 8 byte object), which keeps us 8-byte aligned, and also matches the sizeof( circular_list_node_t ).
#define MIN_ALLOC ((size_t)1 << MIN_ALLOC_LOG2)
#define MAX_ALLOC_LOG2 31 // maximum allocation size is 2GB
#define MAX_ALLOC ((size_t)1 << MAX_ALLOC_LOG2)
#define BUCKET_COUNT (MAX_ALLOC_LOG2 - MIN_ALLOC_LOG2 + 1) // one bucket for each free list of pow2 size
#define NODE_COUNT (1 << (BUCKET_COUNT - 1)) // # of nodes in linearized binary tree of bits = one for every potential block EXCEPT for the smallest blocks (i.e. we only care about storing the 'is_split' info for each parent)
#define NODE_CHUNKS (NODE_COUNT / 64)

typedef struct buddy_heap {
    void *start, *end;
    uint64_t bit_tree[NODE_CHUNKS];
    circular_list_node_t buckets[BUCKET_COUNT];
} buddy_heap_t;

void buddy_heap_init( buddy_heap_t *heap, void* start, size_t size );
void* buddy_heap_allocate( buddy_heap_t* heap, size_t size );
void buddy_heap_free( buddy_heap_t* heap, void* ptr );

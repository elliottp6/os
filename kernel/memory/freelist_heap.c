#include "freelist_heap.h"

typedef struct free_block {
    circular_list_node_t node;
    size_t size;
    uint8_t data[];
} free_block;

void freelist_heap_init( freelist_heap *heap, void* start, size_t size ) {
    heap->start = start;
    heap->size = size;

    // allocate root block
    // TODO
}

static free_block *find_free_block( freelist_heap *heap ) {
    return NULL;
}

void* freelist_heap_allocate( freelist_heap* heap, size_t size ) {
    return NULL;
}

void freelist_heap_free( freelist_heap* heap, void* ptr ) {

}

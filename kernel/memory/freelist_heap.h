#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "circular_list.h"

typedef struct freelist_heap {
    void* start;
    size_t size
} freelist_heap;

//void freelist_heap_init( freelist_heap *heap, void* start, size_t size );
//void* freelist_heap_allocate( freelist_heap* heap, size_t size );
//void freelist_heap_free( freelist_heap* heap, void* ptr );

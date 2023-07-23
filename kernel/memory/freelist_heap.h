#pragma once

#include <stddef.h>

void freelist_heap_init( void *heap_start, size_t heap_size );
void *freelist_heap_alloc( void *heap_start, size_t object_size );
void freelist_heap_free( void *heap_start, void *object );
size_t freelist_heap_free_block_count( void *heap_start );
void freelist_heap_print( void *heap_start );

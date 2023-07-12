#pragma once
#include <stddef.h>

void buddy_heap_init( void *heap, size_t size );
void* buddy_heap_allocate( void* heap, size_t size );
void buddy_heap_free( void* heap, void* p );

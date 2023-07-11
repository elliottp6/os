#pragma once
#include <stddef.h>

void heap_init( void *heap, size_t size );
void* heap_allocate( void* heap, size_t size );
void heap_free( void* heap, void* p );

#include <stddef.h>

#define KERNEL_HEAP_START 0x1000000 // 16 MB
#define KERNEL_HEAP_SIZE 104857600 // 100 MB

void kernel_heap_init();
void *kernel_heap_alloc( size_t object_size );
void kernel_heap_free( void *object );

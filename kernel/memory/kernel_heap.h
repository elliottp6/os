#include <stddef.h>

#define KERNEL_HEAP_START 0x300000 // 3 MB
#define KERNEL_HEAP_END 0x10000000 // 256 MB (max physical memory)
#define KERNEL_HEAP_SIZE (KERNEL_HEAP_END - KERNEL_HEAP_START)

void kernel_heap_init();
void *kernel_heap_alloc( size_t object_size );
void kernel_heap_free( void *object );

#include <stddef.h>

#define KERNEL_HEAP_START 0x200000 // 2 MB (i.e. the heap starts right at the top of the stack)
#define KERNEL_HEAP_END 0x40000000 // 1 GB (max physical memory)
#define KERNEL_HEAP_SIZE (KERNEL_HEAP_END - KERNEL_HEAP_START)

void kernel_heap_init();
void *kernel_heap_alloc( size_t object_size );
void kernel_heap_free( void *object );

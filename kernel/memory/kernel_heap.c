#include <stdint.h>
#include "kernel_heap.h"
#include "freelist_heap.h"
#include "../text/string.h" // for error messages
#include "../text/vga_text.h" // for error messages
#include "../main.h" // for panic

static void kernel_heap_test() {
    // run a self-test
    // heap should start w/ just a single free block
    size_t free_block_count = freelist_heap_free_block_count( (void*)KERNEL_HEAP_START );
    if( 1 != free_block_count ) {
        vga_text_print( "kernel_heap_init: free block count starts @ ", 0x17 );
        vga_text_print( string_int64_to_temp( (int64_t)free_block_count ), 0x17 );
        vga_text_print( "\n", 0x17 );
        panic( "kernel_heap_init: expect initial free_block_count to be 1" );
    }

    // test the kernel heap. 1st object should be 40 bytes away from the heap's 2MB start, b/c we need 32 bytes for the heap's header, and then 8 bytes for the block's size.
    void *obj1 = kernel_heap_alloc( 8 );
    if( (int64_t)obj1 != (int64_t)(0x200000 + 40) ) panic( "kernel_heap_init: 1st allocated object must be 8 bytes after the heap's header" );
    
    // should have a single free block (shifted to after obj1)
    free_block_count = freelist_heap_free_block_count( (void*)KERNEL_HEAP_START );
    if( 1 != free_block_count ) {
        vga_text_print( "kernel_heap_init: after 1st allocation, free block count is ", 0x17 );
        vga_text_print( string_int64_to_temp( (int64_t)free_block_count ), 0x17 );
        vga_text_print( "\n", 0x17 );
        panic( "kernel_heap_init: expect free_block_count to be 1" );
    }

    // 1st block is at 32 bytes from heap's start, and it's only 24 bytes in size, so 2nd block should be 32+24 56 bytes, so 2nd object is @ 64 bytes (b/c we need an 8-byte size header)
    void *obj2 = kernel_heap_alloc( 16 );
    if( (int64_t)obj2 != (int64_t)(0x200000 + 64) ) panic( "kernel_heap_init: 2nd allocated object must be 64 bytes after the heap's header" );

    // should still have just a single free block
    free_block_count = freelist_heap_free_block_count( (void*)KERNEL_HEAP_START );
    if( 1 != free_block_count ) {
        vga_text_print( "kernel_heap_init: after 2nd allocation, free block count is ", 0x17 );
        vga_text_print( string_int64_to_temp( (int64_t)free_block_count ), 0x17 );
        vga_text_print( "\n", 0x17 );
        panic( "kernel_heap_init: expect free_block_count to be 1" );
    }

    // free obj1, which should create a hole, where we'll have a small free block
    kernel_heap_free( obj1 );

    // shoud now have 2 free blocks
    free_block_count = freelist_heap_free_block_count( (void*)KERNEL_HEAP_START );
    if( 2 != free_block_count ) {
        vga_text_print( "kernel_heap_init: after freeing 1st obj, free block count is ", 0x17 );
        vga_text_print( string_int64_to_temp( (int64_t)free_block_count ), 0x17 );
        vga_text_print( "\n", 0x17 );
        panic( "kernel_heap_init: expect free_block_count to be 2" );
    }

    // reuse that first free block (note that an object up to size 16 can use that first free block)
    obj1 = kernel_heap_alloc( 16 );
    if( (int64_t)obj1 != (int64_t)(0x200000 + 40) ) panic( "kernel_heap_init: 1st allocated object must be 8 bytes after the heap's header" );

    // now we should be back to just 1 free block
    free_block_count = freelist_heap_free_block_count( (void*)KERNEL_HEAP_START );
    if( 1 != free_block_count ) {
        vga_text_print( "kernel_heap_init: after allocating 1st obj again, free block count is ", 0x17 );
        vga_text_print( string_int64_to_temp( (int64_t)free_block_count ), 0x17 );
        vga_text_print( "\n", 0x17 );
        panic( "kernel_heap_init: expect free_block_count to be 1" );
    }

    // free it again, which gets us back to 2 free blocks
    kernel_heap_free( obj1 );
    free_block_count = freelist_heap_free_block_count( (void*)KERNEL_HEAP_START );
    if( 2 != free_block_count ) {
        vga_text_print( "kernel_heap_init: after freeing 1st obj yet again, free block count is ", 0x17 );
        vga_text_print( string_int64_to_temp( (int64_t)free_block_count ), 0x17 );
        vga_text_print( "\n", 0x17 );
        panic( "kernel_heap_init: expect free_block_count to be 2" );
    }

    // now allocate an object > 16 bytes, which means we cannot use the first block
    obj1 = kernel_heap_alloc( 17 );
    if( (int64_t)obj1 != (int64_t)(0x200000 + 88) ) panic( "kernel_heap_init: big allocated object must be 88 bytes after the heap's header" );

    // this means we still have 2 free blocks
    // TODO: finish testing

    // free both objects
    kernel_heap_free( obj1 );
    kernel_heap_free( obj2 );

    // now we should be back to a single contiguous free block
    free_block_count = freelist_heap_free_block_count( (void*)KERNEL_HEAP_START );
    if( 1 != free_block_count ) {
        vga_text_print( "kernel_heap_init: after freeing objects, free block count is now ", 0x17 );
        vga_text_print( string_int64_to_temp( (int64_t)free_block_count ), 0x17 );
        vga_text_print( "\n", 0x17 );
        panic( "kernel_heap_init: after freeing all blocks, expect free_block_count to be 1" );
    }
}

void kernel_heap_init() {
    // initialize the heap
    freelist_heap_init( (void*)KERNEL_HEAP_START, KERNEL_HEAP_SIZE );

    // run self-tests
    kernel_heap_test();
}

void *kernel_heap_alloc( size_t object_size ) {
    return freelist_heap_alloc( (void*)KERNEL_HEAP_START, object_size );
}

void kernel_heap_free( void *object ) {
    freelist_heap_free( (void*)KERNEL_HEAP_START, object );
}

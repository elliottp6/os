#include "bit_tree.h"

size_t bit_tree_get_parent_index( size_t i ) {
    return (i - 1) >> 1;
}

size_t bit_tree_get_left_child_index( size_t i ) {
    return (i << 1) + 1;
}

size_t bit_tree_get_right_child_index( size_t i ) {
    return (i << 1) + 2;
}

uint64_t bit_tree_get_value( uint64_t* bit_tree, size_t i ) {
    return (bit_tree[i >> 5] >> (i & 63)) & 1;
}

void bit_tree_set_value( uint64_t* bit_tree, size_t i ) {
    bit_tree[i >> 5]|= 1 << (i & 63);
}

void bit_tree_flip_value( uint64_t *bit_tree, size_t i ) {
    bit_tree[i >> 5] ^= 1 << (i & 63);
}

void bit_tree_clear_value( uint64_t* bit_tree, size_t i ) {
    bit_tree[i >> 5]&= ~(1 << (i & 63));
}

void bit_tree_change_value( uint64_t* bit_tree, size_t i, uint64_t value ) {
    uint64_t chunk = i >> 5, bit = i & 63;
    bit_tree[chunk] = (bit_tree[chunk] & ~(1 << bit)) | ((value & 1) << bit);
}

uint64_t bit_tree_get_parent_value( uint64_t* bit_tree, size_t i ) {
    if( 0 == i ) return 0;
    return bit_tree_get_value( bit_tree, bit_tree_get_parent_index( i ) );
}

void bit_tree_flip_parent_value( uint64_t *bit_tree, size_t i ) {
    if( 0 == i ) return;
    bit_tree_flip_value( bit_tree, bit_tree_get_parent_index( i ) );
}

void bit_tree_clear( uint64_t* bit_tree, size_t uint64_size ) {
    for( int i = 0; i < uint64_size; i++ ) bit_tree[i] = 0;
}

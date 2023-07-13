#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

size_t bit_tree_get_parent_index( size_t i );
size_t bit_tree_get_left_child_index( size_t i );
size_t bit_tree_get_right_child_index( size_t i );
uint64_t bit_tree_get_value( uint64_t* bit_tree, size_t i );
void bit_tree_set_value( uint64_t* bit_tree, size_t i );
void bit_tree_flip_value( uint64_t *bit_tree, size_t i );
void bit_tree_clear_value( uint64_t* bit_tree, size_t i );
void bit_tree_change_value( uint64_t* bit_tree, size_t i, uint64_t value );
uint64_t bit_tree_get_parent_value( uint64_t* bit_tree, size_t i );
void bit_tree_flip_parent_value( uint64_t *bit_tree, size_t i );
void bit_tree_clear( uint64_t* bit_tree, size_t uint64_size );

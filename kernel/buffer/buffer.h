#pragma once

#include <stdint.h>
#include <stddef.h>

void buffer_set_qwords( uint64_t *buffer, uint64_t value, size_t count );
void buffer_clear_qwords( uint64_t *buffer, size_t count );

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

char *string_from_int64( int64_t n );
size_t string_length( const char* p );
int string_compare( const char* a, const char* b );
bool string_equal( const char* a, const char* b );
void string_run_tests();

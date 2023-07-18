#pragma once

#include <stdbool.h>

typedef struct circular_list_node {
    struct circular_list_node *prior, *next;
} circular_list_node_t;

void circular_list_init( circular_list_node_t *root );
void circular_list_insert_after( circular_list_node_t *location, circular_list_node_t *new_node );
void circular_list_insert_before( circular_list_node_t *location, circular_list_node_t *new_node );
void circular_list_remove( circular_list_node_t *node );
void circular_list_replace( circular_list_node_t *node, circular_list_node_t *replacement );
circular_list_node_t *circular_list_pop_next( circular_list_node_t *node );
circular_list_node_t *circular_list_pop_prior( circular_list_node_t *node );
circular_list_node_t *circular_list_find( circular_list_node_t *start, bool (*match)(circular_list_node_t* node, void *closure), void *closure );

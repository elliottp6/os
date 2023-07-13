#include <stddef.h>
#include "circular_list.h"

void circular_list_init( circular_list_node_t *root ) {
    root->prior = root->next = root;
}

void circular_list_insert_after( circular_list_node_t *location, circular_list_node_t *new_node ) {
    circular_list_node_t *next = location->next;
    new_node->prior = location;
    new_node->next = next;
    location->next = next->prior = new_node;
}

void circular_list_insert_before( circular_list_node_t *location, circular_list_node_t *new_node ) {
    circular_list_node_t *prior = location->prior;
    new_node->prior = prior;
    new_node->next = location;
    location->prior = prior->next = new_node;
}

void circular_list_remove( circular_list_node_t *node ) {
    circular_list_node_t *prior = node->prior, *next = node->next;
    prior->next = next;
    next->prior = prior;
}

circular_list_node_t *circular_list_pop_next( circular_list_node_t *node ) {
    circular_list_node_t *next = node->next;
    if( next == node ) return NULL;
    circular_list_remove( next );
    return next;
}

circular_list_node_t *circular_list_pop_prior( circular_list_node_t *node ) {
    circular_list_node_t *prior = node->prior;
    if( prior == node ) return NULL;
    circular_list_remove( prior );
    return prior;
}
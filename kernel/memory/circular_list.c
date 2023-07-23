#include "circular_list.h"

typedef circular_list_node_t node_t;

void circular_list_init( node_t *root ) {
    root->prior = root->next = root;
}

void circular_list_insert_after( node_t *location, node_t *new_node ) {
    node_t *next = location->next;
    new_node->prior = location;
    new_node->next = next;
    location->next = next->prior = new_node;
}

void circular_list_insert_before( node_t *location, node_t *new_node ) {
    node_t *prior = location->prior;
    new_node->prior = prior;
    new_node->next = location;
    location->prior = prior->next = new_node;
}

void circular_list_remove( node_t *node ) {
    node_t *prior = node->prior, *next = node->next;
    prior->next = next;
    next->prior = prior;
}

void circular_list_replace( node_t *node, node_t *replacement ) {
    node_t *prior = node->prior, *next = node->next;
    replacement->next = next;
    replacement->prior = prior;
    prior->next = replacement;
    next->prior = replacement;
}

node_t *circular_list_pop_next( node_t *node ) {
    node_t *next = node->next;
    if( next == node ) return NULL;
    circular_list_remove( next );
    return next;
}

node_t *circular_list_pop_prior( node_t *node ) {
    node_t *prior = node->prior;
    if( prior == node ) return NULL;
    circular_list_remove( prior );
    return prior;
}

node_t *circular_list_find( node_t *start, bool (*match)(node_t *node, void *closure), void *closure ) {
    node_t *node = start;
    do {
        if( match( node, closure ) ) return node;
    } while( (node = node->next) != start );
    return NULL;
}

size_t circular_list_length( node_t *start ) {
    size_t count = 1;
    for( node_t *node = start; (node = node->next) != start; count++ );
    return count;
}

void circular_list_foreach( node_t *start, void (*on_node)(node_t *node, void *closure), void *closure ) {
    for( node_t *node = start; (node = node->next) != start; ) {
        on_node( node, closure );
    }
}

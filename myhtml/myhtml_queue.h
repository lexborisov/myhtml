//
//  myhtml_queue.h
//  myhtml
//
//  Created by Alexander Borisov on 26.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_queue_h
#define myhtml_queue_h

#include "myosi.h"
#include "myhtml.h"

#define myhtml_queue_node_get(__queue__, __idx__, __patam__) __queue__->nodes[__idx__].__patam__

#define myhtml_queue_node_malloc(__queue__, __html__, __token__, __sys__, __opt__, __tree__)  \
    myhtml_base_add(__queue__, nodes, nodes_length, nodes_size, myhtml_queue_node_t, 4096)    \
    __queue__->nodes[__queue__->nodes_length].html           = __html__;                      \
    __queue__->nodes[__queue__->nodes_length].token_idx      = __token__;                     \
    __queue__->nodes[__queue__->nodes_length].is_system      = __sys__;                       \
    __queue__->nodes[__queue__->nodes_length].opt            = __opt__;                       \
    __queue__->nodes[__queue__->nodes_length].myhtml_tree    = __tree__

#define myhtml_queue_node_current(__queue__) __queue__->nodes_length

struct myhtml_queue_node {
    myhtml_token_index_t token_idx;
    
    mybool_t is_system;
    enum myhtml_queue_node_opt opt;
    
    myhtml_tree_t* myhtml_tree;
    const char* html;
};

struct myhtml_queue {
    myhtml_queue_node_t* nodes;
    volatile size_t nodes_length;
    size_t nodes_size;
    size_t nodes_root;
};


myhtml_queue_t * myhtml_queue_create(size_t size);
void myhtml_queue_clean(myhtml_queue_t* queue);
myhtml_queue_t * myhtml_queue_destroy(myhtml_queue_t* token);

void myhtml_queue_node_clean(myhtml_queue_node_t* qnode);

#endif /* myhtml_queue_h */

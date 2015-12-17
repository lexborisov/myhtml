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
#include "token.h"

#define myhtml_queue_node_get(__queue__, __idx__, __patam__) __queue__->nodes[__idx__].__patam__

#define myhtml_queue_node_current(__queue__) __queue__->nodes_length
#define myhtml_queue_attr_node_current(__queue__) __queue__->attr_nodes_length

#define myhtml_queue_attr_node(__queue__) __queue__->attr_nodes[ myhtml_queue_attr_node_current(__queue__) ]
#define myhtml_queue_attr_node_get(__queue__, __idx__, __attr__) __queue__->attr_nodes[__idx__].__attr__

#define myhtml_queue_attr_node_malloc(__queue__)  \
    myhtml_base_add(__queue__, attr_nodes, attr_nodes_length, attr_nodes_size, myhtml_token_attr_t, 4096)

struct myhtml_queue_node {
    myhtml_token_node_t* token;
    
    size_t begin;
    size_t length;
    
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

size_t myhtml_queue_node_malloc(myhtml_queue_t* queue, const char* html, size_t begin,
                                mybool_t is_system, enum myhtml_queue_node_opt opt, myhtml_tree_t* tree);

#endif /* myhtml_queue_h */

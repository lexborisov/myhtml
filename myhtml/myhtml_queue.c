//
//  myhtml_queue.c
//  myhtml
//
//  Created by Alexander Borisov on 26.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_queue.h"


myhtml_queue_t * myhtml_queue_create(size_t size)
{
    myhtml_queue_t* queue = (myhtml_queue_t*)mymalloc(sizeof(myhtml_queue_t));
    
    queue->nodes_size = size;
    queue->nodes = (myhtml_queue_node_t*)mymalloc(sizeof(myhtml_queue_node_t) * queue->nodes_size);
    
    myhtml_queue_clean(queue);
    
    return queue;
}

void myhtml_queue_clean(myhtml_queue_t* queue)
{
    queue->nodes_length = 0;
    myhtml_queue_node_clean(&queue->nodes[myhtml_queue_node_current(queue)]);
    
    queue->nodes_root = myhtml_queue_node_malloc(queue, 0, 0, myfalse, 0, 0);
}

myhtml_queue_t * myhtml_queue_destroy(myhtml_queue_t* queue)
{
    if(queue == NULL)
        return NULL;
    
    if(queue->nodes)
        free(queue);
    
    return NULL;
}

void myhtml_queue_node_clean(myhtml_queue_node_t* qnode)
{
    qnode->html           = NULL;
    qnode->token          = NULL;
    qnode->begin          = 0;
    qnode->length         = 0;
    qnode->is_system      = myfalse;
    qnode->opt            = MyHTML_QUEUE_OPT_CLEAN;
    qnode->myhtml_tree    = NULL;
}

size_t myhtml_queue_node_malloc(myhtml_queue_t* queue, const char* html, size_t begin,
                                mybool_t is_system, enum myhtml_queue_node_opt opt, myhtml_tree_t* tree)
{
    myhtml_base_add(queue, nodes, nodes_length, nodes_size, myhtml_queue_node_t, 4096);
    
    queue->nodes[queue->nodes_length].html           = html;
    queue->nodes[queue->nodes_length].begin          = begin;
    queue->nodes[queue->nodes_length].is_system      = is_system;
    queue->nodes[queue->nodes_length].opt            = opt;
    queue->nodes[queue->nodes_length].myhtml_tree    = tree;
    
    return queue->nodes_length;
}



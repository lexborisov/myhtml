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
    myhtml_queue_t* queue = (myhtml_queue_t*)malloc(sizeof(myhtml_queue_t));
    
    queue->nodes_size = size;
    queue->nodes = (myhtml_queue_node_t*)malloc(sizeof(myhtml_queue_node_t) * queue->nodes_size);
    
    myhtml_queue_clean(queue);
    
    return queue;
}

void myhtml_queue_clean(myhtml_queue_t* queue)
{
    queue->nodes_length = 0;
    myhtml_queue_node_clean(&queue->nodes[myhtml_queue_node_current(queue)]);
    
    myhtml_queue_node_malloc(queue, 0, 0, myfalse, 0, 0);
    queue->nodes_root = myhtml_queue_node_current(queue);
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
    qnode->token_idx      = 0;
    qnode->is_system      = myfalse;
    qnode->opt            = MyHTML_QUEUE_OPT_CLEAN;
    qnode->myhtml_tree    = NULL;
}




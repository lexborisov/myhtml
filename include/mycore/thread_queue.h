/*
 Copyright (C) 2015-2017 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#ifndef MyCORE_THREAD_QUEUE_H
#define MyCORE_THREAD_QUEUE_H
#pragma once

#include <mycore/myosi.h>
#include <mycore/mythread.h>

#ifdef __cplusplus
extern "C" {
#endif

// queue
struct mythread_queue_node {
    void* context;
    void* args;
    
    mythread_queue_node_t* prev;
};

struct mythread_queue_thread_param {
    volatile size_t use;
};

struct mythread_queue_list_entry {
    mythread_queue_t*              queue;
    mythread_queue_thread_param_t* thread_param;
    size_t thread_param_size;
    
    mythread_queue_list_entry_t* next;
    mythread_queue_list_entry_t* prev;
};

struct mythread_queue_list {
    mythread_queue_list_entry_t *first;
    mythread_queue_list_entry_t *last;
    
    volatile size_t count;
};

struct mythread_queue {
    mythread_queue_node_t **nodes;
    
    size_t nodes_pos;
    size_t nodes_pos_size;
    size_t nodes_length;
    
    volatile size_t nodes_uses;
    volatile size_t nodes_size;
    volatile size_t nodes_root;
};

mythread_queue_t * mythread_queue_create(void);
mystatus_t mythread_queue_init(mythread_queue_t* queue, size_t size);
void mythread_queue_clean(mythread_queue_t* queue);
mythread_queue_t * mythread_queue_destroy(mythread_queue_t* token);

void mythread_queue_node_clean(mythread_queue_node_t* qnode);
size_t mythread_queue_count_used_node(mythread_queue_t* queue);

mythread_queue_node_t * mythread_queue_get_first_node(mythread_queue_t* queue);
mythread_queue_node_t * mythread_queue_get_prev_node(mythread_queue_node_t* qnode);
mythread_queue_node_t * mythread_queue_get_current_node(mythread_queue_t* queue);

mythread_queue_node_t * mythread_queue_node_malloc(mythread_t *mythread, mythread_queue_t* queue, mystatus_t *status);
mythread_queue_node_t * mythread_queue_node_malloc_limit(mythread_t *mythread, mythread_queue_t* queue, size_t limit, mystatus_t *status);
#ifndef MyCORE_BUILD_WITHOUT_THREADS
mythread_queue_node_t * mythread_queue_node_malloc_round(mythread_t *mythread, mythread_queue_list_entry_t *entry);
#endif

#ifndef MyCORE_BUILD_WITHOUT_THREADS
mythread_queue_list_t * mythread_queue_list_create(mystatus_t *status);
void mythread_queue_list_destroy(mythread_queue_list_t* queue_list);
size_t mythread_queue_list_get_count(mythread_queue_list_t* queue_list);

void mythread_queue_list_wait_for_done(mythread_t* mythread, mythread_queue_list_t* queue_list);
bool mythread_queue_list_see_for_done(mythread_t* mythread, mythread_queue_list_t* queue_list);
bool mythread_queue_list_see_for_done_by_thread(mythread_t* mythread, mythread_queue_list_t* queue_list, mythread_id_t thread_id);

mythread_queue_list_entry_t * mythread_queue_list_entry_push(mythread_t** mythread_list, size_t list_size, mythread_queue_list_t* queue_list, mythread_queue_t* queue, size_t thread_param_size, mystatus_t* status);
mythread_queue_list_entry_t * mythread_queue_list_entry_delete(mythread_t** mythread_list, size_t list_size, mythread_queue_list_t *queue_list, mythread_queue_list_entry_t *entry, bool destroy_queue);
void mythread_queue_list_entry_clean(mythread_queue_list_entry_t *entry);
void mythread_queue_list_entry_wait_for_done(mythread_t *mythread, mythread_queue_list_entry_t *entry);
void mythread_queue_list_entry_make_batch(mythread_t* mythread, mythread_queue_list_entry_t* entry);
void mythread_queue_list_entry_make_stream(mythread_t* mythread, mythread_queue_list_entry_t* entry);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyCORE_THREAD_QUEUE_H */

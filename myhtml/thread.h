//
//  myhtml_thread.h
//  myhtml
//
//  Created by Alexander Borisov on 02.10.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#ifndef __myhtml__myhtml_thread__
#define __myhtml__myhtml_thread__

#include "myosi.h"
#include "myhtml.h"
#include "tree.h"

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

#define MyTHREAD_SEM_NAME "/mythread"


void mythread_function_stream(void *arg);
void mythread_function_batch(void *arg);

// thread
struct mythread_context {
    mythread_id_t id;
    
    sem_t *sem;
    char *sem_name;
    size_t sem_name_size;
    
    volatile size_t use;
    volatile mythread_queue_node_t *qnode;
    
    mythread_f func;
    
    volatile size_t t_count;
    volatile mythread_thread_opt_t opt;
    
    mythread_t *mythread;
};

struct mythread_list {
    pthread_t pth;
    mythread_context_t data;
};

struct mythread_workers_list {
    mythread_list_t *list;
    size_t count;
};

struct mythread {
    mythread_list_t *pth_list;
    size_t pth_list_length;
    size_t pth_list_size;
    size_t pth_list_root;
    
    mythread_queue_t *queue;
    
    char  *sem_prefix;
    size_t sem_prefix_length;
    
    pthread_attr_t *attr;
    int sys_last_error;
    
    mythread_id_t batch_first_id;
    mythread_id_t batch_count;
    
    volatile mythread_thread_opt_t stream_opt;
    volatile mythread_thread_opt_t batch_opt;
};

mythread_t * mythread_create(void);
myhtml_status_t mythread_init(mythread_t *mythread, const char *sem_prefix, size_t thread_count);
void mythread_clean(mythread_t *mythread);
mythread_t * mythread_destroy(mythread_t *mythread, mybool_t self_destroy);

mythread_id_t myhread_create_stream(mythread_t *mythread, mythread_f func, myhtml_status_t *status);
mythread_id_t myhread_create_batch(mythread_t *mythread, mythread_f func, myhtml_status_t *status, size_t count);

void mythread_stream_pause_all(mythread_t *mythread);
void mythread_batch_pause_all(mythread_t *mythread);

void mythread_wait_all(mythread_t *mythread);
void mythread_resume_all(mythread_t *mythread);

void mythread_stream_quit_all(mythread_t *mythread);
void mythread_batch_quit_all(mythread_t *mythread);

// queue
struct mythread_queue_node {
    myhtml_token_node_t *token;
    myhtml_tree_t *tree;
    
    size_t begin;
    size_t length;
    
    const char* text;
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

mythread_queue_t * mythread_queue_create(size_t size, myhtml_status_t *status);
void mythread_queue_clean(mythread_queue_t* queue);
mythread_queue_t * mythread_queue_destroy(mythread_queue_t* token);

void mythread_queue_node_clean(mythread_queue_node_t* qnode);

mythread_queue_node_t * mythread_queue_get_prev_node(mythread_queue_t* queue);
mythread_queue_node_t * mythread_queue_get_current_node(mythread_queue_t* queue);
mythread_queue_node_t * mythread_queue_node_malloc(mythread_queue_t* queue, const char* text, size_t begin, myhtml_status_t *status);


#endif /* defined(__myhtml__myhtml_thread__) */


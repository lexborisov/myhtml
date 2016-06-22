/*
 Copyright (C) 2015-2016 Alexander Borisov
 
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

#ifndef MyHTML_THREAD_H
#define MyHTML_THREAD_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <myhtml/myosi.h>

#ifndef MyHTML_BUILD_WITHOUT_THREADS

#if !defined(IS_OS_WINDOWS)
#   include <pthread.h>
#   include <semaphore.h>
#endif

#include <time.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <errno.h>

#include <myhtml/myhtml.h>
#include <myhtml/tree.h>
#include <myhtml/mystring.h>

#endif /* MyHTML_BUILD_WITHOUT_THREADS */

#define MyTHREAD_SEM_NAME "myhtml"


#ifdef MyHTML_BUILD_WITHOUT_THREADS

struct mythread {
    int sys_last_error;
};

#else /* MyHTML_BUILD_WITHOUT_THREADS */

void mythread_function_stream(void *arg);
void mythread_function_batch(void *arg);

// thread
struct mythread_context {
    mythread_id_t id;
    
#if defined(IS_OS_WINDOWS)
    HANDLE mutex;
#else
    pthread_mutex_t *mutex;
#endif
    
    size_t sem_name_size;
    
    mythread_f func;
    
    volatile size_t t_count;
    volatile mythread_thread_opt_t opt;
    
    mythread_t *mythread;
};

struct mythread_list {
#if defined(IS_OS_WINDOWS)
    HANDLE pth;
#else
    pthread_t pth;
#endif
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
    
    mythread_queue_list_t *queue_list;
    
    char  *sem_prefix;
    size_t sem_prefix_length;
    
#if !defined(IS_OS_WINDOWS)
    pthread_attr_t *attr;
#endif
    
    int sys_last_error;
    
    mythread_id_t batch_first_id;
    mythread_id_t batch_count;
    
    volatile mythread_thread_opt_t stream_opt;
    volatile mythread_thread_opt_t batch_opt;
};

mythread_id_t myhread_create_stream(mythread_t *mythread, mythread_f func, myhtml_status_t *status);
mythread_id_t myhread_create_batch(mythread_t *mythread, mythread_f func, myhtml_status_t *status, size_t count);

void myhtml_thread_nanosleep(const struct timespec *tomeout);
    
#endif

mythread_t * mythread_create(void);
myhtml_status_t mythread_init(mythread_t *mythread, const char *sem_prefix, size_t thread_count, size_t queue_size);
void mythread_clean(mythread_t *mythread);
mythread_t * mythread_destroy(mythread_t *mythread, bool self_destroy);

void mythread_stream_quit_all(mythread_t *mythread);
void mythread_batch_quit_all(mythread_t *mythread);

void mythread_stream_stop_all(mythread_t *mythread);
void mythread_batch_stop_all(mythread_t *mythread);

void mythread_stop_all(mythread_t *mythread);
void mythread_wait_all_for_done(mythread_t *mythread);
void mythread_resume_all(mythread_t *mythread);
void mythread_suspend_all(mythread_t *mythread);

// queue
struct mythread_queue_node {
    mythread_queue_node_t *prev;
    myhtml_token_node_t *token;
    myhtml_tree_t *tree;
};

struct mythread_queue_thread_param {
    volatile size_t use;
};

struct mythread_queue_list_entry {
    mythread_queue_list_entry_t *next;
    mythread_queue_list_entry_t *prev;
    mythread_queue_t *queue;
    mythread_queue_thread_param_t *thread_param;
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

mythread_queue_t * mythread_queue_create(size_t size, myhtml_status_t *status);
void mythread_queue_clean(mythread_queue_t* queue);
mythread_queue_t * mythread_queue_destroy(mythread_queue_t* token);

void mythread_queue_node_clean(mythread_queue_node_t* qnode);

size_t mythread_queue_count_used_node(mythread_queue_t* queue);
mythread_queue_node_t * mythread_queue_get_first_node(mythread_queue_t* queue);
mythread_queue_node_t * mythread_queue_get_prev_node(mythread_queue_node_t* qnode);
mythread_queue_node_t * mythread_queue_get_current_node(mythread_queue_t* queue);
mythread_queue_node_t * mythread_queue_node_malloc(mythread_t *mythread, mythread_queue_t* queue, myhtml_status_t *status);
mythread_queue_node_t * mythread_queue_node_malloc_limit(mythread_t *mythread, mythread_queue_t* queue, size_t limit, myhtml_status_t *status);

#ifndef MyHTML_BUILD_WITHOUT_THREADS

mythread_queue_list_t * mythread_queue_list_create(mythread_t *mythread, myhtml_status_t *status);
mythread_queue_list_entry_t * mythread_queue_list_entry_push(mythread_t *mythread, mythread_queue_t *queue, myhtml_status_t *status);
mythread_queue_list_entry_t * mythread_queue_list_entry_delete(mythread_t *mythread, mythread_queue_list_entry_t *entry, bool destroy_queue);
void mythread_queue_list_entry_clean(mythread_t *mythread, mythread_queue_list_entry_t *entry);
void mythread_queue_list_entry_wait_for_done(mythread_t *mythread, mythread_queue_list_entry_t *entry);
    
mythread_queue_node_t * mythread_queue_node_malloc_round(mythread_t *mythread, mythread_queue_list_entry_t *entry, myhtml_status_t *status);

#endif /* MyHTML_BUILD_WITHOUT_THREADS */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined(__myhtml__myhtml_thread__) */


//
//  myhtml_thread.h
//  myhtml
//
//  Created by Alexander Borisov on 02.10.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#ifndef __myhtml__myhtml_thread__
#define __myhtml__myhtml_thread__

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

#include "myosi.h"
#include "myhtml.h"
#include "myhtml_tree.h"

#define MyHTML_THREAD_MASTER_ID 0
#define MyHTML_THREAD_STREAM_ID 1
#define MyHTML_THREAD_WORKERS_BEGIN_ID 2

#define MyHTML_THREAD_SEM_NAME "/myhtmlthread"
#define MyHTML_THREAD_SEM_MAX_NAME 128

#define MyHTML_THREAD_MIN_THREADS 1
#define MyHTML_THREAD_MAX_THREADS 100

#define myhtml_thread(__thread__, __idx__, __attr__) __thread__->pth_list[__idx__].__attr__
#define myhtml_thread_master(__thread__, __attr__) __thread__->pth_list[MyHTML_THREAD_MASTER_ID].__attr__
#define myhtml_thread_stream(__thread__, __attr__) __thread__->pth_list[MyHTML_THREAD_STREAM_ID].__attr__

#define myhtml_thread_get(__thread__, __idx__, __attr__) __thread__->pth_list[__idx__].data.__attr__
#define myhtml_thread_set(__thread__, __idx__, __attr__) myhtml_thread_get(__thread__, __idx__, __attr__)

#define myhtml_thread_master_get(__thread__, __attr__) __thread__->pth_list[MyHTML_THREAD_MASTER_ID].data.__attr__
#define myhtml_thread_master_set(__thread__, __attr__) myhtml_thread_master_get(__thread__, __attr__)

#define myhtml_thread_stream_get(__thread__, __attr__) __thread__->pth_list[MyHTML_THREAD_STREAM_ID].data.__attr__
#define myhtml_thread_stream_set(__thread__, __attr__) myhtml_thread_stream_get(__thread__, __attr__)

struct myhtml_thread_context {
    size_t id;
    myhtml_t* myhtml;
    volatile size_t queue_pos;
    
    sem_t* sem;
    volatile mybool_t is_done;
    
    myhtml_thread_f func;
};

struct myhtml_thread_list {
    pthread_t pth;
    myhtml_thread_context_t data;
};

struct myhtml_thread {
    myhtml_thread_list_t* pth_list;
    size_t pth_list_length;
    size_t thread_next;
    
    pthread_attr_t attr;
    
    // for say exit all thread after job done
    volatile mybool_t is_quit;
    // for a global variable
    pthread_mutex_t global_mutex;
};

void myhtml_thread_init(myhtml_t* myhtml, const char* sem_prefix, size_t sem_prefix_length, size_t thread_count,
                        myhtml_thread_f stream_func, myhtml_thread_f worker_func);
void myhtml_thread_clean(myhtml_thread_t* thread, size_t start_pos);
void myhtml_thread_destroy(myhtml_t* myhtml);

void myhtml_thread_wait_all_for_done(myhtml_t* myhtml);

#endif /* defined(__myhtml__myhtml_thread__) */


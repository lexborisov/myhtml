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

#ifndef MyCORE_THREAD_H
#define MyCORE_THREAD_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mycore/myosi.h"
#include "mycore/mystring.h"

#ifdef MyCORE_BUILD_WITHOUT_THREADS

struct mythread {
    int sys_last_error;
};

#else
/* functions */
typedef void (*mythread_callback_before_entry_join_f)(mythread_t* mythread, mythread_entry_t* entry, void* ctx);
typedef void * (*mythread_process_f)(void* arg);
typedef void (*mythread_work_f)(mythread_id_t thread_id, void* arg);
    
void * mythread_function_queue_stream(void *arg);
void * mythread_function_queue_batch(void *arg);
void * mythread_function(void *arg);

enum mythread_thread_opt {
    MyTHREAD_OPT_UNDEF = 0x00,
    MyTHREAD_OPT_WAIT  = 0x01,
    MyTHREAD_OPT_QUIT  = 0x02,
    MyTHREAD_OPT_STOP  = 0x04,
    MyTHREAD_OPT_DONE  = 0x08
}
typedef mythread_thread_opt_t;

enum mythread_type {
    MyTHREAD_TYPE_STREAM = 0x00,
    MyTHREAD_TYPE_BATCH  = 0x01
}
typedef mythread_type_t;

// thread
struct mythread_context {
    mythread_id_t id;
    mythread_work_f func;
    
    volatile size_t t_count;
    volatile mythread_thread_opt_t opt;
    
    mystatus_t status;
    
    void* mutex;
    void* timespec;
    mythread_t* mythread;
};

struct mythread_entry {
    void* thread;
    
    mythread_context_t context;
    mythread_process_f process_func;
};

struct mythread {
    mythread_entry_t *entries;
    size_t entries_length;
    size_t entries_size;
    size_t id_increase;
    
    void* context;
    void* attr;
    void* timespec;
    
    int sys_last_error;
    
    mythread_type_t type;
    volatile mythread_thread_opt_t opt;
};

mythread_t * mythread_create(void);
mystatus_t mythread_init(mythread_t *mythread, mythread_type_t type, size_t threads_count, size_t id_increase);
void mythread_clean(mythread_t *mythread);
mythread_t * mythread_destroy(mythread_t *mythread, mythread_callback_before_entry_join_f before_join, void* ctx, bool self_destroy);

mythread_id_t myhread_increase_id_by_entry_id(mythread_t* mythread, mythread_id_t thread_id);

/* set for all threads */
mystatus_t mythread_join(mythread_t *mythread, mythread_callback_before_entry_join_f before_join, void* ctx);
mystatus_t mythread_quit(mythread_t *mythread, mythread_callback_before_entry_join_f before_join, void* ctx);
mystatus_t mythread_stop(mythread_t *mythread);
mystatus_t mythread_resume(mythread_t *mythread, mythread_thread_opt_t send_opt);
mystatus_t mythread_suspend(mythread_t *mythread);
mystatus_t mythread_check_status(mythread_t *mythread);

mythread_thread_opt_t mythread_option(mythread_t *mythread);
void mythread_option_set(mythread_t *mythread, mythread_thread_opt_t opt);

/* Entries */
mystatus_t myhread_entry_create(mythread_t *mythread, mythread_process_f process_func, mythread_work_f func, mythread_thread_opt_t opt);

mystatus_t mythread_entry_join(mythread_entry_t* entry, mythread_callback_before_entry_join_f before_join, void* ctx);
mystatus_t mythread_entry_quit(mythread_entry_t* entry, mythread_callback_before_entry_join_f before_join, void* ctx);
mystatus_t mythread_entry_stop(mythread_entry_t* entry);
mystatus_t mythread_entry_resume(mythread_entry_t* entry, mythread_thread_opt_t send_opt);
mystatus_t mythread_entry_suspend(mythread_entry_t* entry);
mystatus_t mythread_entry_status(mythread_entry_t* entry);
mythread_t * mythread_entry_mythread(mythread_entry_t* entry);

/* API for ports */
void * mythread_thread_create(mythread_t *mythread, mythread_process_f process_func, void* ctx);
mystatus_t mythread_thread_join(mythread_t *mythread, void* thread);
mystatus_t mythread_thread_cancel(mythread_t *mythread, void* thread);
mystatus_t mythread_thread_destroy(mythread_t *mythread, void* thread);

void * mythread_thread_attr_init(mythread_t *mythread);
void mythread_thread_attr_clean(mythread_t *mythread, void* attr);
void mythread_thread_attr_destroy(mythread_t *mythread, void* attr);

void * mythread_mutex_create(mythread_t *mythread);
mystatus_t mythread_mutex_post(mythread_t *mythread, void* mutex);
mystatus_t mythread_mutex_wait(mythread_t *mythread, void* mutex);
void mythread_mutex_close(mythread_t *mythread, void* mutex);

void * mythread_nanosleep_create(mythread_t* mythread);
void mythread_nanosleep_clean(void* timespec);
void mythread_nanosleep_destroy(void* timespec);
mystatus_t mythread_nanosleep_sleep(void* timespec);

/* callback */
void mythread_callback_quit(mythread_t* mythread, mythread_entry_t* entry, void* ctx);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyCORE_THREAD_H */

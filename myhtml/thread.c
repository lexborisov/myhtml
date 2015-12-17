//
//  myhtml_thread.c
//  myhtml
//
//  Created by Alexander Borisov on 02.10.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#include "thread.h"

// private functions
void myhtml_thread_create_sem(myhtml_thread_t* thread, const char* sem_prefix, size_t sem_prefix_length, myhtml_thread_index_t thr_idx);
void myhtml_thread_create_stream(myhtml_t* myhtml, size_t ptr_id, const char* sem_prefix, size_t sem_prefix_length, myhtml_thread_f func);
void myhtml_thread_create_master(myhtml_t* myhtml, const char* sem_prefix, size_t sem_prefix_length);
void myhtml_thread_create_worker(myhtml_t* myhtml, myhtml_thread_index_t thr_idx, const char* sem_prefix, size_t sem_prefix_length, myhtml_thread_f func);

void myhtml_thread_worker_function(void* arg);
void myhtml_thread_master_function(void* arg);
void myhtml_thread_stream_function(void* arg);

// global functions
void myhtml_thread_init(myhtml_t* myhtml, const char* sem_prefix, size_t sem_prefix_length, size_t thread_count,
                        myhtml_thread_f stream_func, myhtml_thread_f worker_func, myhtml_thread_f index_func)
{
    myhtml_thread_t* thread = (myhtml_thread_t*)mymalloc(sizeof(myhtml_thread_t));
    myhtml->thread = thread;
    
    if(thread_count > MyHTML_THREAD_MAX_THREADS)
        thread_count = MyHTML_THREAD_MAX_THREADS;
    
    thread->pth_list_length = thread_count + MyHTML_THREAD_WORKERS_BEGIN_ID;
    thread->pth_list = (myhtml_thread_list_t*)mymalloc(sizeof(myhtml_thread_list_t) * thread->pth_list_length);
    
    thread->is_quit      = myfalse;
    thread->thread_next  = myhtml->queue->nodes_length; // queue start
    
    // mutex
    pthread_mutex_init(&thread->global_mutex, NULL);
    
    // pthreads attributes
    pthread_attr_init(&thread->attr);
    pthread_attr_setdetachstate(&thread->attr, PTHREAD_CREATE_JOINABLE);
    
    size_t i;
    for(i = MyHTML_THREAD_WORKERS_BEGIN_ID; i < thread->pth_list_length; i++) {
        myhtml_thread_create_worker(myhtml, i, sem_prefix, sem_prefix_length, worker_func);
    }
    
    myhtml_thread_create_master(myhtml, sem_prefix, sem_prefix_length);
    myhtml_thread_create_stream(myhtml, MyHTML_THREAD_STREAM_ID, sem_prefix, sem_prefix_length, stream_func);
    myhtml_thread_create_stream(myhtml, MyHTML_THREAD_INDEX_ID , sem_prefix, sem_prefix_length, index_func);
}

void myhtml_thread_clean(myhtml_thread_t* thread, size_t start_pos)
{
    myhtml_thread_master_set(thread, queue_pos) = start_pos;
    myhtml_thread_stream_set(thread, queue_pos) = start_pos;
}

void myhtml_thread_destroy(myhtml_t* myhtml)
{
    myhtml_thread_t* thread = myhtml->thread;
    
    if(thread == NULL)
        return;
    
    if(thread->pth_list) {
        thread->is_quit = mytrue;
        
        myhtml_thread_master_set(thread, is_done) = myfalse;
        myhtml_thread_stream_set(thread, is_done) = myfalse;
        
        // semaphores
        sem_post(myhtml_thread_master_get(thread, sem));
        sem_post(myhtml_thread_stream_get(thread, sem));
        
        // wait all threads
        size_t i;
        for(i = 0; i < thread->pth_list_length; i++) {
            pthread_join(myhtml_thread(thread, i, pth), NULL);
            sem_close(myhtml_thread_get(thread, i, sem));
        }
        
        free(thread->pth_list);
    }
    
    pthread_attr_destroy(&thread->attr);
    pthread_mutex_destroy(&thread->global_mutex);
    
    free(thread);
    
    myhtml->thread = NULL;
}

void myhtml_thread_wait_all_for_done(myhtml_t* myhtml)
{
    myhtml_thread_t* thread = myhtml->thread;
    
    //const struct timespec tomeout = {0, 4000};
    volatile size_t counter = 0;
    size_t i = 0;
    
    for(i = MyHTML_THREAD_MASTER_ID; i < MyHTML_THREAD_WORKERS_BEGIN_ID; i++)
    {
        while(myhtml_thread_get(thread, i, queue_pos) < myhtml->queue->nodes_length) {
            counter++;
            
            if(counter > 100) {
                counter = 0;
            }
        }
    }
    
    for(i = MyHTML_THREAD_WORKERS_BEGIN_ID; i < thread->pth_list_length; i++)
    {
        while(myhtml_thread_get(thread, i, queue_pos)) {
            counter++;
            
            if(counter > 100) {
                counter = 0;
            }
        }
    }
    
    mh_thread_master_done(mytrue);
    mh_thread_stream_done(mytrue);
}

void myhtml_thread_stream_function(void* arg)
{
    myhtml_thread_context_t* ctx = (myhtml_thread_context_t*)arg;
    myhtml_t* myhtml = ctx->myhtml;
    myhtml_thread_t* thread = myhtml->thread;
    
    const struct timespec tomeout = {0, 4000};
    volatile size_t counter = 0;
    
    sem_wait(ctx->sem);
    
    //TODO: nanosleep? must be removed
    do {
        while(ctx->queue_pos >= myhtml->queue->nodes_length)
        {
            if(thread->is_quit)
                return;
            
            if(ctx->is_done) {
                sem_wait(ctx->sem);
            }
            
            counter++;
            if(counter > 100) {
                nanosleep(&tomeout, NULL);
                counter = 0;
            }
        }
        
        if(mh_queue_get(ctx->queue_pos, is_system) == myfalse) {
            ctx->func(mh_queue_get(ctx->queue_pos, myhtml_tree), ctx->queue_pos, mh_queue_get(ctx->queue_pos, token));
        }
        
        // code here
        ctx->queue_pos++;
    }
    while (1);
}

void myhtml_thread_master_function(void* arg)
{
    myhtml_thread_context_t* ctx = (myhtml_thread_context_t*)arg;
    myhtml_t* myhtml = ctx->myhtml;
    myhtml_thread_t* thread = myhtml->thread;
    
    mybool_t loop = mytrue;
    const struct timespec tomeout = {0, 4000};
    
    sem_wait(myhtml_thread_master_get(thread, sem));
    
    size_t i;
    for(i = MyHTML_THREAD_WORKERS_BEGIN_ID; i < thread->pth_list_length; i++) {
        myhtml_thread_set(thread, i, is_done) = myfalse;
        sem_post(myhtml_thread_get(thread, i, sem));
    }
    
    //TODO: nanosleep, not sure it's right
    do {
        volatile size_t counter = 0;
        
        while(ctx->queue_pos >= myhtml->queue->nodes_length)
        {
            if(thread->is_quit) {
                loop = myfalse;
                break;
            }
            
            if(myhtml_thread_master_get(thread, is_done)) {
                for(i = 1; i < thread->pth_list_length; i++) {
                    myhtml_thread_set(thread, i, is_done) = mytrue;
                }
                
                sem_wait(myhtml_thread_master_get(thread, sem));
                
                for(i = 1; i < thread->pth_list_length; i++) {
                    myhtml_thread_set(thread, i, is_done) = myfalse;
                    sem_post(myhtml_thread_get(thread, i, sem));
                }
            }
            
            counter++;
            
            if(counter > 100) {
                nanosleep(&tomeout, NULL);
                counter = 0;
            }
        }
        
        if(loop) {
            while(myhtml_thread_get(thread, thread->thread_next, queue_pos)) {
                thread->thread_next++;
                
                if(thread->thread_next == thread->pth_list_length)
                    thread->thread_next = 1;
            }
            
            myhtml_thread_set(thread, thread->thread_next, queue_pos) = ctx->queue_pos;
            ctx->queue_pos++;
        }
    }
    while (loop);
    
    for(i = MyHTML_THREAD_WORKERS_BEGIN_ID; i < thread->pth_list_length; i++) {
        mh_queue_last(is_system) = mytrue;
        mh_queue_last(opt) = MyHTML_QUEUE_OPT_QUIT;
        
        volatile size_t counter = 0;
        
        while(myhtml_thread_get(thread, i, queue_pos)) {
            counter++;
            
            if(counter > 100) {
                nanosleep(&tomeout, NULL);
                counter = 0;
            }
        }
        
        myhtml_thread_set(thread, i, queue_pos) = myhtml->queue->nodes_length;
        sem_post(myhtml_thread_get(thread, i, sem));
        
        myhtml_queue_node_malloc(myhtml->queue, 0, 0, mytrue, MyHTML_QUEUE_OPT_QUIT, 0);
    }
}

void myhtml_thread_worker_function(void* arg)
{
    myhtml_thread_context_t* ctx = (myhtml_thread_context_t*)arg;
    myhtml_t* myhtml = ctx->myhtml;
    //myhtml_thread_t* thread = myhtml->thread;
    
    const struct timespec tomeout = {0, 4000};
    
    while (1)
    {
        volatile size_t counter = 0;
        
        while(ctx->queue_pos == 0) {
            counter++;
            
            if(counter > 100) {
                nanosleep(&tomeout, NULL);
                
                if(ctx->is_done) {
                    sem_wait(ctx->sem);
                }
                
                counter = 0;
            }
        }
        
        if(mh_queue_get(ctx->queue_pos, is_system)) {
            if(mh_queue_get(ctx->queue_pos, opt) & MyHTML_QUEUE_OPT_QUIT) {
                break;
            }
        }
        else {
            myhtml_token_node_t* token = mh_queue_get(ctx->queue_pos, token);
            ctx->func(mh_queue_get(ctx->queue_pos, myhtml_tree), ctx->queue_pos, token);
        }
        
        ctx->queue_pos = 0;
    }
}

void myhtml_thread_create_sem(myhtml_thread_t* thread, const char* sem_prefix, size_t sem_prefix_length, myhtml_thread_index_t thr_idx)
{
    char text_sem[MyHTML_THREAD_SEM_MAX_NAME] = {0};
    size_t self_sem_name_size = strlen(MyHTML_THREAD_SEM_NAME) + 7;
    
    if((sem_prefix_length + self_sem_name_size) >= MyHTML_THREAD_SEM_MAX_NAME)
    sem_prefix_length = MyHTML_THREAD_SEM_MAX_NAME - self_sem_name_size;
    
    sprintf(text_sem, "%s%.*s%lu", MyHTML_THREAD_SEM_NAME, (int)sem_prefix_length, sem_prefix, thr_idx);
    
    myhtml_thread_set(thread, thr_idx, sem) = sem_open(text_sem, O_CREAT, S_IRUSR|S_IWUSR, 0);
    
    if(myhtml_thread_get(thread, thr_idx, sem) == SEM_FAILED) {
        fprintf(stderr, "Sem open error\n");
    }
}

void myhtml_thread_create_stream(myhtml_t* myhtml, size_t ptr_id, const char* sem_prefix, size_t sem_prefix_length, myhtml_thread_f func)
{
    myhtml_thread_set(myhtml->thread, ptr_id, id       ) = ptr_id;
    myhtml_thread_set(myhtml->thread, ptr_id, myhtml   ) = myhtml;
    myhtml_thread_set(myhtml->thread, ptr_id, queue_pos) = myhtml->queue->nodes_length;
    myhtml_thread_set(myhtml->thread, ptr_id, is_done  ) = mytrue;
    myhtml_thread_set(myhtml->thread, ptr_id, func     ) = func;
    
    myhtml_thread_create_sem(myhtml->thread, sem_prefix, sem_prefix_length, ptr_id);
    
    pthread_create(&myhtml_thread(myhtml->thread, ptr_id, pth), &myhtml->thread->attr,
                   (void*)myhtml_thread_stream_function,
                   (void*)(&myhtml_thread(myhtml->thread, ptr_id, data)));
}

void myhtml_thread_create_master(myhtml_t* myhtml, const char* sem_prefix, size_t sem_prefix_length)
{
    myhtml_thread_master_set(myhtml->thread, id       ) = MyHTML_THREAD_MASTER_ID;
    myhtml_thread_master_set(myhtml->thread, myhtml   ) = myhtml;
    myhtml_thread_master_set(myhtml->thread, queue_pos) = myhtml->queue->nodes_length;
    myhtml_thread_master_set(myhtml->thread, is_done  ) = mytrue;
    myhtml_thread_master_set(myhtml->thread, func     ) = NULL;
    
    myhtml_thread_create_sem(myhtml->thread, sem_prefix, sem_prefix_length, MyHTML_THREAD_MASTER_ID);
    
    pthread_create(&myhtml_thread_master(myhtml->thread, pth), &myhtml->thread->attr,
                   (void*)myhtml_thread_master_function,
                   (void*)(&myhtml_thread_master(myhtml->thread, data)));
}

void myhtml_thread_create_worker(myhtml_t* myhtml, myhtml_thread_index_t thr_idx, const char* sem_prefix, size_t sem_prefix_length, myhtml_thread_f func)
{
    myhtml_thread_set(myhtml->thread, thr_idx, id       ) = thr_idx;
    myhtml_thread_set(myhtml->thread, thr_idx, myhtml   ) = myhtml;
    myhtml_thread_set(myhtml->thread, thr_idx, queue_pos) = 0;
    myhtml_thread_set(myhtml->thread, thr_idx, is_done  ) = mytrue;
    myhtml_thread_set(myhtml->thread, thr_idx, func     ) = func;
    
    myhtml_thread_create_sem(myhtml->thread, sem_prefix, sem_prefix_length, thr_idx);
    
    pthread_create(&myhtml_thread(myhtml->thread, thr_idx, pth), &myhtml->thread->attr,
                   (void*)myhtml_thread_worker_function,
                   (void*)(&myhtml_thread(myhtml->thread, thr_idx, data)));
}



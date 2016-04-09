/*
 Copyright 2015-2016 Alexander Borisov
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#include "myhtml/thread.h"

#ifndef MyHTML_BUILD_WITHOUT_THREADS

#if defined(IS_OS_WINDOWS)
/***********************************************************************************
 *
 * For Windows
 *
 ***********************************************************************************/
myhtml_status_t myhtml_thread_create(mythread_t *mythread, mythread_list_t *thr, void *work_func)
{
    thr->pth = CreateThread(NULL,                   // default security attributes
                            0,                      // use default stack size
                            work_func,              // thread function name
                            &thr->data,             // argument to thread function
                            0,                      // use default creation flags
                            NULL);                  // returns the thread identifier

    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_join(mythread_t *mythread, mythread_list_t *thr)
{
    WaitForSingleObject(thr->pth, INFINITE);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_cancel(mythread_t *mythread, mythread_list_t *thr)
{
    TerminateThread(thr->pth, 0);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_attr_init(mythread_t *mythread)
{
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_attr_clean(mythread_t *mythread)
{
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_attr_destroy(mythread_t *mythread)
{
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_create(mythread_t *mythread, mythread_context_t *ctx, size_t prefix_id)
{
    ctx->mutex = CreateSemaphore(NULL, 0, 1, NULL);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_post(mythread_t *mythread, mythread_context_t *ctx)
{
    ReleaseSemaphore(ctx->mutex, 1, NULL);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_wait(mythread_t *mythread, mythread_context_t *ctx)
{
    WaitForSingleObject(ctx->mutex, INFINITE);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_try_wait(mythread_t *mythread, mythread_context_t *ctx)
{
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_close(mythread_t *mythread, mythread_context_t *ctx)
{
    CloseHandle(ctx->mutex);
    
    return MyHTML_STATUS_OK;
}

void myhtml_thread_nanosleep(const struct timespec *tomeout)
{
    Sleep(0);
}

#else /* defined(IS_OS_WINDOWS) */
/***********************************************************************************
 *
 * For all unix system. POSIX pthread
 *
 ***********************************************************************************/

myhtml_status_t myhtml_thread_create(mythread_t *mythread, mythread_list_t *thr, void *work_func)
{
    pthread_create(&thr->pth, mythread->attr,
                   work_func,
                   (void*)(&thr->data));
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_join(mythread_t *mythread, mythread_list_t *thr)
{
    pthread_join(thr->pth, NULL);
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_cancel(mythread_t *mythread, mythread_list_t *thr)
{
    pthread_cancel(thr->pth);
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_attr_init(mythread_t *mythread)
{
    mythread->attr = (pthread_attr_t*)calloc(1, sizeof(pthread_attr_t));
    
    if(mythread->attr == NULL)
        return MyHTML_STATUS_THREAD_ERROR_ATTR_MALLOC;
    
    mythread->sys_last_error = pthread_attr_init(mythread->attr);
    if(mythread->sys_last_error)
        return MyHTML_STATUS_THREAD_ERROR_ATTR_INIT;
    
    mythread->sys_last_error = pthread_attr_setdetachstate(mythread->attr, PTHREAD_CREATE_JOINABLE);
    if(mythread->sys_last_error)
        return MyHTML_STATUS_THREAD_ERROR_ATTR_SET;
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_attr_clean(mythread_t *mythread)
{
    mythread->attr = NULL;
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_thread_attr_destroy(mythread_t *mythread)
{
    if(mythread->attr) {
        mythread->sys_last_error = pthread_attr_destroy(mythread->attr);
        
        free(mythread->attr);
        mythread->attr = NULL;
        
        if(mythread->sys_last_error)
            return MyHTML_STATUS_THREAD_ERROR_ATTR_DESTROY;
    }
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_create(mythread_t *mythread, mythread_context_t *ctx, size_t prefix_id)
{
    ctx->mutex = (pthread_mutex_t*)calloc(1, sizeof(pthread_mutex_t));
    
    if(ctx->mutex == NULL)
        return MyHTML_STATUS_THREAD_ERROR_MUTEX_MALLOC;
    
    if(pthread_mutex_init(ctx->mutex, NULL)) {
        mythread->sys_last_error = errno;
        return MyHTML_STATUS_THREAD_ERROR_MUTEX_INIT;
    }
    
    if(pthread_mutex_lock(ctx->mutex)) {
        mythread->sys_last_error = errno;
        return MyHTML_STATUS_THREAD_ERROR_MUTEX_LOCK;
    }
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_post(mythread_t *mythread, mythread_context_t *ctx)
{
    if(pthread_mutex_unlock(ctx->mutex)) {
        mythread->sys_last_error = errno;
        return MyHTML_STATUS_THREAD_ERROR_MUTEX_UNLOCK;
    }
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_wait(mythread_t *mythread, mythread_context_t *ctx)
{
    if(pthread_mutex_lock(ctx->mutex)) {
        mythread->sys_last_error = errno;
        return MyHTML_STATUS_THREAD_ERROR_MUTEX_LOCK;
    }
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_try_wait(mythread_t *mythread, mythread_context_t *ctx)
{
    if(pthread_mutex_trylock(ctx->mutex)) {
        mythread->sys_last_error = errno;
        return MyHTML_STATUS_THREAD_ERROR_MUTEX_LOCK;
    }
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_hread_mutex_close(mythread_t *mythread, mythread_context_t *ctx)
{
    if(ctx->mutex) {
        pthread_mutex_destroy(ctx->mutex);
        free(ctx->mutex);
        
        ctx->mutex = NULL;
    }
    
    return MyHTML_STATUS_OK;
}

void myhtml_thread_nanosleep(const struct timespec *tomeout)
{
    nanosleep(tomeout, NULL);
}

#endif /* !defined(IS_OS_WINDOWS) */
#endif /* MyHTML_BUILD_WITHOUT_THREADS */

/*
 *
 * MyTHREAD logic
 *
 */

mythread_t * mythread_create(void)
{
    return calloc(1, sizeof(mythread_t));
}

#ifdef MyHTML_BUILD_WITHOUT_THREADS

myhtml_status_t mythread_init(mythread_t *mythread, const char *sem_prefix, size_t thread_count, size_t queue_size)
{
    return MyHTML_STATUS_OK;
}

#else /* MyHTML_BUILD_WITHOUT_THREADS */

myhtml_status_t mythread_init(mythread_t *mythread, const char *sem_prefix, size_t thread_count, size_t queue_size)
{
    mythread->batch_count    = 0;
    mythread->batch_first_id = 0;
    mythread->stream_opt     = MyTHREAD_OPT_STOP;
    mythread->batch_opt      = MyTHREAD_OPT_STOP;
    
    if(thread_count)
    {
        myhtml_status_t status = myhtml_thread_attr_init(mythread);
        if(status)
            return status;
        
        mythread->pth_list_root   = 1;
        mythread->pth_list_length = 1;
        mythread->pth_list_size   = thread_count + 1;
        mythread->pth_list        = (mythread_list_t*)calloc(mythread->pth_list_size, sizeof(mythread_list_t));
        
        if(mythread->pth_list == NULL)
            return MyHTML_STATUS_THREAD_ERROR_LIST_INIT;
    }
    else {
        myhtml_thread_attr_clean(mythread);
        
        mythread->sys_last_error  = 0;
        mythread->pth_list_root   = 1;
        mythread->pth_list_length = 1;
        mythread->pth_list_size   = 0;
        mythread->pth_list        = NULL;
    }
    
    myhtml_status_t status;
    mythread->queue_list = mythread_queue_list_create(mythread, 1024, &status);
    
    if(mythread->queue_list == NULL)
        return status;
    
    if(sem_prefix)
    {
        mythread->sem_prefix_length = strlen(sem_prefix);
        
        if(mythread->sem_prefix_length) {
            mythread->sem_prefix = calloc((mythread->sem_prefix_length + 1), sizeof(char));
            
            if(mythread->sem_prefix == NULL) {
                mythread->sem_prefix_length = 0;
                return MyHTML_STATUS_THREAD_ERROR_SEM_PREFIX_MALLOC;
            }
            
            myhtml_string_raw_copy(mythread->sem_prefix, sem_prefix, mythread->sem_prefix_length);
        }
    }
    
    return MyHTML_STATUS_OK;
}

#endif /* MyHTML_BUILD_WITHOUT_THREADS */

void mythread_clean(mythread_t *mythread)
{
    mythread->sys_last_error = 0;
}

mythread_t * mythread_destroy(mythread_t *mythread, bool self_destroy)
{
    if(mythread == NULL)
        return NULL;
    
#ifndef MyHTML_BUILD_WITHOUT_THREADS
    
    myhtml_thread_attr_destroy(mythread);
    
    if(mythread->pth_list) {
        mythread_resume_all(mythread);
        mythread_stream_quit_all(mythread);
        mythread_batch_quit_all(mythread);
        mythread_wait_all_for_done(mythread);
        
        for (size_t i = mythread->pth_list_root; i < mythread->pth_list_length; i++)
        {
            myhtml_thread_join(mythread, &mythread->pth_list[i]);
        }
        
        free(mythread->pth_list);
        mythread->pth_list = NULL;
    }
    
    if(mythread->queue_list) {
        free(mythread->queue_list);
    }
    
    if(mythread->sem_prefix) {
        free(mythread->sem_prefix);
        
        mythread->sem_prefix = NULL;
        mythread->sem_prefix_length = 0;
    }
    
#endif /* MyHTML_BUILD_WITHOUT_THREADS */
    
    if(self_destroy) {
        free(mythread);
        return NULL;
    }
    
    return mythread;
}

#ifndef MyHTML_BUILD_WITHOUT_THREADS

mythread_id_t _myhread_create_stream_raw(mythread_t *mythread, mythread_f func, void *work_func, myhtml_status_t *status, size_t total_count)
{
    mythread->sys_last_error = 0;
    
    if(status)
        *status = MyHTML_STATUS_OK;
    
    if(mythread->pth_list_length >= mythread->pth_list_size) {
        if(status)
            *status = MyHTML_STATUS_THREAD_ERROR_NO_SLOTS;
        
        return 0;
    }
    
    mythread_list_t *thr = &mythread->pth_list[mythread->pth_list_length];
    
    thr->data.mythread = mythread;
    thr->data.func     = func;
    thr->data.id       = mythread->pth_list_length;
    thr->data.t_count  = total_count;
    thr->data.opt      = MyTHREAD_OPT_STOP;
    
    myhtml_status_t m_status = myhtml_hread_mutex_create(mythread, &thr->data, 0);
    
    if(m_status != MyHTML_STATUS_OK && status) {
        *status = m_status;
        return 0;
    }
    
    m_status = myhtml_thread_create(mythread, thr, work_func);
    if(m_status != MyHTML_STATUS_OK)
        return 0;
    
    mythread->pth_list_length++;
    return thr->data.id;
}

mythread_id_t myhread_create_stream(mythread_t *mythread, mythread_f func, myhtml_status_t *status)
{
    return _myhread_create_stream_raw(mythread, func, mythread_function_stream, status, 0);
}

mythread_id_t myhread_create_batch(mythread_t *mythread, mythread_f func, myhtml_status_t *status, size_t count)
{
    if(mythread->batch_count) {
        *status = MyHTML_STATUS_THREAD_ERROR_BATCH_INIT;
        return 0;
    }
    else if((mythread->pth_list_length + count) > mythread->pth_list_size) {
        if(status)
            *status = MyHTML_STATUS_THREAD_ERROR_NO_SLOTS;
        
        return 0;
    }
    
    if(count == 0)
        count = 1;
    
    mythread->batch_first_id = 0;
    mythread->batch_count    = count;
    
    size_t start = mythread->pth_list_length;
    *status = MyHTML_STATUS_OK;
    
    bool init_first = false;
    
    for (size_t i = 0; i < count; i++)
    {
        mythread_id_t curr_id = _myhread_create_stream_raw(mythread, func, mythread_function_batch, status, i);
        
        if(init_first == false) {
            mythread->batch_first_id = curr_id;
            init_first = true;
        }
        
        if(*status)
        {
            for (size_t n = start; n < (start + i); n++)
            {
                mythread_list_t *thr = &mythread->pth_list[n];
                
                myhtml_thread_cancel(mythread, thr);
                
                myhtml_hread_mutex_post(mythread, &thr->data);
                myhtml_hread_mutex_close(mythread, &thr->data);
            }
            
            mythread->batch_first_id = 0;
            mythread->batch_count    = 0;
            
            break;
        }
    }
    
    return mythread->batch_first_id;
}

#endif /* MyHTML_BUILD_WITHOUT_THREADS */

// mythread queue functions
#ifndef MyHTML_BUILD_WITHOUT_THREADS
mythread_queue_list_t * mythread_queue_list_create(mythread_t *mythread, size_t size, myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_OK;
    
    if(size < 1024)
        size = 1024;
    
    mythread_queue_list_t* queue_list = (mythread_queue_list_t*)mycalloc(1, sizeof(mythread_queue_list_t));
    
    if(queue_list == NULL) {
        if(status)
            *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_MALLOC;
        return NULL;
    }
    
    return queue_list;
}

mythread_queue_list_entry_t * mythread_queue_list_entry_push(mythread_t *mythread, mythread_queue_t *queue, myhtml_status_t *status)
{
    mythread_queue_list_t *queue_list = mythread->queue_list;
    
    if(status)
        *status = MyHTML_STATUS_OK;
    
    mythread_queue_list_entry_t* entry = (mythread_queue_list_entry_t*)mycalloc(1, sizeof(mythread_queue_list_entry_t));
    
    if(entry == NULL) {
        if(status)
            *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_MALLOC;
        return NULL;
    }
    
    entry->thread_param = (mythread_queue_thread_param_t*)mycalloc(mythread->pth_list_size, sizeof(mythread_queue_thread_param_t));
    
    if(entry->thread_param == NULL) {
        free(entry);
        
        if(status)
            *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_MALLOC;
        return NULL;
    }
    
    size_t idx;
    for (idx = mythread->batch_first_id; idx < (mythread->batch_first_id + mythread->batch_count); idx++) {
        entry->thread_param[idx].use = mythread->pth_list[idx].data.t_count;
    }
    
    entry->queue = queue;
    
    if(mythread->stream_opt == MyTHREAD_OPT_UNDEF) {
        mythread_suspend_all(mythread);
    }
    else if(mythread->stream_opt == MyTHREAD_OPT_STOP) {
        mythread_stop_all(mythread);
    }
    
    if(queue_list->first) {
        queue_list->last->next = entry;
        entry->prev = queue_list->last;
        
        queue_list->last = entry;
    }
    else {
        queue_list->first = entry;
        queue_list->last = entry;
    }
    
    queue_list->count++;
    
    if(mythread->stream_opt != MyTHREAD_OPT_STOP)
        mythread_resume_all(mythread);
    
    return entry;
}

mythread_queue_list_entry_t * mythread_queue_list_entry_delete(mythread_t *mythread, mythread_queue_list_entry_t *entry, bool destroy_queue)
{
    mythread_queue_list_t *queue_list = mythread->queue_list;
    
    mythread_queue_list_entry_t *next = entry->next;
    mythread_queue_list_entry_t *prev = entry->prev;
    
    if(mythread->stream_opt == MyTHREAD_OPT_UNDEF) {
        mythread_suspend_all(mythread);
    }
    else if(mythread->stream_opt == MyTHREAD_OPT_STOP) {
        mythread_stop_all(mythread);
    }
    
    if(prev)
        prev->next = next;
    
    if(next)
        next->prev = prev;
    
    if(queue_list->first == entry)
        queue_list->first = next;
    
    if(queue_list->last == entry)
        queue_list->last = prev;
    
    if(mythread->stream_opt != MyTHREAD_OPT_STOP)
        mythread_resume_all(mythread);
    
    if(destroy_queue && entry->queue)
        mythread_queue_destroy(entry->queue);
    
    if(entry->thread_param)
        free(entry->thread_param);
    
    free(entry);
    
    queue_list->count--;
    
    return NULL;
}

void mythread_queue_list_entry_clean(mythread_t *mythread, mythread_queue_list_entry_t *entry)
{
    if(entry == NULL)
        return;
    
    mythread_queue_clean(entry->queue);
    
    size_t idx;
    for (idx = mythread->pth_list_root; idx < mythread->batch_first_id; idx++) {
        entry->thread_param[idx].use = 0;
    }
    
    for (idx = mythread->batch_first_id; idx < (mythread->batch_first_id + mythread->batch_count); idx++) {
        entry->thread_param[idx].use = mythread->pth_list[idx].data.t_count;
    }
}

void mythread_queue_list_entry_wait_for_done(mythread_t *mythread, mythread_queue_list_entry_t *entry)
{
    if(entry == NULL)
        return;
    
    size_t idx;
    const struct timespec tomeout = {0, 0};
    
    for (idx = mythread->pth_list_root; idx < mythread->pth_list_size; idx++) {
        mythread_queue_thread_param_t *thread_param = &entry->thread_param[ idx ];
        while(thread_param->use < entry->queue->nodes_uses) {
            myhtml_thread_nanosleep(&tomeout);
        }
    }
}

#endif /* MyHTML_BUILD_WITHOUT_THREADS */

mythread_queue_t * mythread_queue_create(size_t size, myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_OK;
    
    if(size < 4096)
        size = 4096;
    
    mythread_queue_t* queue = (mythread_queue_t*)mymalloc(sizeof(mythread_queue_t));
    
    if(queue == NULL) {
        if(status)
            *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_MALLOC;
        return NULL;
    }
    
    queue->nodes_pos_size = 512;
    queue->nodes_size     = size;
    queue->nodes          = (mythread_queue_node_t**)mycalloc(queue->nodes_pos_size, sizeof(mythread_queue_node_t*));
    
    if(queue->nodes == NULL) {
        free(queue);
        
        if(status)
            *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_NODES_MALLOC;
        return NULL;
    }
    
    mythread_queue_clean(queue);
    
    queue->nodes[queue->nodes_pos] = (mythread_queue_node_t*)mymalloc(sizeof(mythread_queue_node_t) * queue->nodes_size);
    
    if(queue->nodes[queue->nodes_pos] == NULL) {
        free(queue->nodes);
        free(queue);
        
        if(status)
            *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_NODE_MALLOC;
        return NULL;
    }
    
    return queue;
}

void mythread_queue_clean(mythread_queue_t* queue)
{
    queue->nodes_length = 0;
    queue->nodes_pos    = 0;
    queue->nodes_root   = 0;
    queue->nodes_uses   = 0;
    
    if(queue->nodes[queue->nodes_pos])
        mythread_queue_node_clean(&queue->nodes[queue->nodes_pos][queue->nodes_length]);
}

mythread_queue_t * mythread_queue_destroy(mythread_queue_t* queue)
{
    if(queue == NULL)
        return NULL;
    
    if(queue->nodes) {
        for (size_t i = 0; i <= queue->nodes_pos; i++) {
            free(queue->nodes[i]);
        }
        
        free(queue->nodes);
    }
    
    free(queue);
    
    return NULL;
}

void mythread_queue_node_clean(mythread_queue_node_t* qnode)
{
    memset(qnode, 0, sizeof(mythread_queue_node_t));
}

mythread_queue_node_t * mythread_queue_get_prev_node(mythread_queue_node_t* qnode)
{
    return qnode->prev;
}

mythread_queue_node_t * mythread_queue_get_current_node(mythread_queue_t* queue)
{
    return &queue->nodes[queue->nodes_pos][queue->nodes_length];
}

mythread_queue_node_t * mythread_queue_get_first_node(mythread_queue_t* queue)
{
    return &queue->nodes[0][0];
}

size_t mythread_queue_count_used_node(mythread_queue_t* queue)
{
    return queue->nodes_uses;
}

mythread_queue_node_t * mythread_queue_node_malloc(mythread_t *mythread, mythread_queue_t* queue, const char* text, size_t begin, myhtml_status_t *status)
{
    queue->nodes_length++;
    
    if(queue->nodes_length >= queue->nodes_size)
    {
        queue->nodes_pos++;
        
        if(queue->nodes_pos >= queue->nodes_pos_size)
        {
            mythread_wait_all_for_done(mythread);
            
            queue->nodes_pos_size <<= 1;
            mythread_queue_node_t** tmp = realloc(queue->nodes, sizeof(mythread_queue_node_t*) * queue->nodes_pos_size);
            
            if(tmp) {
                memset(&tmp[queue->nodes_pos], 0, sizeof(mythread_queue_node_t*) * (queue->nodes_pos_size - queue->nodes_pos));
                
                queue->nodes = tmp;
            }
            else {
                if(status)
                    *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_NODES_MALLOC;
                
                return NULL;
            }
        }
        
        if(queue->nodes[queue->nodes_pos] == NULL) {
            queue->nodes[queue->nodes_pos] = (mythread_queue_node_t*)malloc(sizeof(mythread_queue_node_t) * queue->nodes_size);
            
            if(queue->nodes[queue->nodes_pos] == NULL) {
                if(status)
                    *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_NODE_MALLOC;
                
                return NULL;
            }
        }
        
        queue->nodes_length = 0;
    }
    
    queue->nodes_uses++;
    
    mythread_queue_node_t *qnode = &queue->nodes[queue->nodes_pos][queue->nodes_length];
    
    qnode->text  = text;
    qnode->begin = begin;
    
    return qnode;
}

mythread_queue_node_t * mythread_queue_node_malloc_limit(mythread_t *mythread, mythread_queue_t* queue, const char* text, size_t begin, size_t limit, myhtml_status_t *status)
{
    queue->nodes_length++;
    
    if(queue->nodes_uses >= limit) {
        queue->nodes_uses++;
        mythread_wait_all_for_done(mythread);
        
        queue->nodes_length = 0;
        queue->nodes_pos    = 0;
        queue->nodes_root   = 0;
        queue->nodes_uses   = 0;
    }
    else if(queue->nodes_length >= queue->nodes_size)
    {
        queue->nodes_pos++;
        
        if(queue->nodes_pos >= queue->nodes_pos_size)
        {
            mythread_wait_all_for_done(mythread);
            
            queue->nodes_pos_size <<= 1;
            mythread_queue_node_t** tmp = realloc(queue->nodes, sizeof(mythread_queue_node_t*) * queue->nodes_pos_size);
            
            if(tmp) {
                memset(&tmp[queue->nodes_pos], 0, sizeof(mythread_queue_node_t*) * (queue->nodes_pos_size - queue->nodes_pos));
                
                queue->nodes = tmp;
            }
            else {
                if(status)
                    *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_NODES_MALLOC;
                
                return NULL;
            }
        }
        
        if(queue->nodes[queue->nodes_pos] == NULL) {
            queue->nodes[queue->nodes_pos] = (mythread_queue_node_t*)malloc(sizeof(mythread_queue_node_t) * queue->nodes_size);
            
            if(queue->nodes[queue->nodes_pos] == NULL) {
                if(status)
                    *status = MyHTML_STATUS_THREAD_ERROR_QUEUE_NODE_MALLOC;
                
                return NULL;
            }
        }
        
        queue->nodes_length = 0;
    }
    
    queue->nodes_uses++;
    
    mythread_queue_node_t *qnode = &queue->nodes[queue->nodes_pos][queue->nodes_length];
    
    qnode->text  = text;
    qnode->begin = begin;
    
    return qnode;
}

#ifndef MyHTML_BUILD_WITHOUT_THREADS

mythread_queue_node_t * mythread_queue_node_malloc_round(mythread_t *mythread, mythread_queue_list_entry_t *entry,
                                                              const char* text, size_t begin, myhtml_status_t *status)
{
    mythread_queue_t* queue = entry->queue;
    
    queue->nodes_length++;
    
    if(queue->nodes_length >= queue->nodes_size) {
        queue->nodes_uses++;
        
        mythread_queue_list_entry_wait_for_done(mythread, entry);
        mythread_queue_list_entry_clean(mythread, entry);
    }
    else
        queue->nodes_uses++;
    
    mythread_queue_node_t *qnode = &queue->nodes[queue->nodes_pos][queue->nodes_length];
    
    qnode->text  = text;
    qnode->begin = begin;
    
    return qnode;
}

#endif /* MyHTML_BUILD_WITHOUT_THREADS */

#ifdef MyHTML_BUILD_WITHOUT_THREADS

void mythread_stream_quit_all(mythread_t *mythread) {}
void mythread_batch_quit_all(mythread_t *mythread) {}
void mythread_stream_stop_all(mythread_t *mythread) {};
void mythread_batch_stop_all(mythread_t *mythread) {};
void mythread_stop_all(mythread_t *mythread) {};
void mythread_resume_all(mythread_t *mythread) {};
void mythread_wait_all_for_done(mythread_t *mythread) {};
void mythread_suspend_all(mythread_t *mythread) {};

#else /* MyHTML_BUILD_WITHOUT_THREADS */

// mythread functions
void mythread_stream_quit_all(mythread_t *mythread)
{
    mythread->stream_opt = MyTHREAD_OPT_QUIT;
}

void mythread_batch_quit_all(mythread_t *mythread)
{
    mythread->batch_opt = MyTHREAD_OPT_QUIT;
}

void mythread_stream_stop_all(mythread_t *mythread)
{
    if(mythread->stream_opt != MyTHREAD_OPT_STOP)
        mythread->stream_opt = MyTHREAD_OPT_STOP;
    
    size_t idx;
    const struct timespec tomeout = {0, 0};
    
    for (idx = mythread->pth_list_root; idx < mythread->batch_first_id; idx++) {
        while(mythread->pth_list[idx].data.opt != MyTHREAD_OPT_STOP) {
            myhtml_thread_nanosleep(&tomeout);
        }
    }
}

void mythread_batch_stop_all(mythread_t *mythread)
{
    if(mythread->batch_opt != MyTHREAD_OPT_STOP)
        mythread->batch_opt = MyTHREAD_OPT_STOP;
    
    size_t idx;
    const struct timespec tomeout = {0, 0};
    
    for (idx = mythread->batch_first_id; idx < (mythread->batch_first_id + mythread->batch_count); idx++) {
        while(mythread->pth_list[idx].data.opt != MyTHREAD_OPT_STOP) {
            myhtml_thread_nanosleep(&tomeout);
        }
    }
}

void mythread_stop_all(mythread_t *mythread)
{
    mythread_stream_stop_all(mythread);
    mythread_batch_stop_all(mythread);
}

void mythread_resume_all(mythread_t *mythread)
{
    if(mythread->stream_opt == MyTHREAD_OPT_UNDEF &&
       mythread->batch_opt  == MyTHREAD_OPT_UNDEF)
        return;
    
    if(mythread->stream_opt == MyTHREAD_OPT_WAIT ||
       mythread->batch_opt == MyTHREAD_OPT_WAIT)
    {
        mythread->stream_opt = MyTHREAD_OPT_UNDEF;
        mythread->batch_opt  = MyTHREAD_OPT_UNDEF;
    }
    else {
        mythread->stream_opt = MyTHREAD_OPT_UNDEF;
        mythread->batch_opt  = MyTHREAD_OPT_UNDEF;
        
        for (size_t idx = mythread->pth_list_root; idx < mythread->pth_list_size; idx++) {
            myhtml_hread_mutex_post(mythread, &mythread->pth_list[idx].data);
        }
    }
}

void mythread_wait_all_for_done(mythread_t *mythread)
{
    const struct timespec tomeout = {0, 0};
    
    mythread_queue_list_t *queue_list = mythread->queue_list;
    mythread_queue_list_entry_t *entry = queue_list->first;
    
    while(entry)
    {
        for (size_t idx = mythread->pth_list_root; idx < mythread->pth_list_size; idx++) {
            while(entry->thread_param[idx].use < entry->queue->nodes_uses) {
                myhtml_thread_nanosleep(&tomeout);
            }
        }
        
        entry = entry->next;
    }
}

void mythread_suspend_all(mythread_t *mythread)
{
    if(mythread->stream_opt != MyTHREAD_OPT_WAIT)
        mythread->stream_opt = MyTHREAD_OPT_WAIT;
    
    if(mythread->batch_opt != MyTHREAD_OPT_WAIT)
        mythread->batch_opt  = MyTHREAD_OPT_WAIT;
    
    const struct timespec tomeout = {0, 0};
    
    for (size_t idx = mythread->pth_list_root; idx < mythread->pth_list_size; idx++) {
        myhtml_hread_mutex_try_wait(mythread, &mythread->pth_list[idx].data);
        
        while(mythread->pth_list[idx].data.opt != MyTHREAD_OPT_WAIT) {
            myhtml_thread_nanosleep(&tomeout);
        }
    }
}

bool mythread_function_see_for_all_done(mythread_queue_list_t *queue_list, size_t thread_id)
{
    size_t done_count = 0;
    
    mythread_queue_list_entry_t *entry = queue_list->first;
    while(entry)
    {
        if(entry->thread_param[ thread_id ].use >= entry->queue->nodes_uses) {
            done_count++;
            entry = entry->next;
        }
        else
            break;
    }
    
    return done_count == queue_list->count;
}

bool mythread_function_see_opt(mythread_context_t *ctx, volatile mythread_thread_opt_t opt, size_t done_count, const struct timespec *timeout)
{
    mythread_t * mythread = ctx->mythread;
    mythread_queue_list_t *queue_list = mythread->queue_list;
    
    if(done_count != queue_list->count)
        return false;
    
    if(opt & MyTHREAD_OPT_STOP)
    {
        if(mythread_function_see_for_all_done(queue_list, ctx->id))
        {
            ctx->opt = MyTHREAD_OPT_STOP;
            myhtml_hread_mutex_wait(mythread, ctx);
            ctx->opt = MyTHREAD_OPT_UNDEF;
            
            return false;
        }
    }
    else if(opt & MyTHREAD_OPT_QUIT)
    {
        if(mythread_function_see_for_all_done(queue_list, ctx->id))
        {
            myhtml_hread_mutex_close(mythread, ctx);
            ctx->opt = MyTHREAD_OPT_QUIT;
            return true;
        }
    }
    
    myhtml_thread_nanosleep(timeout);
    
    return false;
}

void mythread_function_batch(void *arg)
{
    mythread_context_t *ctx = (mythread_context_t*)arg;
    mythread_t * mythread = ctx->mythread;
    mythread_queue_list_t *queue_list = mythread->queue_list;
    
    const struct timespec timeout = {0, 0};
    myhtml_hread_mutex_wait(mythread, ctx);
    
    do {
        if(mythread->batch_opt & MyTHREAD_OPT_WAIT) {
            ctx->opt = MyTHREAD_OPT_WAIT;
            
            while (mythread->batch_opt & MyTHREAD_OPT_WAIT) {
                myhtml_thread_nanosleep(&timeout);
            }
            
            ctx->opt = MyTHREAD_OPT_UNDEF;
        }
        
        mythread_queue_list_entry_t *entry = queue_list->first;
        size_t done_count = 0;
        
        while(entry)
        {
            mythread_queue_thread_param_t *thread_param = &entry->thread_param[ ctx->id ];
            
            if(thread_param->use < entry->queue->nodes_uses)
            {
                size_t pos = thread_param->use / entry->queue->nodes_size;
                size_t len = thread_param->use % entry->queue->nodes_size;
                
                mythread_queue_node_t *qnode = &entry->queue->nodes[pos][len];
                
                if((qnode->tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
                    ctx->func(ctx->id, qnode);
                
                thread_param->use += mythread->batch_count;
            }
            else
                done_count++;
            
            entry = entry->next;
        }
        
        if(done_count == queue_list->count &&
           mythread_function_see_opt(ctx, mythread->batch_opt, done_count, &timeout))
            break;
    }
    while (1);
}

void mythread_function_stream(void *arg)
{
    mythread_context_t *ctx = (mythread_context_t*)arg;
    mythread_t * mythread = ctx->mythread;
    mythread_queue_list_t *queue_list = mythread->queue_list;
    
    const struct timespec timeout = {0, 0};
    myhtml_hread_mutex_wait(mythread, ctx);
    
    do {
        if(mythread->stream_opt & MyTHREAD_OPT_WAIT) {
            ctx->opt = MyTHREAD_OPT_WAIT;
            
            while (mythread->stream_opt & MyTHREAD_OPT_WAIT) {
                myhtml_thread_nanosleep(&timeout);
            }
            
            ctx->opt = MyTHREAD_OPT_UNDEF;
        }
        
        mythread_queue_list_entry_t *entry = queue_list->first;
        size_t done_count = 0;
        
        while(entry)
        {
            mythread_queue_thread_param_t *thread_param = &entry->thread_param[ ctx->id ];
            
            if(thread_param->use < entry->queue->nodes_uses)
            {
                size_t pos = thread_param->use / entry->queue->nodes_size;
                size_t len = thread_param->use % entry->queue->nodes_size;
                
                mythread_queue_node_t *qnode = &entry->queue->nodes[pos][len];
                
                if((qnode->tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
                    ctx->func(ctx->id, qnode);
                
                thread_param->use++;
            }
            else
                done_count++;
            
            entry = entry->next;
        }
        
        if(done_count == queue_list->count &&
           mythread_function_see_opt(ctx, mythread->stream_opt, done_count, &timeout))
            break;
    }
    while (1);
}

#endif /* MyHTML_BUILD_WITHOUT_THREADS */



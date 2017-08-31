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

#include "mycore/thread_queue.h"

mythread_queue_t * mythread_queue_create(void)
{
    return mycore_calloc(1, sizeof(mythread_queue_t));
}

mystatus_t mythread_queue_init(mythread_queue_t* queue, size_t size)
{
    if(size < 32)
        size = 32;
    
    queue->nodes_pos_size = 512;
    queue->nodes_size     = size;
    queue->nodes          = (mythread_queue_node_t**)mycore_calloc(queue->nodes_pos_size, sizeof(mythread_queue_node_t*));
    
    if(queue->nodes == NULL)
        return MyCORE_STATUS_THREAD_ERROR_QUEUE_NODES_MALLOC;
    
    queue->nodes[queue->nodes_pos] = (mythread_queue_node_t*)mycore_malloc(sizeof(mythread_queue_node_t) * queue->nodes_size);
    
    if(queue->nodes[queue->nodes_pos] == NULL) {
        queue->nodes = mycore_free(queue->nodes);
        return MyCORE_STATUS_THREAD_ERROR_QUEUE_NODE_MALLOC;
    }
    
    return MyCORE_STATUS_OK;
}

void mythread_queue_clean(mythread_queue_t* queue)
{
    queue->nodes_length = 0;
    queue->nodes_pos    = 0;
    queue->nodes_root   = 0;
    queue->nodes_uses   = 0;
}

mythread_queue_t * mythread_queue_destroy(mythread_queue_t* queue)
{
    if(queue == NULL)
        return NULL;
    
    if(queue->nodes) {
        /* '<=' it is normal */
        for (size_t i = 0; i <= queue->nodes_pos; i++) {
            mycore_free(queue->nodes[i]);
        }
        
        mycore_free(queue->nodes);
    }
    
    mycore_free(queue);
    
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

mythread_queue_node_t * mythread_queue_node_malloc(mythread_t *mythread, mythread_queue_t* queue, mystatus_t *status)
{
    queue->nodes_length++;
    
    if(queue->nodes_length >= queue->nodes_size)
    {
        queue->nodes_pos++;
        
        if(queue->nodes_pos >= queue->nodes_pos_size)
        {
#ifndef MyCORE_BUILD_WITHOUT_THREADS
            if(mythread)
                mythread_queue_list_wait_for_done(mythread, mythread->context);
#endif
            size_t new_size = queue->nodes_pos_size + 512;
            mythread_queue_node_t** tmp = mycore_realloc(queue->nodes, sizeof(mythread_queue_node_t*) * new_size);
            
            if(tmp) {
                memset(&tmp[queue->nodes_pos], 0, sizeof(mythread_queue_node_t*) * (new_size - queue->nodes_pos));
                
                queue->nodes = tmp;
                queue->nodes_pos_size = new_size;
            }
            else {
                if(status)
                    *status = MyCORE_STATUS_THREAD_ERROR_QUEUE_NODES_MALLOC;
                
                return NULL;
            }
        }
        
        if(queue->nodes[queue->nodes_pos] == NULL) {
            queue->nodes[queue->nodes_pos] = (mythread_queue_node_t*)mycore_malloc(sizeof(mythread_queue_node_t) * queue->nodes_size);
            
            if(queue->nodes[queue->nodes_pos] == NULL) {
                if(status)
                    *status = MyCORE_STATUS_THREAD_ERROR_QUEUE_NODE_MALLOC;
                
                return NULL;
            }
        }
        
        queue->nodes_length = 0;
    }
    
    queue->nodes_uses++;
    
    return &queue->nodes[queue->nodes_pos][queue->nodes_length];
}

mythread_queue_node_t * mythread_queue_node_malloc_limit(mythread_t *mythread, mythread_queue_t* queue, size_t limit, mystatus_t *status)
{
    queue->nodes_length++;
    
    if(queue->nodes_uses >= limit) {
        queue->nodes_uses++;
        
#ifndef MyCORE_BUILD_WITHOUT_THREADS
        if(mythread)
            mythread_queue_list_wait_for_done(mythread, mythread->context);
#endif
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
#ifndef MyCORE_BUILD_WITHOUT_THREADS
            if(mythread)
                mythread_queue_list_wait_for_done(mythread, mythread->context);
#endif
            size_t new_size = queue->nodes_pos_size + 512;
            mythread_queue_node_t** tmp = mycore_realloc(queue->nodes, sizeof(mythread_queue_node_t*) * new_size);
            
            if(tmp) {
                memset(&tmp[queue->nodes_pos], 0, sizeof(mythread_queue_node_t*) * (new_size - queue->nodes_pos));
                
                queue->nodes = tmp;
                queue->nodes_pos_size = new_size;
            }
            else {
                if(status)
                    *status = MyCORE_STATUS_THREAD_ERROR_QUEUE_NODES_MALLOC;
                
                return NULL;
            }
        }
        
        if(queue->nodes[queue->nodes_pos] == NULL) {
            queue->nodes[queue->nodes_pos] = (mythread_queue_node_t*)mycore_malloc(sizeof(mythread_queue_node_t) * queue->nodes_size);
            
            if(queue->nodes[queue->nodes_pos] == NULL) {
                if(status)
                    *status = MyCORE_STATUS_THREAD_ERROR_QUEUE_NODE_MALLOC;
                
                return NULL;
            }
        }
        
        queue->nodes_length = 0;
    }
    
    queue->nodes_uses++;
    
    return &queue->nodes[queue->nodes_pos][queue->nodes_length];
}

#ifndef MyCORE_BUILD_WITHOUT_THREADS
mythread_queue_node_t * mythread_queue_node_malloc_round(mythread_t *mythread, mythread_queue_list_entry_t *entry)
{
    mythread_queue_t* queue = entry->queue;
    
    queue->nodes_length++;
    
    if(queue->nodes_length >= queue->nodes_size) {
        queue->nodes_uses++;
        
#ifndef MyCORE_BUILD_WITHOUT_THREADS
        if(mythread)
            mythread_queue_list_entry_wait_for_done(mythread, entry);
#endif
        
        mythread_queue_list_entry_clean(entry);
    }
    else
        queue->nodes_uses++;
    
    return &queue->nodes[queue->nodes_pos][queue->nodes_length];
}
#endif

#ifndef MyCORE_BUILD_WITHOUT_THREADS
/*
 * Queue List
 */
mythread_queue_list_t * mythread_queue_list_create(mystatus_t *status)
{
    return (mythread_queue_list_t*)mycore_calloc(1, sizeof(mythread_queue_list_t));
}

void mythread_queue_list_destroy(mythread_queue_list_t* queue_list)
{
    if(queue_list == NULL)
        return;
    
    mycore_free(queue_list);
}

size_t mythread_queue_list_get_count(mythread_queue_list_t* queue_list)
{
    return queue_list->count;
}

void mythread_queue_list_wait_for_done(mythread_t* mythread, mythread_queue_list_t* queue_list)
{
    if(queue_list == NULL)
        return;
    
    mythread_queue_list_entry_t *entry = queue_list->first;
    
    while(entry)
    {
        for (size_t i = 0; i < mythread->entries_length; i++) {
            while(entry->thread_param[i].use < entry->queue->nodes_uses)
                mythread_nanosleep_sleep(mythread->timespec);
        }
        
        entry = entry->next;
    }
}

bool mythread_queue_list_see_for_done(mythread_t* mythread, mythread_queue_list_t* queue_list)
{
    if(queue_list == NULL)
        return true;
    
    mythread_queue_list_entry_t *entry = queue_list->first;
    
    while(entry)
    {
        for (size_t i = 0; i < mythread->entries_length; i++) {
            if(entry->thread_param[i].use < entry->queue->nodes_uses)
                return false;
        }
        
        entry = entry->next;
    }
    
    return true;
}

bool mythread_queue_list_see_for_done_by_thread(mythread_t* mythread, mythread_queue_list_t* queue_list, mythread_id_t thread_id)
{
    if(queue_list == NULL)
        return true;
    
    mythread_queue_list_entry_t *entry = queue_list->first;
    
    while(entry)
    {
        if(entry->thread_param[thread_id].use < entry->queue->nodes_uses)
            return false;
        
        entry = entry->next;
    }
    
    return true;
}

/*
 * Queue List Entry
 */
mythread_queue_list_entry_t * mythread_queue_list_entry_push(mythread_t** mythread_list, size_t list_size, mythread_queue_list_t* queue_list,
                                                             mythread_queue_t* queue, size_t thread_param_size, mystatus_t* status)
{
    if(status)
        *status = MyCORE_STATUS_OK;
    
    /* create new entry */
    mythread_queue_list_entry_t* entry = (mythread_queue_list_entry_t*)mycore_calloc(1, sizeof(mythread_queue_list_entry_t));
    if(entry == NULL) {
        if(status)
            *status = MyCORE_STATUS_THREAD_ERROR_QUEUE_MALLOC;
        
        return NULL;
    }
    
    /* create thread params */
    entry->thread_param_size = thread_param_size;
    
    if(thread_param_size) {
        entry->thread_param = (mythread_queue_thread_param_t*)mycore_calloc(thread_param_size, sizeof(mythread_queue_thread_param_t));
        
        if(entry->thread_param == NULL) {
            mycore_free(entry);
            
            if(status)
                *status = MyCORE_STATUS_THREAD_ERROR_QUEUE_MALLOC;
            
            return NULL;
        }
    }
    else
        entry->thread_param = NULL;
    
    entry->queue = queue;
    
    for(size_t i = 0; i < list_size; i++) {
        if(mythread_list[i]->type == MyTHREAD_TYPE_BATCH)
            mythread_queue_list_entry_make_batch(mythread_list[i], entry);
        else
            mythread_queue_list_entry_make_stream(mythread_list[i], entry);
        
        if(mythread_list[i])
            mythread_suspend(mythread_list[i]);
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
    
    for(size_t i = 0; i < list_size; i++)
        if(mythread_list[i])
            mythread_resume(mythread_list[i], MyTHREAD_OPT_UNDEF);
    
    return entry;
}

mythread_queue_list_entry_t * mythread_queue_list_entry_delete(mythread_t** mythread_list, size_t list_size, mythread_queue_list_t *queue_list, mythread_queue_list_entry_t *entry, bool destroy_queue)
{
    for(size_t i = 0; i < list_size; i++)
        if(mythread_list[i])
            mythread_suspend(mythread_list[i]);
    
    mythread_queue_list_entry_t *next = entry->next;
    mythread_queue_list_entry_t *prev = entry->prev;
    
    if(prev)
        prev->next = next;
    
    if(next)
        next->prev = prev;
    
    if(queue_list->first == entry)
        queue_list->first = next;
    
    if(queue_list->last == entry)
        queue_list->last = prev;
    
    queue_list->count--;
    
    for(size_t i = 0; i < list_size; i++)
        if(mythread_list[i])
            mythread_resume(mythread_list[i], MyTHREAD_OPT_UNDEF);
    
    if(destroy_queue && entry->queue)
        mythread_queue_destroy(entry->queue);
    
    if(entry->thread_param)
        mycore_free(entry->thread_param);
    
    mycore_free(entry);
    
    return NULL;
}

void mythread_queue_list_entry_clean(mythread_queue_list_entry_t *entry)
{
    if(entry == NULL)
        return;
    
    mythread_queue_clean(entry->queue);
}

void mythread_queue_list_entry_wait_for_done(mythread_t* mythread, mythread_queue_list_entry_t *entry)
{
    if(entry == NULL)
        return;
    
    for(size_t i = 0; i < entry->thread_param_size; i++) {
        while(entry->thread_param[i].use < entry->queue->nodes_uses)
            mythread_nanosleep_sleep(mythread->timespec);
    }
}

bool mythread_queue_list_entry_see_for_done(mythread_queue_list_entry_t *entry)
{
    if(entry == NULL)
        return true;
    
    for(size_t i = 0; i < entry->thread_param_size; i++) {
        if(entry->thread_param[i].use < entry->queue->nodes_uses)
            return false;
    }
    
    return true;
}

void mythread_queue_list_entry_make_batch(mythread_t* mythread, mythread_queue_list_entry_t* entry)
{
    if(entry == NULL)
        return;
    
    size_t i = 0;
    for(size_t from = mythread->id_increase; from <= mythread->entries_length; from++) {
        entry->thread_param[from].use = i;
        i++;
    }
}

void mythread_queue_list_entry_make_stream(mythread_t* mythread, mythread_queue_list_entry_t* entry)
{
    if(entry == NULL)
        return;
    
    for(size_t from = mythread->id_increase; from <= mythread->entries_length; from++) {
        entry->thread_param[from].use = 0;
    }
}

/*
 * Thread Process Functions
 */
bool mythread_function_see_opt(mythread_context_t *ctx, volatile mythread_thread_opt_t opt, mythread_id_t thread_id, size_t done_count, void* timeout)
{
    mythread_t *mythread = ctx->mythread;
    mythread_queue_list_t *queue_list = (mythread_queue_list_t*)mythread->context;
    
    if(done_count != queue_list->count)
        return false;
    
    if(opt & MyTHREAD_OPT_STOP)
    {
        if(mythread_queue_list_see_for_done_by_thread(mythread, queue_list, thread_id))
        {
            ctx->opt = MyTHREAD_OPT_STOP;
            mythread_mutex_wait(mythread, ctx->mutex);
            ctx->opt = MyTHREAD_OPT_UNDEF;
            
            return false;
        }
    }
    else if(opt & MyTHREAD_OPT_QUIT)
    {
        if(mythread_queue_list_see_for_done_by_thread(mythread, queue_list, thread_id))
        {
            mythread_mutex_close(mythread, ctx->mutex);
            mythread_nanosleep_destroy(ctx->timespec);
            
            ctx->opt = MyTHREAD_OPT_QUIT;
            return true;
        }
    }
    
    mythread_nanosleep_sleep(timeout);
    
    return false;
}

void * mythread_function_queue_batch(void *arg)
{
    mythread_context_t *ctx = (mythread_context_t*)arg;
    mythread_t *mythread  = ctx->mythread;
    mythread_queue_list_t *queue_list = (mythread_queue_list_t*)mythread->context;
    mythread_id_t thread_id = myhread_increase_id_by_entry_id(mythread, ctx->id);
    
    mythread_mutex_wait(mythread, ctx->mutex);
    
    do {
        if(mythread->opt & MyTHREAD_OPT_WAIT) {
            ctx->opt = MyTHREAD_OPT_WAIT;
            
            while (mythread->opt & MyTHREAD_OPT_WAIT)
                mythread_nanosleep_sleep(ctx->timespec);
            
            ctx->opt = MyTHREAD_OPT_UNDEF;
        }
        
        mythread_queue_list_entry_t *entry = queue_list->first;
        size_t done_count = 0;
        
        while(entry)
        {
            mythread_queue_thread_param_t *thread_param = &entry->thread_param[ thread_id ];
            
            if(thread_param->use < entry->queue->nodes_uses)
            {
                size_t pos = thread_param->use / entry->queue->nodes_size;
                size_t len = thread_param->use % entry->queue->nodes_size;
                
                mythread_queue_node_t *qnode = &entry->queue->nodes[pos][len];
                
                //if((qnode->tree->flags & MyCORE_TREE_FLAGS_SINGLE_MODE) == 0)
                ctx->func(ctx->id, (void*)qnode);
                
                thread_param->use += mythread->entries_length;
            }
            else
                done_count++;
            
            entry = entry->next;
        }
        
        if(done_count == queue_list->count &&
           mythread_function_see_opt(ctx, mythread->opt, thread_id, done_count, ctx->timespec))
        {
            break;
        }
    }
    while (1);
    
    return NULL;
}

void * mythread_function_queue_stream(void *arg)
{
    mythread_context_t *ctx = (mythread_context_t*)arg;
    mythread_t * mythread = ctx->mythread;
    mythread_queue_list_t *queue_list = (mythread_queue_list_t*)mythread->context;
    mythread_id_t thread_id = myhread_increase_id_by_entry_id(mythread, ctx->id);
    
    mythread_mutex_wait(mythread, ctx->mutex);
    
    do {
        if(mythread->opt & MyTHREAD_OPT_WAIT) {
            ctx->opt = MyTHREAD_OPT_WAIT;
            
            while (mythread->opt & MyTHREAD_OPT_WAIT) {
                mythread_nanosleep_sleep(ctx->timespec);
            }
            
            ctx->opt = MyTHREAD_OPT_UNDEF;
        }
        
        mythread_queue_list_entry_t *entry = queue_list->first;
        size_t done_count = 0;
        
        while(entry)
        {
            mythread_queue_thread_param_t *thread_param = &entry->thread_param[ thread_id ];
            
            if(thread_param->use < entry->queue->nodes_uses)
            {
                size_t pos = thread_param->use / entry->queue->nodes_size;
                size_t len = thread_param->use % entry->queue->nodes_size;
                
                mythread_queue_node_t *qnode = &entry->queue->nodes[pos][len];
                
                //if((qnode->tree->flags & MyCORE_TREE_FLAGS_SINGLE_MODE) == 0)
                ctx->func(ctx->id, (void*)qnode);
                
                thread_param->use++;
            }
            else
                done_count++;
            
            entry = entry->next;
        }
        
        if(done_count == queue_list->count &&
           mythread_function_see_opt(ctx, mythread->opt, thread_id, done_count, ctx->timespec))
        {
            break;
        }
    }
    while(1);
    
    return NULL;
}

void * mythread_function(void *arg)
{
    mythread_context_t *ctx = (mythread_context_t*)arg;
    mythread_t * mythread = ctx->mythread;
    
    mythread_mutex_wait(mythread, ctx->mutex);
    
    do {
        ctx->func(ctx->id, ctx);
        
        ctx->opt |= MyTHREAD_OPT_DONE;
        
        if(ctx->opt & MyTHREAD_OPT_WAIT) {
            while (ctx->opt & MyTHREAD_OPT_WAIT) {
                mythread_nanosleep_sleep(ctx->timespec);
            }
        }
        else {
            ctx->opt |= MyTHREAD_OPT_STOP;
            mythread_mutex_wait(mythread, ctx->mutex);
        }
        
        if(mythread->opt & MyTHREAD_OPT_QUIT || ctx->opt & MyTHREAD_OPT_QUIT)
        {
            mythread_mutex_close(mythread, ctx->mutex);
            mythread_nanosleep_destroy(ctx->timespec);
            
            ctx->opt = MyTHREAD_OPT_QUIT;
            break;
        }
        
        ctx->opt = MyTHREAD_OPT_UNDEF;
    }
    while(1);
    
    return NULL;
}

#endif

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

#include "mycore/mythread.h"

#ifndef MyCORE_BUILD_WITHOUT_THREADS

mythread_t * mythread_create(void)
{
    return mycore_calloc(1, sizeof(mythread_t));
}

mystatus_t mythread_init(mythread_t *mythread, mythread_type_t type, size_t threads_count, size_t id_increase)
{
    if(threads_count == 0)
        return MyCORE_STATUS_ERROR;
    
    mythread->entries_size   = threads_count;
    mythread->entries_length = 0;
    mythread->id_increase    = id_increase;
    mythread->type           = type;
    
    mythread->entries = (mythread_entry_t*)mycore_calloc(mythread->entries_size, sizeof(mythread_entry_t));
    if(mythread->entries == NULL)
        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    
    mythread->attr = mythread_thread_attr_init(mythread);
    if(mythread->attr == NULL)
        return MyCORE_STATUS_THREAD_ERROR_ATTR_INIT;
    
    mythread->timespec = mythread_nanosleep_create(mythread);
    
    return MyCORE_STATUS_OK;
}

void mythread_clean(mythread_t *mythread)
{
    mythread_thread_attr_clean(mythread, mythread->attr);
    mythread_nanosleep_clean(mythread->timespec);
    
    mythread->sys_last_error = 0;
}

mythread_t * mythread_destroy(mythread_t *mythread, mythread_callback_before_entry_join_f before_join, void* ctx, bool self_destroy)
{
    if(mythread == NULL)
        return NULL;
    
    if(mythread->entries) {
        mythread_resume(mythread, MyTHREAD_OPT_QUIT);
        mythread_quit(mythread, before_join, ctx);
        mycore_free(mythread->entries);
    }
    
    mythread_thread_attr_destroy(mythread, mythread->attr);
    mythread_nanosleep_destroy(mythread->timespec);
    
    if(self_destroy) {
        mycore_free(mythread);
        return NULL;
    }
    
    return mythread;
}

mystatus_t myhread_entry_create(mythread_t *mythread, mythread_process_f process_func, mythread_work_f work_func, mythread_thread_opt_t opt)
{
    mythread->sys_last_error = 0;
    
    if(mythread->entries_length >= mythread->entries_size)
        return MyCORE_STATUS_THREAD_ERROR_NO_SLOTS;
    
    mythread_entry_t *entry = &mythread->entries[mythread->entries_length];
    
    entry->context.mythread = mythread;
    entry->context.func     = work_func;
    entry->context.id       = mythread->entries_length;
    entry->context.t_count  = mythread->entries_size;
    entry->context.opt      = opt;
    entry->context.status   = 0;
    
    entry->context.timespec = mythread_nanosleep_create(mythread);
    
    entry->context.mutex = mythread_mutex_create(mythread);
    if(entry->context.mutex == NULL)
        return MyCORE_STATUS_THREAD_ERROR_MUTEX_INIT;
    
    if(mythread_mutex_wait(mythread, entry->context.mutex)) {
        mythread_mutex_close(mythread, entry->context.mutex);
        return MyCORE_STATUS_THREAD_ERROR_MUTEX_LOCK;
    }
    
    entry->thread = mythread_thread_create(mythread, process_func, &entry->context);
    if(entry->thread == NULL) {
        mythread_mutex_close(mythread, entry->context.mutex);
        return MyCORE_STATUS_ERROR;
    }
    
    mythread->entries_length++;
    
    return MyCORE_STATUS_OK;
}

mythread_id_t myhread_increase_id_by_entry_id(mythread_t* mythread, mythread_id_t thread_id)
{
    return mythread->id_increase + thread_id;
}

/*
 * Global functions, for all threads
 */
mystatus_t mythread_join(mythread_t *mythread, mythread_callback_before_entry_join_f before_join, void* ctx)
{
    for (size_t i = 0; i < mythread->entries_length; i++) {
        if(before_join)
            before_join(mythread, &mythread->entries[i], ctx);
        
        if(mythread_thread_join(mythread, mythread->entries[i].thread))
            return MyCORE_STATUS_ERROR;
    }
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_quit(mythread_t *mythread, mythread_callback_before_entry_join_f before_join, void* ctx)
{
    mythread_option_set(mythread, MyTHREAD_OPT_QUIT);
    
    for (size_t i = 0; i < mythread->entries_length; i++)
    {
        if(before_join)
            before_join(mythread, &mythread->entries[i], ctx);
        
        if(mythread_thread_join(mythread, mythread->entries[i].thread) ||
           mythread_thread_destroy(mythread, mythread->entries[i].thread))
        {
            return MyCORE_STATUS_ERROR;
        }
    }
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_stop(mythread_t *mythread)
{
    if(mythread->opt & MyTHREAD_OPT_STOP)
        return MyCORE_STATUS_OK;
    
    mythread_option_set(mythread, MyTHREAD_OPT_STOP);
    
    for (size_t i = 0; i < mythread->entries_length; i++)
    {
        while((mythread->entries[i].context.opt & MyTHREAD_OPT_STOP) == 0) {
            mythread_nanosleep_sleep(mythread->timespec);
        }
    }
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_suspend(mythread_t *mythread)
{
    if(mythread->opt & MyTHREAD_OPT_WAIT)
        return MyCORE_STATUS_OK;
    
    mythread_option_set(mythread, MyTHREAD_OPT_WAIT);
    
    for (size_t i = 0; i < mythread->entries_length; i++)
    {
        while((mythread->entries[i].context.opt & MyTHREAD_OPT_STOP) == 0 &&
              (mythread->entries[i].context.opt & MyTHREAD_OPT_WAIT) == 0)
        {
            mythread_nanosleep_sleep(mythread->timespec);
        }
    }
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_resume(mythread_t *mythread, mythread_thread_opt_t send_opt)
{
    if(mythread->opt & MyTHREAD_OPT_WAIT) {
        mythread_option_set(mythread, send_opt);
        return MyCORE_STATUS_OK;
    }
    
    mythread_option_set(mythread, send_opt);
    
    for (size_t i = 0; i < mythread->entries_length; i++)
    {
        if(mythread->entries[i].context.opt & MyTHREAD_OPT_STOP) {
            mythread->entries[i].context.opt = send_opt;
            
            if(mythread_mutex_post(mythread, mythread->entries[i].context.mutex))
                return MyCORE_STATUS_ERROR;
        }
    }
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_check_status(mythread_t *mythread)
{
    for (size_t i = 0; i < mythread->entries_length; i++)
    {
        if(mythread->entries[i].context.status)
            return mythread->entries[i].context.status;
    }
    
    return MyCORE_STATUS_OK;
}

mythread_thread_opt_t mythread_option(mythread_t *mythread)
{
    return mythread->opt;
}

void mythread_option_set(mythread_t *mythread, mythread_thread_opt_t opt)
{
    mythread->opt = opt;
}

/*
 * Entries functions, for all threads
 */
mystatus_t mythread_entry_join(mythread_entry_t* entry, mythread_callback_before_entry_join_f before_join, void* ctx)
{
    if(before_join)
        before_join(entry->context.mythread, entry, ctx);
    
    if(mythread_thread_join(entry->context.mythread, entry->thread))
        return MyCORE_STATUS_ERROR;
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_entry_quit(mythread_entry_t* entry, mythread_callback_before_entry_join_f before_join, void* ctx)
{
    if(before_join)
        before_join(entry->context.mythread, entry, ctx);
    
    if(mythread_thread_join(entry->context.mythread, entry->thread) ||
       mythread_thread_destroy(entry->context.mythread, entry->thread))
    {
        return MyCORE_STATUS_ERROR;
    }
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_entry_stop(mythread_entry_t* entry)
{
    if(entry->context.opt & MyTHREAD_OPT_STOP)
        return MyCORE_STATUS_OK;
    
    entry->context.opt = MyTHREAD_OPT_STOP;
    
    while((entry->context.opt & MyTHREAD_OPT_STOP) == 0) {
        mythread_nanosleep_sleep(entry->context.mythread->timespec);
    }
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_entry_suspend(mythread_entry_t* entry)
{
    if(entry->context.opt & MyTHREAD_OPT_WAIT)
        return MyCORE_STATUS_OK;
    
    entry->context.opt = MyTHREAD_OPT_WAIT;
    
    while((entry->context.opt & MyTHREAD_OPT_STOP) == 0 && (entry->context.opt & MyTHREAD_OPT_WAIT) == 0) {
        mythread_nanosleep_sleep(entry->context.mythread->timespec);
    }
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_entry_resume(mythread_entry_t* entry, mythread_thread_opt_t send_opt)
{
    if(entry->context.opt & MyTHREAD_OPT_WAIT) {
        entry->context.opt = send_opt;
    }
    else if(entry->context.opt & MyTHREAD_OPT_STOP) {
        entry->context.opt = send_opt;
        
        if(mythread_mutex_post(entry->context.mythread, entry->context.mutex))
            return MyCORE_STATUS_ERROR;
    }
    else
        entry->context.opt = send_opt;
    
    return MyCORE_STATUS_OK;
}

mystatus_t mythread_entry_status(mythread_entry_t* entry)
{
    return entry->context.status;
}

mythread_t * mythread_entry_mythread(mythread_entry_t* entry)
{
    return entry->context.mythread;
}

/* Callbacks */
void mythread_callback_quit(mythread_t* mythread, mythread_entry_t* entry, void* ctx)
{
    while((entry->context.opt & MyTHREAD_OPT_QUIT) == 0)
        mythread_nanosleep_sleep(mythread->timespec);
}

#endif


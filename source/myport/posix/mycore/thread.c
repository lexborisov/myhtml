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
#include "mycore/utils/mcsync.h"

#ifndef MyCORE_BUILD_WITHOUT_THREADS
#include <pthread.h>

/***********************************************************************************
 *
 * For all unix system. POSIX pthread
 *
 ***********************************************************************************/
void * mythread_thread_create(mythread_t *mythread, mythread_process_f process_func, void* ctx)
{
    void *thread = mycore_calloc(1, sizeof(pthread_t));
    
    if(thread == NULL)
        return NULL;
    
    if(pthread_create(&(*((pthread_t*)thread)), mythread->attr, process_func, ctx) == 0)
        return thread;
    
    return NULL;
}

mystatus_t mythread_thread_join(mythread_t *mythread, void* thread)
{
    if(pthread_join(*((pthread_t*)thread), NULL) == 0)
        return MyCORE_STATUS_OK;
    
    return MyCORE_STATUS_ERROR;
}

mystatus_t mythread_thread_cancel(mythread_t *mythread, void* thread)
{
    if(pthread_cancel(*((pthread_t*)thread)) == 0)
        return MyCORE_STATUS_OK;
    
    return MyCORE_STATUS_ERROR;
}

mystatus_t mythread_thread_destroy(mythread_t *mythread, void* thread)
{
    mycore_free(thread);
    return MyCORE_STATUS_OK;
}

void * mythread_thread_attr_init(mythread_t *mythread)
{
    pthread_attr_t *attr = (pthread_attr_t*)mycore_calloc(1, sizeof(pthread_attr_t));
    
    if(attr == NULL)
        return NULL;
    
    mythread->sys_last_error = pthread_attr_init(attr);
    if(mythread->sys_last_error) {
        mycore_free(attr);
        return NULL;
    }
    
    mythread->sys_last_error = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);
    if(mythread->sys_last_error) {
        mycore_free(attr);
        return NULL;
    }
    
    return attr;
}

void mythread_thread_attr_clean(mythread_t *mythread, void* attr)
{
    /* some code */
}

void mythread_thread_attr_destroy(mythread_t *mythread, void* attr)
{
    if(attr == NULL)
        return;
    
    mythread->sys_last_error = pthread_attr_destroy(attr);
    mycore_free(attr);
}

void * mythread_mutex_create(mythread_t *mythread)
{
    void *mutex = mcsync_mutex_create();
    if(mutex == NULL)
        return NULL;
    
    if(mcsync_mutex_init(mutex)) {
        mycore_free(mutex);
        return NULL;
    }
    
    return mutex;
}

mystatus_t mythread_mutex_post(mythread_t *mythread, void* mutex)
{
    return mcsync_mutex_unlock(mutex);
}

mystatus_t mythread_mutex_wait(mythread_t *mythread, void* mutex)
{
    return mcsync_mutex_lock(mutex);
}

mystatus_t mythread_mutex_try_wait(mythread_t *mythread, void* mutex)
{
    return mcsync_mutex_try_lock(mutex);
}

void mythread_mutex_close(mythread_t *mythread, void* mutex)
{
    mcsync_mutex_destroy(mutex);
}

void * mythread_nanosleep_create(mythread_t* mythread)
{
    return mycore_calloc(1, sizeof(struct timespec));
}

void mythread_nanosleep_clean(void* timespec)
{
    memset(timespec, 0, sizeof(struct timespec));
}

void mythread_nanosleep_destroy(void* timespec)
{
    mycore_free(timespec);
}

mystatus_t mythread_nanosleep_sleep(void* timespec)
{
    if(nanosleep(timespec, NULL) == 0)
        return MyCORE_STATUS_OK;
    
    return MyCORE_STATUS_ERROR;
}

#endif

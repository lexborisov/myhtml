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
#include <windows.h>

/***********************************************************************************
 *
 * For Windows
 *
 ***********************************************************************************/
void * mythread_thread_create(mythread_t *mythread, mythread_process_f process_func, void* ctx)
{
    return CreateThread(NULL,                   // default security attributes
                        0,                      // use default stack size
                        (LPTHREAD_START_ROUTINE)process_func,           // thread function name
                        ctx,                    // argument to thread function
                        0,                      // use default creation flags
                        NULL);                  // returns the thread identifier
}

mystatus_t mythread_thread_join(mythread_t *mythread, void* thread)
{
    if(WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0)
        return MyCORE_STATUS_OK;
    
    return MyCORE_STATUS_ERROR;
}

mystatus_t mythread_thread_cancel(mythread_t *mythread, void* thread)
{
    if(TerminateThread(thread, 0))
        return MyCORE_STATUS_OK;
    
    return MyCORE_STATUS_ERROR;
}

mystatus_t mythread_thread_destroy(mythread_t *mythread, void* thread)
{
    if(TerminateThread(thread, 0))
        return MyCORE_STATUS_OK;
    
    return MyCORE_STATUS_ERROR;
}

void * mythread_thread_attr_init(mythread_t *mythread)
{
    return (void*)0x01;
}

void mythread_thread_attr_clean(mythread_t *mythread, void* attr)
{
}

void mythread_thread_attr_destroy(mythread_t *mythread, void* attr)
{
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
    return (void*)0x01;
}

void mythread_nanosleep_clean(void* timespec)
{
}

void mythread_nanosleep_destroy(void* timespec)
{
}

mystatus_t mythread_nanosleep_sleep(void* timespec)
{
    Sleep(0);
    return MyCORE_STATUS_OK;
}

#endif

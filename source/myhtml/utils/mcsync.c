/*
 Copyright 2015 Alexander Borisov
 
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

#include "myhtml/utils/mcsync.h"

#if !defined(MyHTML_WITHOUT_THREADS) && ((defined(__GNUC__) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || \
    defined(__ATOMIC_SEQ_CST))
#define MyHTML_FORCE_SPINLOCK
#endif

#if defined(MyHTML_FORCE_SPINLOCK)
static int mcsync_atomic_compare_exchange(int* ptr, int compare, int exchange)
{
    return __atomic_compare_exchange_n(ptr, &compare, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static void mcsync_atomic_store(int* ptr, int value)
{
    __atomic_store_n(ptr, 0, __ATOMIC_SEQ_CST);
}
#endif

#if !defined(MyHTML_WITHOUT_THREADS) && defined(IS_OS_WINDOWS)
static int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    EnterCriticalSection(mutex);
    return 0;
}

static int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    LeaveCriticalSection(mutex);
    return 0;
}

static int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
    (void)attr;
    InitializeCriticalSection(mutex);
    return 0;
}

static int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    DeleteCriticalSection(mutex);
    return 0;
}
#endif

mcsync_t * mcsync_create(void)
{
    return calloc(1, sizeof(mcsync_t));
}

mcsync_status_t mcsync_init(mcsync_t* mcsync)
{
    mcsync_clean(mcsync);
    return MCSYNC_STATUS_OK;
}

mcsync_t * mcsync_destroy(mcsync_t* mcsync, int destroy_self)
{
    if(mcsync == NULL)
        return NULL;
    
#if !defined(MyHTML_FORCE_SPINLOCK)
    if(mcsync->mutex) {
        pthread_mutex_destroy(mcsync->mutex);
        free(mcsync->mutex);
    }
#endif
    
    if(destroy_self)
        free(mcsync);
    
    return NULL;
}

void mcsync_clean(mcsync_t* mcsync)
{
    mcsync->spinlock = 0;
}

mcsync_status_t mcsync_lock(mcsync_t* mcsync)
{
#if defined(MyHTML_FORCE_SPINLOCK)
    while (!mcsync_atomic_compare_exchange(&mcsync->spinlock, 0, 1)) {}
#else
    mcsync_mutex_lock(mcsync);
#endif
    
    return MCSYNC_STATUS_OK;
}

mcsync_status_t mcsync_unlock(mcsync_t* mcsync)
{
#if defined(MyHTML_FORCE_SPINLOCK)
    mcsync_atomic_store(&mcsync->spinlock, 0);
#else
    mcsync_mutex_unlock(mcsync);
#endif
    
    return MCSYNC_STATUS_OK;
}

mcsync_status_t mcsync_mutex_lock(mcsync_t* mcsync)
{
#if !defined(MyHTML_FORCE_SPINLOCK)
    if(mcsync->mutex == NULL) {
        mcsync->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        
        if(mcsync->mutex == NULL)
            return MCSYNC_STATUS_ERROR_MEM_ALLOCATE;
        
        pthread_mutex_init(mcsync->mutex, NULL);
    }
    
    if(pthread_mutex_lock(mcsync->mutex) == 0)
        return MCSYNC_STATUS_OK;
    else
        return MCSYNC_STATUS_NOT_OK;
#else
    return MCSYNC_STATUS_NOT_OK;
#endif
}

mcsync_status_t mcsync_mutex_unlock(mcsync_t* mcsync)
{
#if !defined(MyHTML_FORCE_SPINLOCK)
    if(pthread_mutex_unlock(mcsync->mutex) == 0)
        return MCSYNC_STATUS_OK;
    else
        return MCSYNC_STATUS_NOT_OK;
#else
    return MCSYNC_STATUS_NOT_OK;
#endif
}


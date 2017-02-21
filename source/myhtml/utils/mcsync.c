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

#include "myhtml/utils/mcsync.h"

#if !defined(MyHTML_BUILD_WITHOUT_THREADS) && ((defined(__GNUC__) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || \
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
    
#if !defined(MyHTML_BUILD_WITHOUT_THREADS) && !defined(MyHTML_FORCE_SPINLOCK)
    if(mcsync->mutex) {
        pthread_mutex_destroy(mcsync->mutex);
        myhtml_free(mcsync->mutex);
    }
#endif
    
    if(destroy_self)
        myhtml_free(mcsync);
    
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
#elif !defined(MyHTML_BUILD_WITHOUT_THREADS)
    mcsync_mutex_lock(mcsync);
#endif
    
    return MCSYNC_STATUS_OK;
}

mcsync_status_t mcsync_unlock(mcsync_t* mcsync)
{
#if defined(MyHTML_FORCE_SPINLOCK)
    mcsync_atomic_store(&mcsync->spinlock, 0);
#elif !defined(MyHTML_BUILD_WITHOUT_THREADS)
    mcsync_mutex_unlock(mcsync);
#endif
    
    return MCSYNC_STATUS_OK;
}

mcsync_status_t mcsync_mutex_lock(mcsync_t* mcsync)
{
#if !defined(MyHTML_BUILD_WITHOUT_THREADS) && !defined(MyHTML_FORCE_SPINLOCK)
    if(mcsync->mutex == NULL) {
        mcsync->mutex = (pthread_mutex_t*)myhtml_malloc(sizeof(pthread_mutex_t));
        
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
#if !defined(MyHTML_BUILD_WITHOUT_THREADS) && !defined(MyHTML_FORCE_SPINLOCK)
    if(pthread_mutex_unlock(mcsync->mutex) == 0)
        return MCSYNC_STATUS_OK;
    else
        return MCSYNC_STATUS_NOT_OK;
#else
    return MCSYNC_STATUS_NOT_OK;
#endif
}


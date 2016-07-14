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

#ifndef MyHTML_UTILS_MCSYNC_H
#define MyHTML_UTILS_MCSYNC_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
    
#include <myhtml/myosi.h>
    
#if !defined(MyHTML_BUILD_WITHOUT_THREADS)
#if defined(IS_OS_WINDOWS)
    typedef CRITICAL_SECTION pthread_mutex_t;
    typedef unsigned long pthread_mutexattr_t;
#else
#	include <pthread.h>
#endif
#endif

enum mcsync_status {
    MCSYNC_STATUS_OK                 = 0,
    MCSYNC_STATUS_NOT_OK             = 1,
    MCSYNC_STATUS_ERROR_MEM_ALLOCATE = 2
}
typedef mcsync_status_t;

struct mcsync {
    int spinlock;
#if !defined(MyHTML_BUILD_WITHOUT_THREADS)
    pthread_mutex_t *mutex;
#endif
}
typedef mcsync_t;

mcsync_t * mcsync_create(void);
mcsync_status_t mcsync_init(mcsync_t* mcsync);
void mcsync_clean(mcsync_t* mcsync);
mcsync_t * mcsync_destroy(mcsync_t* mcsync, int destroy_self);

mcsync_status_t mcsync_lock(mcsync_t* mclock);
mcsync_status_t mcsync_unlock(mcsync_t* mclock);

mcsync_status_t mcsync_mutex_lock(mcsync_t* mclock);
mcsync_status_t mcsync_mutex_unlock(mcsync_t* mclock);

#if !defined(MyHTML_BUILD_WITHOUT_THREADS) && defined(IS_OS_WINDOWS)
    static int pthread_mutex_lock(pthread_mutex_t *mutex);
    static int pthread_mutex_unlock(pthread_mutex_t *mutex);
    static int pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *a);
    static int pthread_mutex_destroy(pthread_mutex_t *m);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* mcsync_h */

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

#ifndef MyCORE_UTILS_MCSYNC_H
#define MyCORE_UTILS_MCSYNC_H
#pragma once

#include <mycore/myosi.h>

#ifdef __cplusplus
extern "C" {
#endif

enum mcsync_status {
    MCSYNC_STATUS_OK                 = 0x00,
    MCSYNC_STATUS_NOT_OK             = 0x01,
    MCSYNC_STATUS_ERROR_MEM_ALLOCATE = 0x02
}
typedef mcsync_status_t;

struct mcsync {
    int* spinlock;
    void* mutex;
}
typedef mcsync_t;

mcsync_t * mcsync_create(void);
mcsync_status_t mcsync_init(mcsync_t* mcsync);
void mcsync_clean(mcsync_t* mcsync);
mcsync_t * mcsync_destroy(mcsync_t* mcsync, int destroy_self);

mcsync_status_t mcsync_lock(mcsync_t* mcsync);
mcsync_status_t mcsync_unlock(mcsync_t* mcsync);

#ifndef MyCORE_BUILD_WITHOUT_THREADS
mcsync_status_t mcsync_spin_lock(void* spinlock);
mcsync_status_t mcsync_spin_unlock(void* spinlock);

mcsync_status_t mcsync_mutex_lock(void* mutex);
mcsync_status_t mcsync_mutex_try_lock(void* mutex);
mcsync_status_t mcsync_mutex_unlock(void* mutex);

void * mcsync_spin_create(void);
mcsync_status_t mcsync_spin_init(void* spinlock);
void mcsync_spin_clean(void* spinlock);
void mcsync_spin_destroy(void* spinlock);

void * mcsync_mutex_create(void);
mcsync_status_t mcsync_mutex_init(void* mutex);
void mcsync_mutex_clean(void* mutex);
void mcsync_mutex_destroy(void* mutex);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* mcsync_h */

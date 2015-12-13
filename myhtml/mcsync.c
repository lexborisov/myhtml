//
//  mcsync.c
//  myhtml
//
//  Created by Alexander Borisov on 24.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "mcsync.h"

static inline int atomic_compare_exchange(int* ptr, int compare, int exchange)
{
    return __atomic_compare_exchange_n(ptr, &compare, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static inline void atomic_store(int* ptr, int value)
{
    __atomic_store_n(ptr, 0, __ATOMIC_SEQ_CST);
}

mcsync_t * mcsync_create(void)
{
    mcsync_t* mcsync = malloc(sizeof(mcsync_t));
    
    mcsync_init(mcsync);
    
    return mcsync;
}

void mcsync_init(mcsync_t* mcsync)
{
    mcsync_clean(mcsync);
}

mcsync_t * mcsync_destroy(mcsync_t* mcsync, int destroy_self)
{
    if(mcsync == NULL)
        return NULL;
    
    if(destroy_self)
        free(mcsync);
    
    return NULL;
}

void mcsync_clean(mcsync_t* mcsync)
{
    mcsync->spinlock = 0;
}

void mcsync_lock(mcsync_t* mcsync)
{
    while (!atomic_compare_exchange(&mcsync->spinlock, 0, 1)) {}
}

void mcsync_unlock(mcsync_t* mcsync)
{
    atomic_store(&mcsync->spinlock, 0);
}



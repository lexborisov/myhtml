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

#include "mcsync.h"

static int atomic_compare_exchange(int* ptr, int compare, int exchange)
{
    return __atomic_compare_exchange_n(ptr, &compare, exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static void atomic_store(int* ptr, int value)
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



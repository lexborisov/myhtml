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

#include "mycore/utils/mcsync.h"

mcsync_t * mcsync_create(void)
{
    return mycore_calloc(1, sizeof(mcsync_t));
}



mcsync_status_t mcsync_init(mcsync_t* mcsync)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    /* spinlock */
    if((mcsync->spinlock = mcsync_spin_create()) == NULL)
        return MCSYNC_STATUS_NOT_OK;
    
    mcsync_status_t status = mcsync_spin_init(mcsync->spinlock);
    if(status) {
        mcsync_spin_destroy(mcsync->spinlock);
        return status;
    }
    
    /* mutex */
    if((mcsync->mutex = mcsync_mutex_create()) == NULL)
        return MCSYNC_STATUS_NOT_OK;
    
    if((status = mcsync_mutex_init(mcsync->mutex))) {
        mcsync_spin_destroy(mcsync->spinlock);
        mcsync_mutex_destroy(mcsync->mutex);
        
        return status;
    }
#endif
    
    return MCSYNC_STATUS_OK;
}

void mcsync_clean(mcsync_t* mcsync)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    mcsync_spin_clean(mcsync->spinlock);
    mcsync_mutex_clean(mcsync->mutex);
#endif
}

mcsync_t * mcsync_destroy(mcsync_t* mcsync, int destroy_self)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    if(mcsync == NULL)
        return NULL;
    
    mcsync_spin_destroy(mcsync->spinlock);
    mcsync_mutex_destroy(mcsync->mutex);
#endif
    if(destroy_self) {
        mycore_free(mcsync);
        return NULL;
    }
    
    return mcsync;
}

mcsync_status_t mcsync_lock(mcsync_t* mcsync)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    return mcsync_spin_lock(mcsync->spinlock);
#else
    return MCSYNC_STATUS_OK;
#endif
}

mcsync_status_t mcsync_unlock(mcsync_t* mcsync)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    return mcsync_spin_unlock(mcsync->spinlock);
#else
    return MCSYNC_STATUS_OK;
#endif
}

//
//  mcsync.h
//  myhtml
//
//  Created by Alexander Borisov on 24.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef mcsync_h
#define mcsync_h

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

struct mcsync {
    int spinlock;
}
typedef mcsync_t;


mcsync_t * mcsync_create(void);
void mcsync_init(mcsync_t* mcsync);
void mcsync_clean(mcsync_t* mcsync);
mcsync_t * mcsync_destroy(mcsync_t* mcsync, int destroy_self);

void mcsync_lock(mcsync_t* mclock);
void mcsync_unlock(mcsync_t* mclock);


#endif /* mcsync_h */

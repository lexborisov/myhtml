//
//  mcobject.h
//  myhtml
//
//  Created by Alexander Borisov on 27.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef mcobject_h
#define mcobject_h

#include "myosi.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef struct mcobject mcobject_t;
typedef void (*mcobject_callback_new_f)(mcobject_t* mcobject, size_t idx);

struct mcobject {
    void**  obj;
    
    void*   nodes;
    size_t  nodes_length;
    size_t  nodes_size;
    
    size_t* cache;
    size_t  cache_length;
    volatile size_t last_length;
    
    size_t  struct_size;
    
    mcobject_callback_new_f new_func;
};

mcobject_t * mcobject_create(size_t size, size_t struct_size, void* obj, mcobject_callback_new_f callback_new);
void mcobject_clean(mcobject_t* mcobject);
mcobject_t * mcobject_destroy(mcobject_t* mcobject);

size_t mcobject_malloc(mcobject_t* mcobject);
void mcobject_free(mcobject_t* mcobject, size_t idx);

void mcobject_malloc_segment(mcobject_t* mcobject, size_t *list, size_t count);
void mcobject_free_segment(mcobject_t* mcobject, size_t *list, size_t length);

void mcobject_call_callback_new(mcobject_t* mcobject, size_t idx);
size_t mcobject_length(mcobject_t* mcobject);

#endif /* mcobject_h */


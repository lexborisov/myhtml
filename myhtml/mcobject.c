//
//  mcobject.c
//  myhtml
//
//  Created by Alexander Borisov on 27.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "mcobject.h"


mcobject_t * mcobject_create(size_t size, size_t struct_size, void* obj, mcobject_callback_new_f callback_new)
{
    mcobject_t* mcobject = (mcobject_t*)malloc(sizeof(mcobject_t));
    
    mcobject->struct_size = struct_size;
    mcobject->nodes_size = size;
    
    mcobject->new_func    = callback_new;
    mcobject->nodes       = malloc(size * struct_size);
    mcobject->cache       = (size_t*)malloc(size * sizeof(size_t));
    mcobject->obj         = obj;
    
    *mcobject->obj = mcobject->nodes;
    
    mcobject_clean(mcobject);
    
    mcobject->last_length = mcobject->nodes_length;
    memset(&mcobject->nodes[mcobject->nodes_length], 0, mcobject->struct_size);
    
    return mcobject;
}

void mcobject_clean(mcobject_t* mcobject)
{
    mcobject->nodes_length = 0;
    mcobject->cache_length = 0;
}

mcobject_t * mcobject_destroy(mcobject_t* mcobject)
{
    if(mcobject == NULL)
        return NULL;
    
    if(mcobject->nodes)
        free(mcobject->nodes);
    
    if(mcobject->cache)
        free(mcobject->cache);
    
    return NULL;
}

size_t mcobject_length(mcobject_t* mcobject)
{
    return mcobject->nodes_length;
}

size_t mcobject_malloc(mcobject_t* mcobject)
{
    if(mcobject->nodes_length == mcobject->nodes_size)
    {
        if(mcobject->cache_length > 0) {
            mcobject->cache_length--;
            return mcobject->cache[ mcobject->cache_length ];
        }
        else {
            mcobject->nodes_size <<= 1;
            mcobject->nodes = myrealloc(mcobject->nodes,
                                        mcobject->nodes_size * mcobject->struct_size);
            
            mcobject->cache = (size_t*)myrealloc(mcobject->cache,
                                                 mcobject->nodes_size * sizeof(size_t));
            
            *mcobject->obj = mcobject->nodes;
        }
    }
    
    size_t set_idx = mcobject->nodes_length;
    mcobject->nodes_length++;
    
    if(mcobject->new_func && mcobject->last_length < mcobject->nodes_length) {
        mcobject->last_length = mcobject->nodes_length;
        mcobject->new_func(mcobject, set_idx);
    }
    
    return set_idx;
}

void mcobject_free(mcobject_t* mcobject, size_t idx)
{
    mcobject->cache[ mcobject->cache_length ] = idx;
    mcobject->cache_length++;
}



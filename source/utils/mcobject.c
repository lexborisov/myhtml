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

#include "mcobject.h"


mcobject_t * mcobject_create(size_t size, size_t struct_size, void* obj, mcobject_callback_new_f callback_new)
{
    mcobject_t* mcobject = (mcobject_t*)mymalloc(sizeof(mcobject_t));
    
    mcobject->struct_size = struct_size;
    mcobject->nodes_size = size;
    
    mcobject->new_func    = callback_new;
    mcobject->nodes       = calloc(size, struct_size);
    mcobject->cache       = (size_t*)mymalloc(size * sizeof(size_t));
    mcobject->obj         = obj;
    
    *mcobject->obj = mcobject->nodes;
    
    mcobject_clean(mcobject);
    
    mcobject->last_length = mcobject->nodes_length;
    
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
    
    free(mcobject);
    
    return NULL;
}

size_t mcobject_length(mcobject_t* mcobject)
{
    return mcobject->nodes_length;
}

void mcobject_call_callback_new(mcobject_t* mcobject, size_t idx)
{
    if(mcobject->new_func && mcobject->last_length < idx) {
        mcobject->last_length = idx;
        mcobject->new_func(mcobject, idx);
    }
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

void mcobject_malloc_segment(mcobject_t* mcobject, size_t *list, size_t count)
{
    size_t need_size = (mcobject->nodes_length + count);
    
    if(need_size >= mcobject->nodes_size)
    {
        if(mcobject->cache_length >= count) {
            mcobject->cache_length -= count;
            
            memcpy(list, &mcobject->cache[mcobject->cache_length],
                   sizeof(size_t) * count);
            
            return;
        }
        else if(mcobject->cache_length)
        {
            memcpy(list, mcobject->cache, sizeof(size_t) * mcobject->cache_length);
            
            mcobject->cache_length = 0;
            count -= mcobject->cache_length;
        }
        
        // TODO: (4096 * 2) -- is strange
        mcobject->nodes_size = (need_size + (4096 * 2));
        mcobject->nodes = myrealloc(mcobject->nodes,
                                    mcobject->nodes_size * mcobject->struct_size);
        
        mcobject->cache = (size_t*)myrealloc(mcobject->cache,
                                             mcobject->nodes_size * sizeof(size_t));
        
        *mcobject->obj = mcobject->nodes;
    }
    
    size_t i;
    for (i = 0; i < count; i++) {
        list[i] = mcobject->nodes_length;
        mcobject->nodes_length++;
    }
    
    return;
}

void mcobject_free(mcobject_t* mcobject, size_t idx)
{
    mcobject->cache[ mcobject->cache_length ] = idx;
    mcobject->cache_length++;
}

void mcobject_free_segment(mcobject_t* mcobject, size_t *list, size_t length)
{
    memcpy(&mcobject->cache[mcobject->cache_length],
           list, sizeof(size_t) * length);
    mcobject->cache_length += length;
}





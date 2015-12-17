//
//  mcobject_async.h
//  myhtml
//
//  Created by Alexander Borisov on 25.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef mcobject_async_h
#define mcobject_async_h

#include "myosi.h"
#include "mcsync.h"

struct mcobject_async_chunk {
    void  *begin;
    size_t length;
    size_t size;
    
    size_t prev;
}
typedef mcobject_async_chunk_t;

struct mcobject_async_node {
    size_t chunk;
    
    void  **cache;
    size_t  cache_size;
    size_t  cache_length;
    
    size_t upto;
}
typedef mcobject_async_node_t;

struct mcobject_async {
    void  **mem;
    size_t  mem_pos;
    size_t  mem_pos_length;
    size_t  mem_pos_size;
    
    size_t  mem_length;
    size_t  mem_size;
    
    size_t  struct_size;
    
    size_t *chunk_cache;
    size_t  chunk_cache_size;
    size_t  chunk_cache_length;
    
    mcobject_async_chunk_t *chunks;
    size_t chunks_size;
    size_t chunks_length;
    
    mcobject_async_node_t *nodes;
    size_t nodes_length;
    size_t nodes_size;
    
    mcsync_t *mcsync;
}
typedef mcobject_async_t;

mcobject_async_t * mcobject_async_create(size_t pos_size, size_t size, size_t struct_size);
void mcobject_async_init(mcobject_async_t *mcobj_async, size_t pos_size, size_t size, size_t struct_size);

void mcobject_async_clean(mcobject_async_t *mcobj_async);
mcobject_async_t * mcobject_async_destroy(mcobject_async_t *mcobj_async, int destroy_self);

size_t mcobject_async_node_add(mcobject_async_t *mcobj_async, size_t length);

void * mcobject_async_malloc(mcobject_async_t *mcobj_async, size_t node_idx);
void mcobject_async_free(mcobject_async_t *mcobj_async, size_t node_idx, void *entry);
size_t mcobject_async_chunk_malloc_without_lock(mcobject_async_t *mcobj_async, size_t length);

#endif /* mcobject_async_h */





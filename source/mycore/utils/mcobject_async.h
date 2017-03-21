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

#ifndef MyCORE_UTILS_MCOBJECT_ASYNC_H
#define MyCORE_UTILS_MCOBJECT_ASYNC_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mycore/myosi.h"
#include "mycore/utils/mcsync.h"

enum mcobject_async_status {
    MCOBJECT_ASYNC_STATUS_OK                                  = 0,
    MCOBJECT_ASYNC_STATUS_ERROR_MEMORY_ALLOCATION             = 1,
    MCOBJECT_ASYNC_STATUS_CHUNK_ERROR_MEMORY_ALLOCATION       = 2,
    MCOBJECT_ASYNC_STATUS_CHUNK_CACHE_ERROR_MEMORY_ALLOCATION = 3,
    MCOBJECT_ASYNC_STATUS_NODES_ERROR_MEMORY_ALLOCATION       = 4,
    MCOBJECT_ASYNC_STATUS_NODES_ERROR_BAD_NODE_ID             = 5,
    MCOBJECT_ASYNC_STATUS_CACHE_ERROR_MEMORY_REALLOC          = 6
}
typedef mcobject_async_status_t;

struct mcobject_async_chunk {
    unsigned char *begin;
    size_t length;
    size_t size;
    
    struct mcobject_async_chunk *next;
    struct mcobject_async_chunk *prev;
}
typedef mcobject_async_chunk_t;

struct mcobject_async_node {
    mcobject_async_chunk_t *chunk;
    
    void  **cache;
    size_t  cache_size;
    size_t  cache_length;
}
typedef mcobject_async_node_t;

struct mcobject_async {
    size_t  origin_size;
    size_t  struct_size;
    size_t  struct_size_sn;
    
    mcobject_async_chunk_t **chunk_cache;
    size_t  chunk_cache_size;
    size_t  chunk_cache_length;
    
    mcobject_async_chunk_t **chunks;
    size_t chunks_pos_size;
    size_t chunks_pos_length;
    size_t chunks_size;
    size_t chunks_length;
    
    mcobject_async_node_t *nodes;
    size_t nodes_length;
    size_t nodes_size;
    
    size_t *nodes_cache;
    size_t nodes_cache_length;
    size_t nodes_cache_size;
    
    mcsync_t *mcsync;
}
typedef mcobject_async_t;

mcobject_async_t * mcobject_async_create(void);
mcobject_async_status_t mcobject_async_init(mcobject_async_t *mcobj_async, size_t chunk_len, size_t obj_size_by_one_chunk, size_t struct_size);

void mcobject_async_clean(mcobject_async_t *mcobj_async);
mcobject_async_t * mcobject_async_destroy(mcobject_async_t *mcobj_async, int destroy_self);

size_t mcobject_async_node_add(mcobject_async_t *mcobj_async, mcobject_async_status_t *status);
void mcobject_async_node_clean(mcobject_async_t *mcobj_async, size_t node_idx);
void mcobject_async_node_all_clean(mcobject_async_t *mcobj_async);
void mcobject_async_node_delete(mcobject_async_t *mcobj_async, size_t node_idx);

void * mcobject_async_malloc(mcobject_async_t *mcobj_async, size_t node_idx, mcobject_async_status_t *status);
mcobject_async_status_t mcobject_async_free(mcobject_async_t *mcobj_async, void *entry);

mcobject_async_chunk_t * mcobject_async_chunk_malloc(mcobject_async_t *mcobj_async, size_t length, mcobject_async_status_t *status);
mcobject_async_chunk_t * mcobject_async_chunk_malloc_without_lock(mcobject_async_t *mcobj_async, size_t length, mcobject_async_status_t *status);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* mcobject_async_h */





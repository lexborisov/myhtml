/*
 Copyright 2015-2016 Alexander Borisov
 
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

#include "myhtml/utils/mcobject_async.h"

mcobject_async_t * mcobject_async_create(void)
{
    return (mcobject_async_t*)mycalloc(1, sizeof(mcobject_async_t));
}

mcobject_async_status_t mcobject_async_chunk_up(mcobject_async_t *mcobj_async)
{
    mcobj_async->chunks_length = 0;
    
    if(mcobj_async->chunks[ mcobj_async->chunks_pos_length ] == NULL) {
        mcobj_async->chunks[ mcobj_async->chunks_pos_length ] = (mcobject_async_chunk_t*)mycalloc(mcobj_async->chunks_size, sizeof(mcobject_async_chunk_t));
        
        if(mcobj_async->chunks[ mcobj_async->chunks_pos_length ] == NULL)
            return MCOBJECT_ASYNC_STATUS_CHUNK_ERROR_MEMORY_ALLOCATION;
    }
    
    return MCOBJECT_ASYNC_STATUS_OK;
}

mcobject_async_status_t mcobject_async_init(mcobject_async_t *mcobj_async, size_t chunk_len, size_t obj_size_by_one_chunk, size_t struct_size)
{
    mcobj_async->origin_size      = obj_size_by_one_chunk;
    mcobj_async->struct_size      = struct_size;
    mcobj_async->struct_size_sn   = struct_size + sizeof(size_t);
    
    mcobj_async->chunks_pos_length = 0;
    mcobj_async->chunks_pos_size   = 128;
    mcobj_async->chunks_size       = chunk_len;
    mcobj_async->chunks            = (mcobject_async_chunk_t**)mycalloc(mcobj_async->chunks_pos_size, sizeof(mcobject_async_chunk_t*));
    
    if(mcobj_async->chunks == NULL)
        return MCOBJECT_ASYNC_STATUS_CHUNK_ERROR_MEMORY_ALLOCATION;
    
    mcobject_async_chunk_up(mcobj_async);
    
    mcobj_async->chunk_cache_size = mcobj_async->chunks_size;
    mcobj_async->chunk_cache      = (mcobject_async_chunk_t**)mycalloc(mcobj_async->chunk_cache_size, sizeof(mcobject_async_chunk_t*));
    
    if(mcobj_async->chunk_cache == NULL)
        return MCOBJECT_ASYNC_STATUS_CHUNK_CACHE_ERROR_MEMORY_ALLOCATION;
    
    mcobj_async->nodes_length     = 0;
    mcobj_async->nodes_size       = 64;
    mcobj_async->nodes            = (mcobject_async_node_t*)mycalloc(mcobj_async->nodes_size, sizeof(mcobject_async_node_t));
    
    if(mcobj_async->nodes == NULL)
        return MCOBJECT_ASYNC_STATUS_NODES_ERROR_MEMORY_ALLOCATION;
    
    mcobj_async->nodes_cache_length     = 0;
    mcobj_async->nodes_cache_size       = mcobj_async->nodes_size;
    mcobj_async->nodes_cache            = (size_t*)mymalloc(mcobj_async->nodes_cache_size * sizeof(size_t));
    
    if(mcobj_async->nodes_cache == NULL)
        return MCOBJECT_ASYNC_STATUS_NODES_ERROR_MEMORY_ALLOCATION;
    
    mcobject_async_clean(mcobj_async);
    
    mcobj_async->mcsync = mcsync_create();
    
    return MCOBJECT_ASYNC_STATUS_OK;
}

void mcobject_async_clean(mcobject_async_t *mcobj_async)
{
    if(mcobj_async->chunks[0] != NULL)
        mcobj_async->chunks_pos_length = 1;
    else
        mcobj_async->chunks_pos_length = 0;
    
    mcobj_async->chunks_length       = 0;
    mcobj_async->chunk_cache_length  = 0;
    
    size_t node_idx;
    for (node_idx = 0; node_idx < mcobj_async->nodes_length; node_idx++)
    {
        mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
        node->cache_length = 0;
        
        node->chunk = mcobject_async_chunk_malloc(mcobj_async, mcobj_async->origin_size, NULL);
        node->chunk->prev = NULL;
    }
}

mcobject_async_t * mcobject_async_destroy(mcobject_async_t *mcobj_async, int destroy_self)
{
    if(mcobj_async == NULL)
        return NULL;
    
    if(mcobj_async->nodes)
    {
        for (size_t node_idx = 0; node_idx < mcobj_async->nodes_length; node_idx++)
        {
            mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
            
            if(node->cache)
                free(node->cache);
        }
        
        free(mcobj_async->nodes);
    }
    
    if(mcobj_async->nodes_cache) {
        free(mcobj_async->nodes_cache);
    }
    
    if(mcobj_async->chunks) {
        for (size_t pos_idx = 0; pos_idx < mcobj_async->chunks_pos_length; pos_idx++) {
            if(mcobj_async->chunks[pos_idx])
            {
                for (size_t idx = 0; idx < mcobj_async->chunks_size; idx++) {
                    if(mcobj_async->chunks[pos_idx][idx].begin)
                        free(mcobj_async->chunks[pos_idx][idx].begin);
                }
                
                free(mcobj_async->chunks[pos_idx]);
            }
        }
        
        free(mcobj_async->chunks);
    }
    
    if(mcobj_async->chunk_cache) {
        free(mcobj_async->chunk_cache);
    }
    
    mcobj_async->mcsync = mcsync_destroy(mcobj_async->mcsync, 1);
    
    memset(mcobj_async, 0, sizeof(mcobject_async_t));
    
    if(destroy_self)
        free(mcobj_async);
    else
        return mcobj_async;
    
    return NULL;
}

mcobject_async_status_t mcobject_async_mem_malloc(mcobject_async_t *mcobj_async, mcobject_async_chunk_t *chunk, size_t length)
{
    if(chunk->begin) {
        if(length > chunk->size) {
            free(chunk->begin);
            
            chunk->size = length + mcobj_async->origin_size;
            chunk->begin = (char*)mymalloc(chunk->size * mcobj_async->struct_size_sn);
        }
    }
    else {
        chunk->size = mcobj_async->origin_size;
        
        if(length > chunk->size)
            chunk->size += length;
        
        chunk->begin = (char*)mymalloc(chunk->size * mcobj_async->struct_size_sn);
    }
    
    chunk->length = 0;
    
    if(chunk->begin == NULL)
        return MCOBJECT_ASYNC_STATUS_CHUNK_ERROR_MEMORY_ALLOCATION;
    
    return MCOBJECT_ASYNC_STATUS_OK;
}

mcobject_async_chunk_t * mcobject_async_chunk_malloc_without_lock(mcobject_async_t *mcobj_async, size_t length, mcobject_async_status_t *status)
{
    if(mcobj_async->chunks_length >= mcobj_async->chunks_size)
    {
        if(mcobj_async->chunks_pos_length >= mcobj_async->chunks_pos_size)
        {
            size_t tmp_pos_size = mcobj_async->chunks_pos_size << 1;
            mcobject_async_chunk_t **tmp_pos = myrealloc(mcobj_async->chunks,
                                                      sizeof(mcobject_async_chunk_t*) * tmp_pos_size);
            
            if(tmp_pos)
            {
                memset(&tmp_pos[mcobj_async->chunks_pos_length], 0, (tmp_pos_size - mcobj_async->chunks_pos_length)
                       * sizeof(mcobject_async_chunk_t*));
                
                mcobj_async->chunks_pos_size = tmp_pos_size;
                mcobj_async->chunks = tmp_pos;
            }
            else {
                if(status)
                    *status = MCOBJECT_ASYNC_STATUS_CHUNK_ERROR_MEMORY_ALLOCATION;
                
                return NULL;
            }
        }
        
        if(mcobject_async_chunk_up(mcobj_async)) {
            if(status)
                *status = MCOBJECT_ASYNC_STATUS_CHUNK_ERROR_MEMORY_ALLOCATION;
            
            return NULL;
        }
        
        mcobj_async->chunks_pos_length++;
    }
    
    mcobject_async_chunk_t* chunk = &mcobj_async->chunks[mcobj_async->chunks_pos_length - 1][mcobj_async->chunks_length];
    mcobj_async->chunks_length++;
    
    if(status)
        *status = mcobject_async_mem_malloc(mcobj_async, chunk, length);
    else
        mcobject_async_mem_malloc(mcobj_async, chunk, length);
    
    return chunk;
}

mcobject_async_chunk_t * mcobject_async_chunk_malloc(mcobject_async_t *mcobj_async, size_t length, mcobject_async_status_t *status)
{
    mcsync_lock(mcobj_async->mcsync);
    mcobject_async_chunk_t* chunk = mcobject_async_chunk_malloc_without_lock(mcobj_async, length, status);
    mcsync_unlock(mcobj_async->mcsync);
    
    return chunk;
}

size_t mcobject_async_node_add(mcobject_async_t *mcobj_async, mcobject_async_status_t *status)
{
    mcsync_lock(mcobj_async->mcsync);
    
    size_t node_idx;
    
    if(mcobj_async->nodes_cache_length) {
        mcobj_async->nodes_cache_length--;
        
        node_idx = mcobj_async->nodes_cache[ mcobj_async->nodes_cache_length ];
    }
    else {
        if(mcobj_async->nodes_length >= mcobj_async->nodes_size) {
            mcsync_unlock(mcobj_async->mcsync);
            return 0;
        }
        
        node_idx = mcobj_async->nodes_length;
        mcobj_async->nodes_length++;
    }
    
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    
    if(status) {
        node->chunk = mcobject_async_chunk_malloc_without_lock(mcobj_async, mcobj_async->origin_size, status);
        
        if(*status) {
            mcsync_unlock(mcobj_async->mcsync);
            return 0;
        }
    }
    else {
        mcobject_async_status_t mystatus;
        node->chunk = mcobject_async_chunk_malloc_without_lock(mcobj_async, mcobj_async->origin_size, &mystatus);
        
        if(mystatus) {
            mcsync_unlock(mcobj_async->mcsync);
            return 0;
        }
    }
    
    node->chunk->next = NULL;
    node->chunk->prev = NULL;
    
    node->cache_length = 0;
    node->cache_size = mcobj_async->origin_size;
    node->cache = (char**)mymalloc(sizeof(char*) * node->cache_size);
    
    if(node->cache == NULL) {
        if(status)
            *status = MCOBJECT_ASYNC_STATUS_CHUNK_CACHE_ERROR_MEMORY_ALLOCATION;
        
        mcsync_unlock(mcobj_async->mcsync);
        return 0;
    }
    
    mcsync_unlock(mcobj_async->mcsync);
    
    return node_idx;
}

void mcobject_async_node_clean(mcobject_async_t *mcobj_async, size_t node_idx)
{
    if(mcobj_async->nodes_length <= node_idx)
        return;
    
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    node->cache_length = 0;
    
    if(node->chunk == NULL)
        return;
    
    while (node->chunk->prev)
        node->chunk = node->chunk->prev;
    
    node->chunk->length = 0;
    node->cache_length  = 0;
}

void mcobject_async_node_all_clean(mcobject_async_t *mcobj_async)
{
    for (size_t node_idx = 0; node_idx < mcobj_async->nodes_length; node_idx++) {
        mcobject_async_node_clean(mcobj_async, node_idx);
    }
}

void mcobject_async_node_delete(mcobject_async_t *mcobj_async, size_t node_idx)
{
    mcsync_lock(mcobj_async->mcsync);
    
    if(mcobj_async->nodes_length <= node_idx) {
        mcsync_unlock(mcobj_async->mcsync);
        return;
    }
    
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    mcobject_async_chunk_t *chunk = node->chunk;
    
    while (chunk->next)
        chunk = chunk->next;
    
    while (chunk)
    {
        if(mcobj_async->chunk_cache_length >= mcobj_async->chunk_cache_size) {
            mcobj_async->chunk_cache_size <<= 1;
            
            mcobject_async_chunk_t **tmp = (mcobject_async_chunk_t**)myrealloc(mcobj_async->chunk_cache,
                                                                               sizeof(mcobject_async_chunk_t*) * mcobj_async->chunk_cache_size);
            
            if(tmp)
                mcobj_async->chunk_cache = tmp;
        }
        
        mcobj_async->chunk_cache[ mcobj_async->chunk_cache_length ] = chunk;
        mcobj_async->chunk_cache_length++;
        
        chunk = chunk->prev;
    }
    
    if(node->cache)
        free(node->cache);
    
    memset(node, 0, sizeof(mcobject_async_node_t));
    
    if(mcobj_async->nodes_cache_length >= mcobj_async->nodes_cache_size) {
        size_t new_size = mcobj_async->nodes_cache_size << 1;
        
        size_t *tmp = (size_t*)myrealloc(mcobj_async->nodes_cache, sizeof(size_t) * mcobj_async->nodes_cache_size);
        
        if(tmp) {
            mcobj_async->nodes_cache = tmp;
            mcobj_async->nodes_cache_size = new_size;
        }
    }
    
    mcobj_async->nodes_cache[ mcobj_async->nodes_cache_length ] = node_idx;
    mcobj_async->nodes_cache_length++;
    
    mcsync_unlock(mcobj_async->mcsync);
}

void * mcobject_async_malloc(mcobject_async_t *mcobj_async, size_t node_idx, mcobject_async_status_t *status)
{
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    
    if(node->cache_length) {
        if(status)
            *status = MCOBJECT_ASYNC_STATUS_OK;
        
        node->cache_length--;
        return node->cache[ node->cache_length ];
    }
    
    if(node->chunk->length >= node->chunk->size)
    {
        if(node->chunk->next) {
            node->chunk = node->chunk->next;
            node->chunk->length = 0;
        }
        else {
            mcobject_async_status_t mystatus;
            mcobject_async_chunk_t *chunk = mcobject_async_chunk_malloc(mcobj_async, mcobj_async->origin_size, &mystatus);
            
            if(mystatus) {
                if(status)
                    *status = mystatus;
                
                return NULL;
            }
            
            chunk->prev = node->chunk;
            node->chunk->next = chunk;
            
            node->chunk = chunk;
        }
    }
    
    if(status)
        *status = MCOBJECT_ASYNC_STATUS_OK;
    
    size_t offset = node->chunk->length * mcobj_async->struct_size_sn;
    *((size_t*)(&node->chunk->begin[offset])) = node_idx;
    
    node->chunk->length++;
    return &node->chunk->begin[(offset + sizeof(size_t))];
}

void mcobject_async_free(mcobject_async_t *mcobj_async, void *entry)
{
    size_t node_idx = *((size_t*)((char*)entry - sizeof(size_t)));
    
    if(node_idx >= mcobj_async->nodes_length)
        return;
    
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    
    if(node->cache_length > node->cache_size) {
        node->cache_size <<= 1;
        
        char **tmp = (char**)myrealloc(node->cache, sizeof(char*) * node->cache_size);
        
        if(tmp)
            node->cache = tmp;
    }
    
    node->cache[ node->cache_length ] = entry;
    node->cache_length++;
}


//
//  mcobject_async.c
//  myhtml
//
//  Created by Alexander Borisov on 25.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "mcobject_async.h"

mcobject_async_t * mcobject_async_create(size_t pos_size, size_t size, size_t struct_size)
{
    mcobject_async_t *mcobj_async = (mcobject_async_t*)calloc(1, sizeof(mcobject_async_t));
    
    mcobject_async_init(mcobj_async, pos_size, size, struct_size);
    
    return mcobj_async;
}

void mcobject_async_init(mcobject_async_t *mcobj_async, size_t pos_size, size_t size, size_t struct_size)
{
    mcobj_async->mem_size         = size;
    mcobj_async->mem_pos_size     = pos_size;
    
    mcobj_async->mem              = (void**)mymalloc(sizeof(void*) * size);
    mcobj_async->mem[0]           = mymalloc(struct_size * size);
    
    mcobj_async->struct_size      = struct_size;
    
    mcobj_async->chunks_size      = 128;
    mcobj_async->chunks           = (mcobject_async_chunk_t*)mymalloc(sizeof(mcobject_async_chunk_t) *
                                                                 mcobj_async->chunks_size);
    
    mcobj_async->chunk_cache_size = mcobj_async->chunks_size;
    mcobj_async->chunk_cache      = (size_t*)mymalloc(sizeof(size_t) * mcobj_async->chunk_cache_size);
    
    mcobj_async->nodes_length     = 0;
    mcobj_async->nodes_size       = 64;
    mcobj_async->nodes            = (mcobject_async_node_t*)mymalloc(sizeof(mcobject_async_node_t) *
                                                                mcobj_async->nodes_size);
    
    mcobject_async_clean(mcobj_async);
    
    mcobj_async->mcsync = mcsync_create();
}

void mcobject_async_mem_malloc(mcobject_async_t *mcobj_async, mcobject_async_chunk_t *chunk, size_t length)
{
    size_t alloc_mem = (mcobj_async->mem_length + length);
    
    if(alloc_mem >= mcobj_async->mem_size)
    {
        size_t set_size = alloc_mem - mcobj_async->mem_size;
        
        if(set_size == 0)
        {
            mcobj_async->mem_pos = mcobj_async->mem_pos_length;
            mcobj_async->mem_pos_length++;
            
            if(mcobj_async->mem_pos_length >= mcobj_async->mem_pos_size)
            {
                mcobj_async->mem_pos_size <<= 1;
                
                void **tmp = (void**)myrealloc(mcobj_async->mem,
                                      sizeof(void*) * mcobj_async->mem_pos_size);
                
                if(tmp)
                    mcobj_async->mem = tmp;
            }
            
            size_t size_init = mcobj_async->mem_size;
            if(length > size_init)
                size_init = length;
            
            mcobj_async->mem_length = 0;
            mcobj_async->mem[mcobj_async->mem_pos] = mycalloc(size_init, mcobj_async->struct_size);
        }
        else {
            length = mcobj_async->mem_size - mcobj_async->mem_length;
        }
    }
    
    unsigned char *tmp = (unsigned char*)mcobj_async->mem[mcobj_async->mem_pos];
    
    chunk->begin  = &tmp[mcobj_async->mem_length * mcobj_async->struct_size];
    chunk->length = 0;
    chunk->size   = length;
    
    mcobj_async->mem_length += length;
    
}

size_t mcobject_async_chunk_malloc_without_lock(mcobject_async_t *mcobj_async, size_t length)
{
    if(mcobj_async->chunks_length >= mcobj_async->chunks_size)
    {
        mcobj_async->chunks_size <<= 1;
        mcobject_async_chunk_t *tmp = myrealloc(mcobj_async->chunks,
                                              sizeof(mcobject_async_chunk_t) * mcobj_async->chunks_size);
        
        if(tmp)
            mcobj_async->chunks = tmp;
        //else
    }
    
    size_t chunk_idx = mcobj_async->chunks_length;
    mcobj_async->chunks_length++;
    
    mcobject_async_mem_malloc(mcobj_async, &mcobj_async->chunks[chunk_idx], length);
    
    return chunk_idx;
}

size_t mcobject_async_chunk_malloc(mcobject_async_t *mcobj_async, size_t length)
{
    mcsync_lock(mcobj_async->mcsync);
    size_t chunk_idx = mcobject_async_chunk_malloc_without_lock(mcobj_async, length);
    mcsync_unlock(mcobj_async->mcsync);
    
    return chunk_idx;
}

size_t mcobject_async_node_add(mcobject_async_t *mcobj_async, size_t length)
{
    mcsync_lock(mcobj_async->mcsync);
    
    if(mcobj_async->nodes_length >= mcobj_async->nodes_size)
        return 0;
    
    size_t node_idx = mcobj_async->nodes_length;
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    
    node->chunk = mcobject_async_chunk_malloc_without_lock(mcobj_async, length);
    
    mcobj_async->chunks[node->chunk].prev = 0;
    
    node->cache_length = 0;
    node->cache_size = length + 1;
    node->cache = (void**)mymalloc(sizeof(void*) * node->cache_size);
    
    node->upto = length;
    
    mcobj_async->nodes_length++;
    
    mcsync_unlock(mcobj_async->mcsync);
    
    return node_idx;
}

size_t mcobject_async_node_delete(mcobject_async_t *mcobj_async, size_t node_idx)
{
    mcsync_lock(mcobj_async->mcsync);
    
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    mcobject_async_chunk_t *chunks = mcobj_async->chunks;
    
    size_t chunk_idx = node->chunk;
    
    while (chunk_idx)
    {
        if(mcobj_async->chunk_cache_length >= mcobj_async->chunk_cache_size) {
            mcobj_async->chunk_cache_size <<= 1;
            
            size_t *tmp = (size_t*)myrealloc(mcobj_async->chunk_cache, sizeof(size_t) * mcobj_async->chunk_cache_size);
            
            if(tmp)
                mcobj_async->chunk_cache = tmp;
        }
        
        mcobj_async->chunk_cache[ mcobj_async->chunk_cache_length ] = chunk_idx;
        mcobj_async->chunk_cache_length++;
        
        chunk_idx = chunks[chunk_idx].prev;
    }
    
    if(node->cache)
        free(node->cache);
    
    mcobj_async->nodes_length--;
    
    mcsync_unlock(mcobj_async->mcsync);
    
    return node_idx;
}

void * mcobject_async_malloc(mcobject_async_t *mcobj_async, size_t node_idx)
{
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    mcobject_async_chunk_t *chunk = &mcobj_async->chunks[node->chunk];
    
    if(chunk->length >= chunk->size)
    {
        if(node->cache_length) {
            node->cache_length--;
            return node->cache[ node->cache_length ];
        }
        
        size_t new_chunk_idx = mcobject_async_chunk_malloc(mcobj_async, node->upto);
        chunk = &mcobj_async->chunks[new_chunk_idx];
        
        chunk->prev = node->chunk;
        node->chunk = new_chunk_idx;
    }
    
    char *tmp = (char*)chunk->begin;
    
    size_t offset = chunk->length * mcobj_async->struct_size;
    void *next_entry = &tmp[offset];
    chunk->length++;
    
    return next_entry;
}

void mcobject_async_free(mcobject_async_t *mcobj_async, size_t node_idx, void *entry)
{
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    
    if(node->cache_length >= node->cache_size) {
        node->cache_size <<= 1;
        
        void **tmp = (void**)myrealloc(node->cache, sizeof(size_t) * node->cache_size);
        
        if(tmp)
            node->cache = tmp;
    }
    
    node->cache[ node->cache_length ] = entry;
    node->cache_length++;
}

void mcobject_async_clean(mcobject_async_t *mcobj_async)
{
    mcobj_async->mem_length          = 0;
    mcobj_async->chunks_length       = 1;
    mcobj_async->chunk_cache_length  = 0;
    mcobj_async->mem_pos             = 0;
    mcobj_async->mem_pos_length      = 1;
    
    size_t node_idx;
    for (node_idx = 0; node_idx < mcobj_async->nodes_length; node_idx++)
    {
        mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
        node->cache_length = 0;
        
        node->chunk = mcobject_async_chunk_malloc(mcobj_async, node->upto);
        mcobj_async->chunks[node->chunk].prev = 0;
    }
}

mcobject_async_t * mcobject_async_destroy(mcobject_async_t *mcobj_async, int destroy_self)
{
    if(mcobj_async == NULL)
        return NULL;
    
    if(mcobj_async->nodes)
    {
        size_t node_idx;
        for (node_idx = 0; node_idx < mcobj_async->nodes_length; node_idx++)
        {
            mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
            
            if(node->cache)
                free(node->cache);
        }
        
        free(mcobj_async->nodes);
        mcobj_async->nodes = NULL;
    }
    
    if(mcobj_async->mem) {
        size_t idx;
        for (idx = 0; idx < mcobj_async->mem_pos_length; idx++) {
            free(mcobj_async->mem[idx]);
        }
        
        free(mcobj_async->mem);
        mcobj_async->mem = NULL;
    }
    
    if(mcobj_async->chunks) {
        free(mcobj_async->chunks);
        mcobj_async->chunks = NULL;
    }
    
    mcobj_async->mcsync = mcsync_destroy(mcobj_async->mcsync, 1);
    
    if(destroy_self)
        free(mcobj_async);
    else
        return mcobj_async;
    
    return NULL;
}





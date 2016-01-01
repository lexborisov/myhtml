//
//  mcobject_async.c
//  myhtml
//
//  Created by Alexander Borisov on 25.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "mcobject_async.h"

mcobject_async_t * mcobject_async_create(size_t chunk_len, size_t obj_size_by_one_chunk, size_t struct_size)
{
    mcobject_async_t *mcobj_async = (mcobject_async_t*)calloc(1, sizeof(mcobject_async_t));
    
    mcobject_async_init(mcobj_async, chunk_len, obj_size_by_one_chunk, struct_size);
    
    return mcobj_async;
}

void mcobject_async_chunk_up(mcobject_async_t *mcobj_async)
{
    mcobj_async->chunks_length = 0;
    
    if(mcobj_async->chunks[ mcobj_async->chunks_pos_length ] == NULL) {
        mcobj_async->chunks[ mcobj_async->chunks_pos_length ] = (mcobject_async_chunk_t*)mycalloc(mcobj_async->chunks_size, sizeof(mcobject_async_chunk_t));
    }
    
    mcobj_async->chunks_pos_length++;
}

void mcobject_async_init(mcobject_async_t *mcobj_async, size_t chunk_len, size_t obj_size_by_one_chunk, size_t struct_size)
{
    mcobj_async->origin_size      = obj_size_by_one_chunk;
    mcobj_async->struct_size      = struct_size;
    
    mcobj_async->chunks_pos_length = 0;
    mcobj_async->chunks_pos_size   = 128;
    mcobj_async->chunks_size       = chunk_len;
    mcobj_async->chunks            = (mcobject_async_chunk_t**)mycalloc(mcobj_async->chunks_pos_size, sizeof(mcobject_async_chunk_t*));
    
    mcobject_async_chunk_up(mcobj_async);
    
    mcobj_async->chunk_cache_size = mcobj_async->chunks_size;
    mcobj_async->chunk_cache      = (mcobject_async_chunk_t**)mycalloc(mcobj_async->chunk_cache_size, sizeof(mcobject_async_chunk_t*));
    
    mcobj_async->nodes_length     = 0;
    mcobj_async->nodes_size       = 64;
    mcobj_async->nodes            = (mcobject_async_node_t*)mycalloc(mcobj_async->nodes_size, sizeof(mcobject_async_node_t));
    
    mcobject_async_clean(mcobj_async);
    
    mcobj_async->mcsync = mcsync_create();
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
        
        node->chunk = mcobject_async_chunk_malloc(mcobj_async, mcobj_async->origin_size);
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
        mcobj_async->nodes = NULL;
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
        mcobj_async->chunks = NULL;
    }
    
    if(mcobj_async->chunk_cache) {
        free(mcobj_async->chunk_cache);
        mcobj_async->chunk_cache = NULL;
    }
    
    mcobj_async->mcsync = mcsync_destroy(mcobj_async->mcsync, 1);
    
    if(destroy_self)
        free(mcobj_async);
    else
        return mcobj_async;
    
    return NULL;
}

void mcobject_async_mem_malloc(mcobject_async_t *mcobj_async, mcobject_async_chunk_t *chunk, size_t length)
{
    if(chunk->begin) {
        if(length > chunk->size) {
            free(chunk->begin);
            
            chunk->size = length + mcobj_async->origin_size;
            chunk->begin = (char*)mymalloc(chunk->size * mcobj_async->struct_size);
        }
    }
    else {
        chunk->size = mcobj_async->origin_size;
        
        if(length > chunk->size)
            chunk->size += length;
        
        chunk->begin = (char*)mymalloc(chunk->size * mcobj_async->struct_size);
    }
    
    chunk->length = 0;
}

mcobject_async_chunk_t * mcobject_async_chunk_malloc_without_lock(mcobject_async_t *mcobj_async, size_t length)
{
    if(mcobj_async->chunks_length >= mcobj_async->chunks_size)
    {
        if(mcobj_async->chunks_pos_length >= mcobj_async->chunks_pos_size)
        {
            mcobj_async->chunks_pos_size <<= 1;
            mcobject_async_chunk_t **tmp_pos = myrealloc(mcobj_async->chunks,
                                                      sizeof(mcobject_async_chunk_t*) * mcobj_async->chunks_pos_size);
            
            if(tmp_pos) {
                memset(&tmp_pos[mcobj_async->chunks_pos_length], 0, (mcobj_async->chunks_pos_size - mcobj_async->chunks_pos_length)
                       * sizeof(mcobject_async_chunk_t*));
                
                mcobj_async->chunks = tmp_pos;
            }
        }
        
        mcobject_async_chunk_up(mcobj_async);
    }
    
    mcobject_async_chunk_t* chunk = &mcobj_async->chunks[mcobj_async->chunks_pos_length - 1][mcobj_async->chunks_length];
    mcobj_async->chunks_length++;
    
    mcobject_async_mem_malloc(mcobj_async, chunk, length);
    
    return chunk;
}

mcobject_async_chunk_t * mcobject_async_chunk_malloc(mcobject_async_t *mcobj_async, size_t length)
{
    mcsync_lock(mcobj_async->mcsync);
    mcobject_async_chunk_t* chunk = mcobject_async_chunk_malloc_without_lock(mcobj_async, length);
    mcsync_unlock(mcobj_async->mcsync);
    
    return chunk;
}

size_t mcobject_async_node_add(mcobject_async_t *mcobj_async)
{
    mcsync_lock(mcobj_async->mcsync);
    
    if(mcobj_async->nodes_length >= mcobj_async->nodes_size) {
        mcsync_unlock(mcobj_async->mcsync);
        return 0;
    }
    
    size_t node_idx = mcobj_async->nodes_length;
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    
    node->chunk = mcobject_async_chunk_malloc_without_lock(mcobj_async, mcobj_async->origin_size);
    
    node->chunk->next = NULL;
    node->chunk->prev = NULL;
    
    node->cache_length = 0;
    node->cache_size = mcobj_async->origin_size;
    node->cache = (char**)mymalloc(sizeof(char*) * node->cache_size);
    
    mcobj_async->nodes_length++;
    
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
    
    mcobj_async->nodes_length--;
    
    mcsync_unlock(mcobj_async->mcsync);
}

void * mcobject_async_malloc(mcobject_async_t *mcobj_async, size_t node_idx)
{
    mcobject_async_node_t *node = &mcobj_async->nodes[node_idx];
    
    if(node->chunk->length >= node->chunk->size)
    {
        if(node->cache_length) {
            node->cache_length--;
            return node->cache[ node->cache_length ];
        }
        
        if(node->chunk->next) {
            node->chunk = node->chunk->next;
            node->chunk->length = 0;
        }
        else {
            mcobject_async_chunk_t *chunk = mcobject_async_chunk_malloc(mcobj_async, mcobj_async->origin_size);
            
            chunk->prev = node->chunk;
            node->chunk->next = chunk;
            
            node->chunk = chunk;
        }
    }
    
    size_t offset = node->chunk->length * mcobj_async->struct_size;
    node->chunk->length++;
    
    return &node->chunk->begin[offset];
}

void mcobject_async_free(mcobject_async_t *mcobj_async, size_t node_idx, void *entry)
{
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

void mcobject_async_test_nodes(mcobject_async_t *mcobj_async, size_t node_count)
{
    size_t good = 0, to_count = 4097, loop;
    
    for(loop = 0; loop < to_count; loop++)
    {
        for (size_t node_id = 0; node_id < node_count; node_id++)
        {
            mcobject_async_node_t* test_struct = (mcobject_async_node_t*)mcobject_async_malloc(mcobj_async, node_id);
            
            if(test_struct)
            {
                test_struct->chunk        = NULL;
                test_struct->cache_size   = 3;
                test_struct->cache_length = 4;
                test_struct->cache        = (void*)5;
                
                good++;
            }
        }
    }
    
    size_t res = (to_count * node_count);
    fprintf(stderr, "Result: good: %lu; bad: %lu\n", good, (res - good));
    
    // test is new param
    mcobject_async_clean(mcobj_async);
    
    //mcobject_async_node_clean(mcobj_async, 0);
    //mcobject_async_node_clean(mcobj_async, 1);
    
    good = 0;
    to_count = 4097 * 10;
    
    for(loop = 0; loop < to_count; loop++)
    {
        for (size_t node_id = 0; node_id < node_count; node_id++)
        {
            mcobject_async_node_t* test_struct = (mcobject_async_node_t*)mcobject_async_malloc(mcobj_async, node_id);
            
            if(test_struct->cache        == (void*)5 &&
               test_struct->cache_length == 4 &&
               test_struct->cache_size   == 3 &&
               test_struct->chunk        == NULL)
            {
                good++;
            }
        }
    }
    
    fprintf(stderr, "Result: good: %lu; bad: %lu\n", good, res * good);
    fprintf(stderr, "Total: current length: %lu\n", mcobj_async->chunks_pos_length);
}

void mcobject_async_test(void)
{
    mcobject_async_t *mcobj_async = mcobject_async_create(4096, 4096 * 6, sizeof(mcobject_async_node_t));
    
    mcobject_async_node_add(mcobj_async);
    mcobject_async_node_add(mcobj_async);
    
    mcobject_async_test_nodes(mcobj_async, 2);
    
    mcobject_async_destroy(mcobj_async, 1);
}



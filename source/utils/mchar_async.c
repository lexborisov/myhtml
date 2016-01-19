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

#include "mchar_async.h"

mchar_async_t * mchar_async_create(size_t pos_size, size_t size)
{
    mchar_async_t *mcobj_async = (mchar_async_t*)calloc(1, sizeof(mchar_async_t));
    
    mchar_async_init(mcobj_async, pos_size, size);
    
    return mcobj_async;
}

void mchar_async_init(mchar_async_t *mchar_async, size_t chunk_len, size_t char_size)
{
    if(char_size < 4096)
        char_size = 4096;
    
    mchar_async->origin_size      = char_size;
    
    mchar_async->chunks_size      = chunk_len;
    mchar_async->chunks_pos_size  = 1024;
    mchar_async->chunks           = (mchar_async_chunk_t**)mycalloc(mchar_async->chunks_pos_size, sizeof(mchar_async_chunk_t*));
    mchar_async->chunks[0]        = (mchar_async_chunk_t*)mycalloc(mchar_async->chunks_size, sizeof(mchar_async_chunk_t));
    
    mchar_async_cache_init(&mchar_async->chunk_cache);
    
    mchar_async->nodes_length     = 0;
    mchar_async->nodes_size       = 64;
    mchar_async->nodes            = (mchar_async_node_t*)mycalloc(mchar_async->nodes_size, sizeof(mchar_async_node_t));
    
    mchar_async_clean(mchar_async);
    
    mchar_async->mcsync = mcsync_create();
}

void mchar_async_clean(mchar_async_t *mchar_async)
{
    mchar_async->chunks_length      = 0;
    mchar_async->chunks_pos_length  = 1;
    
    mchar_async_cache_clean(&mchar_async->chunk_cache);
    
    for (size_t node_idx = 0; node_idx < mchar_async->nodes_length; node_idx++)
    {
        mchar_async_node_t *node = &mchar_async->nodes[node_idx];
        mchar_async_cache_clean(&node->cache);
        
        node->chunk = mchar_async_chunk_malloc(mchar_async, node, mchar_async->origin_size);
        node->chunk->prev = 0;
    }
}

mchar_async_t * mchar_async_destroy(mchar_async_t *mchar_async, int destroy_self)
{
    if(mchar_async == NULL)
        return NULL;
    
    if(mchar_async->nodes)
    {
        for (size_t node_idx = 0; node_idx < mchar_async->nodes_length; node_idx++)
        {
            mchar_async_node_t *node = &mchar_async->nodes[node_idx];
            mchar_async_cache_destroy(&node->cache, myfalse);
        }
        
        free(mchar_async->nodes);
        mchar_async->nodes = NULL;
    }
    
    if(mchar_async->chunks)
    {
        for (size_t pos_idx = 0; pos_idx < mchar_async->chunks_pos_length; pos_idx++) {
            if(mchar_async->chunks[pos_idx])
            {
                for (size_t idx = 0; idx < mchar_async->chunks_size; idx++) {
                    if(mchar_async->chunks[pos_idx][idx].begin)
                        free(mchar_async->chunks[pos_idx][idx].begin);
                }
                
                free(mchar_async->chunks[pos_idx]);
            }
        }
        
        free(mchar_async->chunks);
        mchar_async->chunks = NULL;
    }
    
    mchar_async_cache_destroy(&mchar_async->chunk_cache, myfalse);
    
    mchar_async->mcsync = mcsync_destroy(mchar_async->mcsync, 1);
    
    if(destroy_self)
        free(mchar_async);
    else
        return mchar_async;
    
    return NULL;
}

void mchar_async_mem_malloc(mchar_async_t *mchar_async, mchar_async_node_t *node, mchar_async_chunk_t *chunk, size_t length)
{
    if(chunk == NULL)
        return;
    
    if(chunk->begin) {
        if(length > chunk->size) {
            free(chunk->begin);
            
            chunk->size = length + mchar_async->origin_size;
            chunk->begin = (char*)mymalloc(chunk->size * sizeof(char));
        }
    }
    else {
        chunk->size = mchar_async->origin_size;
        
        if(length > chunk->size)
            chunk->size += length;
        
        chunk->begin = (char*)mymalloc(chunk->size * sizeof(char));
    }
    
    chunk->length = 0;
}

mchar_async_chunk_t * mchar_async_chunk_malloc_without_lock(mchar_async_t *mchar_async, mchar_async_node_t *node, size_t length)
{
    if(mchar_async_cache_has_nodes(mchar_async->chunk_cache))
    {
        size_t index = mchar_async_cache_delete(&mchar_async->chunk_cache, length);
        
        if(index)
            return (mchar_async_chunk_t*)mchar_async->chunk_cache.nodes[index].value;
    }
    
    if(mchar_async->chunks_length >= mchar_async->chunks_size)
    {
        size_t current_idx = mchar_async->chunks_pos_length;
        mchar_async->chunks_pos_length++;
        
        if(mchar_async->chunks_pos_length > mchar_async->chunks_pos_size)
        {
            mchar_async->chunks_pos_size <<= 1;
            mchar_async_chunk_t **tmp_pos = myrealloc(mchar_async->chunks,
                                                      sizeof(mchar_async_chunk_t*) * mchar_async->chunks_pos_size);
            
            if(tmp_pos) {
                memset(&tmp_pos[mchar_async->chunks_pos_length], 0, (mchar_async->chunks_pos_size - mchar_async->chunks_pos_length)
                       * sizeof(mchar_async_chunk_t*));
                
                mchar_async->chunks = tmp_pos;
            }
        }
        
        if(mchar_async->chunks[current_idx] == NULL) {
            mchar_async_chunk_t *tmp = mymalloc(sizeof(mchar_async_chunk_t) * mchar_async->chunks_size);
            
            if(tmp)
                mchar_async->chunks[current_idx] = tmp;
        }
        
        mchar_async->chunks_length = 0;
    }
    
    mchar_async_chunk_t *chunk = &mchar_async->chunks[mchar_async->chunks_pos_length - 1][mchar_async->chunks_length];
    mchar_async->chunks_length++;
    
    mchar_async_mem_malloc(mchar_async, node, chunk, length);
    
    return chunk;
}

mchar_async_chunk_t * mchar_async_chunk_malloc(mchar_async_t *mchar_async, mchar_async_node_t *node, size_t length)
{
    mcsync_lock(mchar_async->mcsync);
    mchar_async_chunk_t *chunk = mchar_async_chunk_malloc_without_lock(mchar_async, node, length);
    mcsync_unlock(mchar_async->mcsync);
    
    return chunk;
}

size_t mchar_async_node_add(mchar_async_t *mchar_async)
{
    mcsync_lock(mchar_async->mcsync);
    
    if(mchar_async->nodes_length >= mchar_async->nodes_size) {
        mcsync_unlock(mchar_async->mcsync);
        return 0;
    }
    
    size_t node_idx = mchar_async->nodes_length;
    mchar_async_node_t *node = &mchar_async->nodes[node_idx];
    
    mchar_async_cache_init(&node->cache);
    
    node->chunk = mchar_async_chunk_malloc_without_lock(mchar_async, node, mchar_async->origin_size);
    
    node->chunk->next = NULL;
    node->chunk->prev = NULL;
    
    mchar_async->nodes_length++;
    
    mcsync_unlock(mchar_async->mcsync);
    
    return node_idx;
}

void mchar_async_node_clean(mchar_async_t *mchar_async, size_t node_idx)
{
    if(mchar_async->nodes_length <= node_idx)
        return;
    
    mchar_async_node_t *node = &mchar_async->nodes[node_idx];
    
    while (node->chunk->prev)
        node->chunk = node->chunk->prev;
    
    node->chunk->length = 0;
    mchar_async_cache_clean(&node->cache);
}

void mchar_async_node_delete(mchar_async_t *mchar_async, size_t node_idx)
{
    mcsync_lock(mchar_async->mcsync);
    
    if(mchar_async->nodes_length <= node_idx) {
        mcsync_unlock(mchar_async->mcsync);
        return;
    }
    
    mchar_async_node_t *node = &mchar_async->nodes[node_idx];
    mchar_async_chunk_t *chunk = node->chunk;
    
    while (chunk->next)
        chunk = chunk->next;
    
    while (chunk)
    {
        mchar_async_cache_add(&mchar_async->chunk_cache, (void*)chunk, chunk->size);
        chunk = chunk->prev;
    }
    
    if(node->cache.nodes)
        free(node->cache.nodes);
    
    mchar_async->nodes_length--;
    
    mcsync_unlock(mchar_async->mcsync);
}

mchar_async_chunk_t * mchar_sync_chunk_find_by_size(mchar_async_node_t *node, size_t size)
{
    mchar_async_chunk_t *chunk = node->chunk->next;
    
    while (chunk) {
        if(chunk->size >= size)
            return chunk;
        
        chunk = chunk->next;
    }
    
    return NULL;
}

void mchar_sync_chunk_insert_after(mchar_async_chunk_t *base, mchar_async_chunk_t *chunk)
{
    if(base->next == chunk)
        return;
    
    if(chunk->prev)
        chunk->prev->next = chunk->next;
    
    if(chunk->next)
        chunk->next->prev = chunk->prev;
    
    if(base->next)
        base->next->prev = chunk;
    
    chunk->next = base->next;
    chunk->prev = base;
    
    base->next = chunk;
}

char * mchar_async_malloc(mchar_async_t *mchar_async, size_t node_idx, size_t size)
{
    if(size == 0)
        return NULL;
    
    mchar_async_node_t *node = &mchar_async->nodes[node_idx];
    mchar_async_chunk_t *chunk = node->chunk;
    
    if(mchar_async_cache_has_nodes(node->cache)) {
        size_t index = mchar_async_cache_delete(&node->cache, size);
        
        if(index) {
            return (char *)(node->cache.nodes[index].value);
        }
    }
    
    size_t new_size = chunk->length + size + sizeof(size_t);
    
    if(new_size > chunk->size)
    {
        if((chunk->length + sizeof(size_t)) < chunk->size)
        {
            size_t size = chunk->size - chunk->length;
            
            char *tmp = &chunk->begin[(chunk->length + sizeof(size_t))];
            *(size_t*)(&chunk->begin[chunk->length]) = size;
            
            chunk->length = chunk->size;
            
            mchar_async_cache_add(&node->cache, tmp, size);
        }
        
        chunk = mchar_sync_chunk_find_by_size(node, size);
        
        if(chunk)
            chunk->length = 0;
        else {
            if(size > mchar_async->origin_size)
                size = (size + mchar_async->origin_size + sizeof(size_t));
            
            chunk = mchar_async_chunk_malloc(mchar_async, node, size);
        }
        
        mchar_sync_chunk_insert_after(node->chunk, chunk);
        node->chunk = chunk;
    }
    
    char *tmp = &chunk->begin[(chunk->length + sizeof(size_t))];
    *((size_t*)(&chunk->begin[chunk->length])) = size;
    
    chunk->length += size + sizeof(size_t);
    
    return tmp;
}

char * mchar_async_realloc(mchar_async_t *mchar_async, size_t node_idx, char *data, size_t data_len, size_t new_size)
{
    if(data == NULL)
        return NULL;
    
    size_t curr_size = *((size_t*)(data - sizeof(size_t)));
    
    if(curr_size >= new_size)
        return data;
    
    mchar_async_node_t *node = &mchar_async->nodes[node_idx];
    
    char *tmp = mchar_async_malloc(mchar_async, node_idx, new_size);
    
    if(tmp) {
        memcpy(tmp, data, sizeof(char) * data_len);
        
        mchar_async_cache_add(&node->cache, data, curr_size);
    }
    
    return tmp;
}

void mchar_async_free(mchar_async_t *mchar_async, size_t node_idx, char *entry)
{
    mchar_async_cache_add(&mchar_async->nodes[node_idx].cache, entry, *(size_t*)(entry - sizeof(size_t)));
}

void mchar_async_cache_init(mchar_async_cache_t *cache)
{
    cache->count        = 0;
    cache->nodes_root   = 0;
    cache->nodes_length = 1;
    cache->nodes_size   = 1024;
    cache->nodes        = (mchar_async_cache_node_t*)mymalloc(sizeof(mchar_async_cache_node_t) * cache->nodes_size);
    
    cache->nodes[0].left  = 0;
    cache->nodes[0].right = 0;
    cache->nodes[0].size  = 0;
    cache->nodes[0].value = NULL;
    
    cache->index_length = 0;
    cache->index_size   = cache->nodes_size;
    cache->index = (size_t*)mymalloc(sizeof(size_t) * cache->index_size);
}

void mchar_async_cache_clean(mchar_async_cache_t *cache)
{
    cache->count        = 0;
    cache->nodes_root   = 0;
    cache->nodes_length = 1;
    cache->index_length = 0;
    
    if(cache->nodes) {
        cache->nodes[0].left  = 0;
        cache->nodes[0].right = 0;
        cache->nodes[0].size  = 0;
        cache->nodes[0].value = NULL;
    }
}

mchar_async_cache_t * mchar_async_cache_destroy(mchar_async_cache_t *cache, mybool_t self_destroy)
{
    if(cache == NULL)
        return NULL;
    
    if(cache->nodes)
        free(cache->nodes);
    
    if(cache->index)
        free(cache->index);
    
    if(self_destroy) {
        free(cache);
        return NULL;
    }
    
    return cache;
}

size_t mchar_async_cache_malloc(mchar_async_cache_t *cache)
{
    if(cache->index_length) {
        cache->index_length--;
        return cache->index[cache->index_length];
    }
    
    cache->nodes_length++;
    
    if(cache->nodes_length >= cache->nodes_size) {
        cache->nodes_size <<= 1;
        
        mchar_async_cache_node_t *tmp = (mchar_async_cache_node_t*)myrealloc(cache->nodes, sizeof(mchar_async_cache_node_t) * cache->nodes_size);
        
        if(tmp)
            cache->nodes = tmp;
    }
    
    return cache->nodes_length - 1;
}

size_t mchar_async_cache_delete(mchar_async_cache_t *cache, size_t size)
{
    mchar_async_cache_node_t *list = cache->nodes;
    size_t idx = cache->nodes_root;
    
    while (idx)
    {
        if(size <= list[idx].size)
        {
            // for a left
            size_t left = list[idx].left;
            
            list[left].parent = list[idx].parent;
            
            if(list[idx].parent) {
                if(list[ list[idx].parent ].left == idx) {
                    list[ list[idx].parent ].left = left;
                }
                else {
                    list[ list[idx].parent ].right = left;
                }
            }
            else {
                cache->nodes_root = left;
            }
            
            while(list[left].right)
                left = list[left].right;
            
            list[left].right = list[idx].right;
            
            list[ list[idx].right ].parent = left;
            
            cache->index[cache->index_length] = idx;
            
            cache->index_length++;
            if(cache->index_length >= cache->index_size)
            {
                cache->index_size <<= 1;
                size_t *tmp = (size_t*)myrealloc(cache->index, sizeof(size_t) * cache->index_size);
                
                if(tmp)
                    cache->index = tmp;
            }
            
            cache->count--;
            
            return idx;
        }
        else {
            idx = list[idx].right;
        }
    }
    
    return 0;
}

void mchar_async_cache_add(mchar_async_cache_t *cache, void* value, size_t size)
{
    cache->count++;
    
    if(cache->nodes_root == 0) {
        mchar_async_cache_node_t *list = cache->nodes;
        
        cache->nodes_root = mchar_async_cache_malloc(cache);
        
        list[cache->nodes_root].parent = 0;
        list[cache->nodes_root].left   = 0;
        list[cache->nodes_root].right  = 0;
        list[cache->nodes_root].size   = size;
        list[cache->nodes_root].value  = value;
        
        return;
    }
    
    size_t idx = cache->nodes_root;
    size_t new_idx = mchar_async_cache_malloc(cache);
    
    mchar_async_cache_node_t *list = cache->nodes;
    
    while(idx)
    {
        if(size == list[idx].size)
        {
            if(list[idx].parent)
            {
                if(list[ list[idx].parent ].left == idx) {
                    list[ list[idx].parent ].left = new_idx;
                }
                else {
                    list[ list[idx].parent ].right = new_idx;
                }
                
                list[new_idx].parent = list[idx].parent;
            }
            else {
                list[new_idx].parent = 0;
                cache->nodes_root = new_idx;
            }
            
            list[new_idx].right  = idx;
            list[new_idx].left   = 0;
            list[new_idx].size   = size;
            list[new_idx].value  = value;
            
            list[idx].parent = new_idx;
            
            break;
        }
        else if(size < list[idx].size)
        {
            if(list[idx].parent)
            {
                if(list[ list[idx].parent ].left == idx) {
                    list[ list[idx].parent ].left = new_idx;
                }
                else {
                    list[ list[idx].parent ].right = new_idx;
                }
                
                list[new_idx].parent = list[idx].parent;
            }
            else {
                list[new_idx].parent = 0;
                cache->nodes_root = new_idx;
            }
            
            list[new_idx].right  = idx;
            list[new_idx].left   = 0;
            list[new_idx].size   = size;
            list[new_idx].value  = value;
            
            list[idx].parent = new_idx;
            
            break;
        }
        else if(size > list[idx].size)
        {
            if(list[idx].right == 0)
            {
                list[idx].right = new_idx;
                
                list[new_idx].right  = 0;
                list[new_idx].left   = 0;
                list[new_idx].parent = idx;
                list[new_idx].size   = size;
                list[new_idx].value  = value;
                
                break;
            }
            else
                idx = list[idx].right;
        }
        else {
            if(list[idx].left == 0)
            {
                list[idx].right = new_idx;
                
                list[new_idx].right  = 0;
                list[new_idx].left   = 0;
                list[new_idx].parent = idx;
                list[new_idx].size   = size;
                list[new_idx].value  = value;
                
                break;
            }
            else
                idx = list[idx].left;
        }
    }
}



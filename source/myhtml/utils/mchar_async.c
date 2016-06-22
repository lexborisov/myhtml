/*
 Copyright (C) 2015-2016 Alexander Borisov
 
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

#include "myhtml/utils/mchar_async.h"

mchar_async_t * mchar_async_create(size_t pos_size, size_t size)
{
    mchar_async_t *mcobj_async = (mchar_async_t*)myhtml_calloc(1, sizeof(mchar_async_t));
    
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
    mchar_async->chunks           = (mchar_async_chunk_t**)myhtml_calloc(mchar_async->chunks_pos_size, sizeof(mchar_async_chunk_t*));
    mchar_async->chunks[0]        = (mchar_async_chunk_t*)myhtml_calloc(mchar_async->chunks_size, sizeof(mchar_async_chunk_t));
    
    mchar_async_cache_init(&mchar_async->chunk_cache);
    
    mchar_async->nodes_length     = 0;
    mchar_async->nodes_size       = 64;
    mchar_async->nodes            = (mchar_async_node_t*)myhtml_calloc(mchar_async->nodes_size, sizeof(mchar_async_node_t));
    
    mchar_async->nodes_cache_length = 0;
    mchar_async->nodes_cache_size   = mchar_async->nodes_size;
    mchar_async->nodes_cache        = (size_t*)myhtml_malloc(mchar_async->nodes_cache_size * sizeof(size_t));
    
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
            mchar_async_cache_destroy(&node->cache, false);
        }
        
        myhtml_free(mchar_async->nodes);
        mchar_async->nodes = NULL;
    }
    
    if(mchar_async->nodes_cache) {
        myhtml_free(mchar_async->nodes_cache);
    }
    
    if(mchar_async->chunks)
    {
        for (size_t pos_idx = 0; pos_idx < mchar_async->chunks_pos_length; pos_idx++) {
            if(mchar_async->chunks[pos_idx])
            {
                for (size_t idx = 0; idx < mchar_async->chunks_size; idx++) {
                    if(mchar_async->chunks[pos_idx][idx].begin)
                        myhtml_free(mchar_async->chunks[pos_idx][idx].begin);
                }
                
                myhtml_free(mchar_async->chunks[pos_idx]);
            }
        }
        
        myhtml_free(mchar_async->chunks);
        mchar_async->chunks = NULL;
    }
    
    mchar_async_cache_destroy(&mchar_async->chunk_cache, false);
    
    mchar_async->mcsync = mcsync_destroy(mchar_async->mcsync, 1);
    
    memset(mchar_async, 0, sizeof(mchar_async_t));
    
    if(destroy_self)
        myhtml_free(mchar_async);
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
            myhtml_free(chunk->begin);
            
            chunk->size = length + mchar_async->origin_size;
            chunk->begin = (char*)myhtml_malloc(chunk->size * sizeof(char));
        }
    }
    else {
        chunk->size = mchar_async->origin_size;
        
        if(length > chunk->size)
            chunk->size = length;
        
        chunk->begin = (char*)myhtml_malloc(chunk->size * sizeof(char));
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
        
        if(mchar_async->chunks_pos_length >= mchar_async->chunks_pos_size)
        {
            mchar_async->chunks_pos_size <<= 1;
            mchar_async_chunk_t **tmp_pos = myhtml_realloc(mchar_async->chunks,
                                                      sizeof(mchar_async_chunk_t*) * mchar_async->chunks_pos_size);
            
            if(tmp_pos) {
                memset(&tmp_pos[mchar_async->chunks_pos_length], 0, (mchar_async->chunks_pos_size - mchar_async->chunks_pos_length)
                       * sizeof(mchar_async_chunk_t*));
                
                mchar_async->chunks = tmp_pos;
            }
        }
        
        if(mchar_async->chunks[current_idx] == NULL) {
            mchar_async_chunk_t *tmp = myhtml_calloc(mchar_async->chunks_size, sizeof(mchar_async_chunk_t));
            
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
    
    size_t node_idx;
    
    if(mchar_async->nodes_cache_length) {
        mchar_async->nodes_cache_length--;
        
        node_idx = mchar_async->nodes_cache[ mchar_async->nodes_cache_length ];
    }
    else {
        if(mchar_async->nodes_length >= mchar_async->nodes_size) {
            mcsync_unlock(mchar_async->mcsync);
            return 0;
        }
        
        node_idx = mchar_async->nodes_length;
        mchar_async->nodes_length++;
    }
    
    mchar_async_node_t *node = &mchar_async->nodes[node_idx];
    
    mchar_async_cache_init(&node->cache);
    
    node->chunk = mchar_async_chunk_malloc_without_lock(mchar_async, node, mchar_async->origin_size);
    
    node->chunk->next = NULL;
    node->chunk->prev = NULL;
    
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
        mchar_async_cache_destroy(&node->cache, false);
    
    memset(node, 0, sizeof(mchar_async_node_t));
    
    if(mchar_async->nodes_cache_length >= mchar_async->nodes_cache_size) {
        size_t new_size = mchar_async->nodes_cache_size << 1;
        
        size_t *tmp = (size_t*)myhtml_realloc(mchar_async->nodes_cache, sizeof(size_t) * mchar_async->nodes_cache_size);
        
        if(tmp) {
            mchar_async->nodes_cache = tmp;
            mchar_async->nodes_cache_size = new_size;
        }
    }
    
    mchar_async->nodes_cache[ mchar_async->nodes_cache_length ] = node_idx;
    mchar_async->nodes_cache_length++;
    
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
            size_t size = (chunk->size - chunk->length) - sizeof(size_t);
            
            if(size) {
                char *tmp = &chunk->begin[(chunk->length + sizeof(size_t))];
                *(size_t*)(&chunk->begin[chunk->length]) = size;
                
                chunk->length = chunk->size;
                
                mchar_async_cache_add(&node->cache, tmp, size);
            }
        }
        
        chunk = mchar_sync_chunk_find_by_size(node, (size + sizeof(size_t)));
        
        if(chunk)
            chunk->length = 0;
        else {
            if((size + sizeof(size_t)) > mchar_async->origin_size)
                chunk = mchar_async_chunk_malloc(mchar_async, node, (size + sizeof(size_t) + mchar_async->origin_size));
            else
                chunk = mchar_async_chunk_malloc(mchar_async, node, mchar_async->origin_size);
        }
        
        mchar_sync_chunk_insert_after(node->chunk, chunk);
        node->chunk = chunk;
    }
    
    char *tmp = &chunk->begin[(chunk->length + sizeof(size_t))];
    *((size_t*)(&chunk->begin[chunk->length])) = size;
    
    chunk->length = chunk->length + size + sizeof(size_t);
    
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
    
    if(node->chunk->length >= curr_size &&
       &node->chunk->begin[ (node->chunk->length - curr_size) ] == data)
    {
        size_t next_size = (node->chunk->length - curr_size) + new_size;
        
        if(next_size <= node->chunk->size) {
            /* it`s Magic */
            *((size_t*)(&node->chunk->begin[ ((node->chunk->length - curr_size) - sizeof(size_t)) ])) = new_size;
            
            node->chunk->length = next_size;
            
            return data;
        }
//        else {
//            size_t re_size = next_size - node->chunk->length;
//            
//            /* a little Magic ;) */
//            *((size_t*)(&node->chunk->begin[ ((node->chunk->length - curr_size) - sizeof(size_t)) ])) = re_size;
//            
//            curr_size = re_size;
//        }
    }
    
    char *tmp = mchar_async_malloc(mchar_async, node_idx, new_size);
    
    if(tmp) {
        memcpy(tmp, data, sizeof(char) * data_len);
        
        mchar_async_cache_add(&node->cache, data, curr_size);
    }
    
    return tmp;
}

char * mchar_async_crop_first_chars(mchar_async_t *mchar_async, size_t node_idx, char *data, size_t crop_len)
{
    if(data == NULL)
        return NULL;
    
    size_t curr_size = *((size_t*)(data - sizeof(size_t)));
    
    char *tmp_old = data;
    data = &data[crop_len];
    
    *((size_t*)(data - sizeof(size_t))) = curr_size - crop_len;
    
    if((crop_len + 4) > sizeof(size_t)) {
        crop_len = crop_len - sizeof(size_t);
        *((size_t*)(tmp_old - sizeof(size_t))) = crop_len;
        
        mchar_async_node_t *node = &mchar_async->nodes[node_idx];
        mchar_async_cache_add(&node->cache, tmp_old, crop_len);
    }
    
    return data;
}

char * mchar_async_crop_first_chars_without_cache(char *data, size_t crop_len)
{
    if(data == NULL)
        return NULL;
    
    size_t curr_size = *((size_t*)(data - sizeof(size_t)));
    data = &data[crop_len];
    
    *((size_t*)(data - sizeof(size_t))) = curr_size - crop_len;
    
    return data;
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
    cache->nodes        = (mchar_async_cache_node_t*)myhtml_malloc(sizeof(mchar_async_cache_node_t) * cache->nodes_size);
    
    cache->nodes[0].left  = 0;
    cache->nodes[0].right = 0;
    cache->nodes[0].size  = 0;
    cache->nodes[0].value = NULL;
    
    cache->index_length = 0;
    cache->index_size   = cache->nodes_size;
    cache->index = (size_t*)myhtml_malloc(sizeof(size_t) * cache->index_size);
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

mchar_async_cache_t * mchar_async_cache_destroy(mchar_async_cache_t *cache, bool self_destroy)
{
    if(cache == NULL)
        return NULL;
    
    if(cache->nodes)
        myhtml_free(cache->nodes);
    
    if(cache->index)
        myhtml_free(cache->index);
    
    if(self_destroy) {
        myhtml_free(cache);
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
        
        mchar_async_cache_node_t *tmp = (mchar_async_cache_node_t*)myhtml_realloc(cache->nodes, sizeof(mchar_async_cache_node_t) * cache->nodes_size);
        
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
            while( list[ list[idx].right ].size == size )
                idx = list[idx].right;
            
            size_t parent = list[idx].parent;
            
            if(parent) {
                if(list[parent].left == idx)
                {
                    if(list[idx].right) {
                        if(list[idx].left) {
                            size_t last_left = list[ list[idx].right ].left;
                            
                            while( list[last_left].left )
                                last_left = list[last_left].left;
                            
                            if(last_left) {
                                list[last_left].left = list[idx].left;
                                list[ list[idx].left ].parent = last_left;
                            }
                            else {
                                list[ list[idx].right ].left = list[idx].left;
                            }
                        }
                        
                        list[parent].left = list[idx].right;
                        list[ list[idx].right ].parent = parent;
                    }
                    else {
                        list[parent].left = list[idx].left;
                        list[ list[idx].left ].parent = parent;
                    }
                }
                else {
                    if(list[idx].left) {
                        if(list[idx].right) {
                            size_t last_right = list[ list[idx].left ].right;
                            
                            while( list[last_right].right )
                                last_right = list[last_right].right;
                            
                            if(last_right) {
                                list[last_right].right = list[idx].right;
                                list[ list[idx].right ].parent = last_right;
                            }
                            else {
                                list[ list[idx].left ].right = list[idx].right;
                            }
                        }
                        
                        list[parent].right = list[idx].left;
                        list[ list[idx].left ].parent = parent;
                    }
                    else {
                        list[parent].right = list[idx].right;
                        list[ list[idx].right ].parent = parent;
                    }
                }
            }
            else {
                if(list[idx].left) {
                    if(list[idx].right) {
                        size_t last_right = list[ list[idx].left ].right;
                        
                        while( list[last_right].right )
                            last_right = list[last_right].right;
                        
                        if(last_right) {
                            list[last_right].right = list[idx].right;
                            list[ list[idx].right ].parent = last_right;
                        }
                        else {
                            list[ list[idx].left ].right = list[idx].right;
                        }
                    }
                    
                    cache->nodes_root = list[idx].left;
                    list[ list[idx].left ].parent = 0;
                }
                else {
                    cache->nodes_root = list[idx].right;
                    list[ list[idx].right ].parent = 0;
                }
            }
            
            cache->index[cache->index_length] = idx;
            
            cache->index_length++;
            if(cache->index_length >= cache->index_size)
            {
                size_t new_size = cache->index_size << 1;
                size_t *tmp = (size_t*)myhtml_realloc(cache->index, sizeof(size_t) * new_size);
                
                if(tmp) {
                    cache->index = tmp;
                    cache->index_size = new_size;
                }
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
            if(list[idx].right) {
                list[new_idx].right = list[idx].right;
                list[ list[idx].right ].parent = new_idx;
            }
            else {
                list[new_idx].right = 0;
            }
            
            list[idx].right = new_idx;
            
            list[new_idx].parent = idx;
            list[new_idx].left   = 0;
            list[new_idx].size   = size;
            list[new_idx].value  = value;
            
            break;
        }
        else if(size < list[idx].size)
        {
            size_t parent = list[idx].parent;
            
            if(parent) {
                if(list[parent].left == idx)
                    list[parent].left = new_idx;
                else
                    list[parent].right = new_idx;
                
                list[new_idx].parent = parent;
            }
            else {
                cache->nodes_root = new_idx;
                list[new_idx].parent = 0;
            }
            
            list[idx].parent = new_idx;
            
            list[new_idx].right  = idx;
            list[new_idx].left   = 0;
            list[new_idx].size   = size;
            list[new_idx].value  = value;
            
            break;
        }
        else // size > list[idx].size
        {
            if(list[idx].right)
                idx = list[idx].right;
            else {
                list[idx].right = new_idx;
                
                list[new_idx].right  = 0;
                list[new_idx].left   = 0;
                list[new_idx].parent = idx;
                list[new_idx].size   = size;
                list[new_idx].value  = value;
            
                break;
            }
        }
    }
}



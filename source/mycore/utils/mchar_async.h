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

#ifndef MyCORE_UTILS_MCHAR_ASYNC_H
#define MyCORE_UTILS_MCHAR_ASYNC_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mycore/myosi.h"
#include "mycore/utils/mcsync.h"

#define mchar_async_cache_has_nodes(cache) cache.count

typedef struct mchar_async_node mchar_async_node_t;

struct mchar_async_cache_node {
    void  *value;
    size_t size;
    
    size_t left;
    size_t right;
    size_t parent;
}
typedef mchar_async_cache_node_t;

struct mchar_async_chunk {
    char  *begin;
    size_t length;
    size_t size;
    
    struct mchar_async_chunk *next;
    struct mchar_async_chunk *prev;
}
typedef mchar_async_chunk_t;

struct mchar_async_cache {
    mchar_async_cache_node_t *nodes;
    size_t  nodes_size;
    size_t  nodes_length;
    size_t  nodes_root;
    
    size_t  count;
    
    size_t *index;
    size_t index_length;
    size_t index_size;
}
typedef mchar_async_cache_t;

struct mchar_async_node {
    mchar_async_chunk_t *chunk;
    mchar_async_cache_t cache;
};

struct mchar_async {
    size_t  origin_size;
    
    mchar_async_chunk_t **chunks;
    size_t chunks_pos_size;
    size_t chunks_pos_length;
    size_t chunks_size;
    size_t chunks_length;
    
    mchar_async_cache_t chunk_cache;
    
    mchar_async_node_t *nodes;
    size_t nodes_length;
    size_t nodes_size;
    
    size_t *nodes_cache;
    size_t nodes_cache_length;
    size_t nodes_cache_size;
    
    mcsync_t *mcsync;
}
typedef mchar_async_t;


mchar_async_t * mchar_async_create(void);
mystatus_t mchar_async_init(mchar_async_t *mchar_async, size_t chunk_len, size_t char_size);
mystatus_t mchar_async_clean(mchar_async_t *mchar_async);
mchar_async_t * mchar_async_destroy(mchar_async_t *mchar_async, int destroy_self);

char * mchar_async_malloc(mchar_async_t *mchar_async, size_t node_idx, size_t size);
char * mchar_async_realloc(mchar_async_t *mchar_async, size_t node_idx, char *data, size_t data_len, size_t new_size);
void mchar_async_free(mchar_async_t *mchar_async, size_t node_idx, char *entry);

size_t mchar_async_node_add(mchar_async_t *mchar_async, mystatus_t* status);
void mchar_async_node_clean(mchar_async_t *mchar_async, size_t node_idx);
void mchar_async_node_delete(mchar_async_t *mchar_async, size_t node_idx);

mchar_async_chunk_t * mchar_async_chunk_malloc(mchar_async_t *mchar_async, mchar_async_node_t *node, size_t length);
char * mchar_async_crop_first_chars(mchar_async_t *mchar_async, size_t node_idx, char *data, size_t crop_len);
char * mchar_async_crop_first_chars_without_cache(char *data, size_t crop_len);

size_t mchar_async_get_size_by_data(const char *data);

// cache
mystatus_t mchar_async_cache_init(mchar_async_cache_t *cache);
mchar_async_cache_t * mchar_async_cache_destroy(mchar_async_cache_t *cache, bool self_destroy);
void mchar_async_cache_clean(mchar_async_cache_t *cache);

void mchar_async_cache_add(mchar_async_cache_t *cache, void* value, size_t size);
size_t mchar_async_cache_delete(mchar_async_cache_t *cache, size_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* mchar_async_h */


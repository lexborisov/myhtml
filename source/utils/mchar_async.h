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

#ifndef MyHTML_UTILS_MCHAR_ASYNC_H
#define MyHTML_UTILS_MCHAR_ASYNC_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myosi.h"
#include "utils/mcsync.h"

#define mchar_async_cache_has_nodes(__cache__) __cache__.count

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


mchar_async_t * mchar_async_create(size_t pos_size, size_t size);
void mchar_async_init(mchar_async_t *mchar_async, size_t chunk_len, size_t char_size);
void mchar_async_clean(mchar_async_t *mchar_async);
mchar_async_t * mchar_async_destroy(mchar_async_t *mchar_async, int destroy_self);

char * mchar_async_malloc(mchar_async_t *mchar_async, size_t node_idx, size_t size);
char * mchar_async_realloc(mchar_async_t *mchar_async, size_t node_idx, char *data, size_t data_len, size_t new_size);
void mchar_async_free(mchar_async_t *mchar_async, size_t node_idx, char *entry);

size_t mchar_async_node_add(mchar_async_t *mchar_async);
void mchar_async_node_clean(mchar_async_t *mchar_async, size_t node_idx);
void mchar_async_node_delete(mchar_async_t *mchar_async, size_t node_idx);

mchar_async_chunk_t * mchar_async_chunk_malloc(mchar_async_t *mchar_async, mchar_async_node_t *node, size_t length);

// cache
void mchar_async_cache_init(mchar_async_cache_t *cache);
mchar_async_cache_t * mchar_async_cache_destroy(mchar_async_cache_t *cache, mybool_t self_destroy);
void mchar_async_cache_clean(mchar_async_cache_t *cache);

void mchar_async_cache_add(mchar_async_cache_t *cache, void* value, size_t size);
size_t mchar_async_cache_delete(mchar_async_cache_t *cache, size_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* mchar_async_h */


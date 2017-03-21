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

#ifndef MyCORE_UTILS_MCTREE_H
#define MyCORE_UTILS_MCTREE_H
#pragma once

#include <mycore/myosi.h>
#include <mycore/utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

#define mctree_node_get_free_id(mctree) mctree->nodes_length

#define mctree_node_clean(mctree, idx)      \
    mctree->nodes[idx].str          = NULL; \
    mctree->nodes[idx].value        = NULL; \
    mctree->nodes[idx].child_count  = 0;    \
    mctree->nodes[idx].prev         = 0;    \
    mctree->nodes[idx].next         = 0;    \
    mctree->nodes[idx].child        = 0

#define mctree_node_add(mctree)                                     \
    mctree->nodes_length++;                                         \
    if(mctree->nodes_length >= mctree->nodes_size) {                \
        mctree->nodes_size = mctree->nodes_length + 4096;           \
        mctree->nodes = (mctree_node_t*)mycore_realloc(mctree->nodes,    \
            sizeof(mctree_node_t) * mctree->nodes_size);            \
    }                                                               \
    mctree_node_clean(mctree, mctree->nodes_length)

#define mctree_make_first_idx(mctree, key, size) \
    ((mycore_string_chars_lowercase_map[ (const unsigned char)(key[0]) ] + mycore_string_chars_lowercase_map[ (const unsigned char)(key[size - 1]) ]) % mctree->start_size) + 1


typedef size_t mctree_index_t;

struct mctree_node {
    const char* str;
    size_t str_size;
    void* value;
    
    size_t        child_count;
    mctree_index_t prev;
    mctree_index_t next;
    mctree_index_t child;
}
typedef mctree_node_t;

struct mctree_tree {
    mctree_node_t* nodes;
    size_t nodes_length;
    size_t nodes_size;
    
    size_t start_size;
}
typedef mctree_t;

typedef void (*mctree_before_insert_f)(const char* key, size_t key_size, void **value);


mctree_t * mctree_create(size_t start_size);
void mctree_clean(mctree_t* mctree);
mctree_t * mctree_destroy(mctree_t* mctree);

mctree_index_t mctree_insert(mctree_t* mctree, const char* key, size_t key_size, void* value, mctree_before_insert_f b_insert);
mctree_index_t mctree_search(mctree_t* mctree, const char* key, size_t key_size);
mctree_index_t mctree_search_lowercase(mctree_t* mctree, const char* key, size_t key_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined(__mhtml__mtree__) */

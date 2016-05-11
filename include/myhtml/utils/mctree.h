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

#ifndef MyHTML_UTILS_MCTREE_H
#define MyHTML_UTILS_MCTREE_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define mctree_node_get_free_id(__mctree__) __mctree__->nodes_length

#define mctree_node_clean(__mctree__, __idx__) \
    __mctree__->nodes[__idx__].str          = NULL; \
    __mctree__->nodes[__idx__].value        = NULL; \
    __mctree__->nodes[__idx__].child_count  = 0;    \
    __mctree__->nodes[__idx__].prev         = 0;    \
    __mctree__->nodes[__idx__].next         = 0;    \
    __mctree__->nodes[__idx__].child        = 0

#define mctree_node_add(__mctree__)                                     \
    __mctree__->nodes_length++;                                         \
    if(__mctree__->nodes_length == __mctree__->nodes_size) {            \
        __mctree__->nodes_size += 4096;                                 \
        __mctree__->nodes = (mctree_node_t*)myrealloc(__mctree__->nodes,  \
            sizeof(mctree_node_t) * __mctree__->nodes_size);            \
    }                                                                   \
    mctree_node_clean(__mctree__, __mctree__->nodes_length)

#define mctree_make_first_idx(__mctree__, __key__, __size__) \
    ((myhtml_string_chars_lowercase_map[ (const unsigned char)(__key__[0]) ] + myhtml_string_chars_lowercase_map[ (const unsigned char)(__key__[__size__ - 1]) ]) % __mctree__->start_size) + 1


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

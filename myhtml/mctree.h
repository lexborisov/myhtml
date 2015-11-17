//
//  mctree.h
//  myhtml
//
//  Created by Alexander Borisov on 25.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#ifndef __mchtml__mctree__
#define __mchtml__mctree__

#include "myosi.h"
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
    (((int)(__key__[0]) + (int)(__key__[__size__ - 1])) % __mctree__->start_size) + 1

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
mctree_t * mctree_destroy(mctree_t* mctree);

mctree_index_t mctree_insert(mctree_t* mctree, const char* key, size_t key_size, void* value, mctree_before_insert_f b_insert);
mctree_index_t mctree_search(mctree_t* mctree, const char* key, size_t key_size);
mctree_index_t mctree_search_lowercase(mctree_t* mctree, const char* key, size_t key_size);


#endif /* defined(__mhtml__mtree__) */

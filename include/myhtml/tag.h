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

#ifndef myhtml_tag_H
#define myhtml_tag_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <myhtml/myosi.h>

#include <myhtml/tag_const.h>
#include <myhtml/tokenizer.h>
#include <myhtml/tree.h>
#include <mycore/utils.h>
#include <mycore/utils/mctree.h>
#include <mycore/utils/mchar_async.h>
#include <mycore/utils/mcobject.h>
#include <mycore/utils/mcobject_async.h>
#include <mycore/utils/mcsimple.h>

#define myhtml_tag_get(tags, idx, attr) tags->context[idx].attr

#define myhtml_tag_context_clean(tags, idx)                       \
    tags->context[idx].id          = 0;                           \
    tags->context[idx].name        = NULL;                        \
    tags->context[idx].name_length = 0;                           \
    tags->context[idx].data_parser = MyHTML_TOKENIZER_STATE_DATA; \
    memset(tags->context[idx].cats, MyHTML_TAG_CATEGORIES_UNDEF, sizeof(tags->context[idx].cats));


#define myhtml_tag_context_add(tags)                                         \
    tags->context_length++;                                                  \
    if(tags->context_length == tags->context_size) {                         \
        tags->context_size += 4096;                                          \
        tags->context = (myhtml_tag_context_t*)mycore_realloc(tags->context,      \
            sizeof(myhtml_tag_context_t) * tags->context_size);              \
    }                                                                        \
    myhtml_tag_context_clean(tags, tags->context_length)

#define myhtml_tag_index_clean_node(index_node)             \
    memset(index_node, 0, sizeof(myhtml_tag_index_node_t));

struct myhtml_tag_context {
    myhtml_tag_id_t id;
    
    const char* name;
    size_t name_length;
    
    enum myhtml_tokenizer_state data_parser;
    enum myhtml_tag_categories cats[MyHTML_NAMESPACE_LAST_ENTRY];
}
typedef myhtml_tag_context_t;

struct myhtml_tag_static_list {
    const myhtml_tag_context_t* ctx;
    size_t next;
    size_t cur;
}
typedef myhtml_tag_static_list_t;

struct myhtml_tag {
    mctree_t* tree;
    mcsimple_t* mcsimple_context;
    
    size_t tags_count;
    size_t mchar_node;
    
    mchar_async_t *mchar;
};

myhtml_tag_t * myhtml_tag_create(void);
mystatus_t myhtml_tag_init(myhtml_tree_t *tree, myhtml_tag_t *tags);
void myhtml_tag_clean(myhtml_tag_t* tags);
myhtml_tag_t * myhtml_tag_destroy(myhtml_tag_t* tags);

myhtml_tag_id_t myhtml_tag_add(myhtml_tag_t* tags, const char* key, size_t key_size,
                              enum myhtml_tokenizer_state data_parser, bool to_lcase);

void myhtml_tag_set_category(myhtml_tag_t* tags, myhtml_tag_id_t tag_idx,
                         enum myhtml_namespace ns, enum myhtml_tag_categories cats);

const myhtml_tag_context_t * myhtml_tag_get_by_id(myhtml_tag_t* tags, myhtml_tag_id_t tag_id);
const myhtml_tag_context_t * myhtml_tag_get_by_name(myhtml_tag_t* tags, const char* name, size_t length);

const myhtml_tag_context_t * myhtml_tag_static_get_by_id(size_t idx);
const myhtml_tag_context_t * myhtml_tag_static_search(const char* name, size_t length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

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

#ifndef myhtml_tag_H
#define myhtml_tag_H

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"

#include "myhtml/utils/mctree.h"
#include "myhtml/tag_const.h"
#include "myhtml/tokenizer.h"
#include "myhtml/tree.h"
#include "myhtml/utils/mcobject_async.h"

#define myhtml_tag_get(__tags__, __idx__, __attr__) __tags__->context[__idx__].__attr__

#define myhtml_tag_context_get_free_id(__tags__) __tags__->context_length

#define myhtml_tag_context_clean(__tags__, __idx__)                        \
    __tags__->context[__idx__].id          = 0;                        \
    __tags__->context[__idx__].mctree_id   = 0;                        \
    __tags__->context[__idx__].data_parser = MyHTML_TOKENIZER_STATE_DATA; \
    memset(__tags__->context[__idx__].cats, MyHTML_TAG_CATEGORIES_UNDEF, sizeof(__tags__->context[__idx__].cats));


#define myhtml_tag_context_add(__tags__)                                         \
    __tags__->context_length++;                                              \
    if(__tags__->context_length == __tags__->context_size) {               \
        __tags__->context_size += 4096;                                      \
        __tags__->context = (myhtml_tag_context_t*)myrealloc(__tags__->context,  \
            sizeof(myhtml_tag_context_t) * __tags__->context_size);              \
    }                                                                          \
    myhtml_tag_context_clean(__tags__, __tags__->context_length)

#define myhtml_tag_index_clean_node(__index_node__) \
    memset(__index_node__, 0, sizeof(myhtml_tag_index_node_t));

struct myhtml_tag_index_entry {
    myhtml_tag_index_node_t *first;
    myhtml_tag_index_node_t *last;
    size_t count;
};

struct myhtml_tag_index_node {
    myhtml_tag_index_node_t *next;
    myhtml_tag_index_node_t *prev;
    
    myhtml_tree_node_t* node;
};

struct myhtml_tag_index {
    myhtml_tag_index_entry_t* tags;
    size_t tags_length;
    size_t tags_size;
    
    size_t index_id;
};

struct myhtml_tag_context {
    size_t id;
    mctree_index_t mctree_id;
    
    enum myhtml_tokenizer_state data_parser;
    enum myhtml_tag_categories cats[MyHTML_NAMESPACE_LAST_ENTRY];
}
typedef myhtml_tag_context_t;

struct myhtml_tag {
    mctree_t* tree;
    
    myhtml_tag_context_t *context;
    size_t context_length;
    size_t context_size;
    
    char* cache_name;
    size_t cache_name_length;
    size_t cache_name_size;
    
    mcobject_async_t* index_nodes;
};

myhtml_tag_t * myhtml_tag_create(void);
myhtml_status_t myhtml_tag_init(myhtml_tag_t *tags);
myhtml_tag_t * myhtml_tag_destroy(myhtml_tag_t* tags);

void myhtml_tag_init_tags(myhtml_tag_t* tags);
void myhtml_tag_init_tags_categories(myhtml_tag_t* tags);
void myhtml_tag_clean(myhtml_tag_t* tags);

myhtml_tag_id_t myhtml_tag_add(myhtml_tag_t* tags, const char* key, size_t key_size,
                              enum myhtml_tokenizer_state data_parser);

void myhtml_tag_set_category(myhtml_tag_t* tags, myhtml_tag_id_t tag_idx,
                         enum myhtml_namespace my_namespace, enum myhtml_tag_categories cats);

myhtml_tag_index_t * myhtml_tag_index_create(myhtml_tag_t* tags);
myhtml_status_t myhtml_tag_index_init(myhtml_tag_t* tags, myhtml_tag_index_t* idx_tags);
void myhtml_tag_index_clean(myhtml_tag_t* tags, myhtml_tag_index_t* index_tags);
myhtml_tag_index_t * myhtml_tag_index_destroy(myhtml_tag_t* tags, myhtml_tag_index_t* idx_tags);

myhtml_status_t myhtml_tag_index_add(myhtml_tag_t* tags, myhtml_tag_index_t* idx_tags, myhtml_tree_node_t* node);
myhtml_tag_index_entry_t * myhtml_tag_index_get(myhtml_tag_index_t* idx_tags, myhtml_tag_id_t tag_idx);
myhtml_tag_index_node_t * myhtml_tag_index_get_first(myhtml_tag_index_t* idx_tags, myhtml_tag_id_t tag_ctx_idx);
myhtml_tag_index_node_t * myhtml_tag_index_get_last(myhtml_tag_index_t* idx_tags, myhtml_tag_id_t tag_ctx_idx);

void myhtml_tag_print(myhtml_tag_t* tags, FILE* fh);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

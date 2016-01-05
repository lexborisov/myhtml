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

#ifndef MyHTML_MYTAGS_H
#define MyHTML_MYTAGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"

#include "myhtml/utils/mctree.h"
#include "myhtml/mytags_const.h"
#include "myhtml/tokenizer.h"
#include "myhtml/tree.h"
#include "myhtml/utils/mcobject_async.h"

#define mytags_get(__mytags__, __idx__, __attr__) __mytags__->context[__idx__].__attr__

#define mytags_context_get_free_id(__mytags__) __mytags__->context_length

#define mytags_context_clean(__mytags__, __idx__)                        \
    __mytags__->context[__idx__].id          = 0;                        \
    __mytags__->context[__idx__].mctree_id   = 0;                        \
    __mytags__->context[__idx__].data_parser = MyHTML_TOKENIZER_STATE_DATA;

#define mytags_context_add(__mytags__)                                         \
    __mytags__->context_length++;                                              \
    if(__mytags__->context_length == __mytags__->context_size) {               \
        __mytags__->context_size += 4096;                                      \
        __mytags__->context = (mytags_context_t*)myrealloc(__mytags__->context,  \
            sizeof(mytags_context_t) * __mytags__->context_size);              \
    }                                                                          \
    mytags_context_clean(__mytags__, __mytags__->context_length)

#define mytags_index_tag_node_attr(__index__, __idx__, __attr__) \
    __index__->nodes[__idx__].__attr__

#define mytags_index_tag_node(__index__, __idx__) \
    __index__->nodes[__idx__]

#define mytags_index_tag_clean(__index_node__) \
    memset(__index_node__, 0, sizeof(mytags_index_tag_node_t));

struct mytags_index_tag {
    mytags_index_tag_node_t *first;
    mytags_index_tag_node_t *last;
    size_t count;
};

struct mytags_index_tag_node {
    mytags_index_tag_node_t *next;
    mytags_index_tag_node_t *prev;
    
    myhtml_tree_node_t* node;
};

struct mytags_index {
    mytags_index_tag_t* tags;
    size_t tags_length;
    size_t tags_size;
    
    size_t index_id;
};

struct mytags_context {
    size_t id;
    mctree_index_t mctree_id;
    
    enum myhtml_tokenizer_state data_parser;
    enum mytags_categories cats[MyHTML_NAMESPACE_LAST_ENTRY];
}
typedef mytags_context_t;

struct mytags {
    mctree_t* tree;
    
    mytags_context_t *context;
    size_t context_length;
    size_t context_size;
    
    char* cache_name;
    size_t cache_name_length;
    size_t cache_name_size;
    
    mcobject_async_t* index_nodes;
};

mytags_t * mytags_create(void);
myhtml_status_t mytags_init(mytags_t *mytags);
mytags_t * mytags_destroy(mytags_t* mytags);

void mytags_init_tags(mytags_t* tags);
void mytags_init_tags_categories(mytags_t* tags);
void mytags_clean(mytags_t* mytags);

mytags_ctx_index_t mytags_add(mytags_t* mytags, const char* key, size_t key_size,
                              enum myhtml_tokenizer_state data_parser);

void mytags_set_category(mytags_t* mytags, mytags_ctx_index_t tag_idx,
                         enum myhtml_namespace my_namespace, enum mytags_categories cats);

mytags_index_t * mytags_index_create(mytags_t* mytags);
myhtml_status_t mytags_index_init(mytags_t* mytags, mytags_index_t* idx_tags);
void mytags_index_clean(mytags_t* mytags, mytags_index_t* index_tags);
mytags_index_t * mytags_index_destroy(mytags_t* mytags, mytags_index_t* idx_tags);

myhtml_status_t mytags_index_tag_add(mytags_t* mytags, mytags_index_t* idx_tags, myhtml_tree_node_t* node);
mytags_index_tag_t * mytags_index_tag_get(mytags_index_t* idx_tags, mytags_ctx_index_t tag_idx);
mytags_index_tag_node_t * mytags_index_tag_get_first(mytags_index_t* idx_tags, mytags_ctx_index_t tag_ctx_idx);
mytags_index_tag_node_t * mytags_index_tag_get_last(mytags_index_t* idx_tags, mytags_ctx_index_t tag_ctx_idx);

void mytags_print(mytags_t* mytags, FILE* fh);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

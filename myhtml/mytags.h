//
//  mytags_init.h
//  myhtml
//
//  Created by Alexander Borisov on 29.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_mytags_init_h
#define myhtml_mytags_init_h

#include "myosi.h"

#include "mctree.h"
#include "mytags_const.h"
#include "myhtml_tokenizer.h"
#include "mcobject.h"

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

#define mytags_index_tag_clean(__index__, __idx__) \
    memset(&mytags_index_tag_node(__index__, __idx__), 0, sizeof(mytags_index_tag_node_t));

struct mytags_index_tag {
    size_t first;
    size_t last;
    size_t count;
};

struct mytags_index_tag_node {
    size_t next;
    size_t prev;
    
    myhtml_token_node_t* token;
};

struct mytags_index {
    mytags_index_tag_t* tags;
    size_t tags_length;
    size_t tags_size;
    
    mytags_index_tag_node_t* nodes;
    mcobject_t* tag_nodes_obj;
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
};

mytags_t * mytags_init(void);
void mytags_init_tags(mytags_t* tags);
void mytags_init_tags_categories(mytags_t* tags);
void mytags_clean(mytags_t* mytags);
mytags_t * mytags_destroy(mytags_t* mytags);

mytags_ctx_index_t mytags_add(mytags_t* mytags, const char* key, size_t key_size,
                              enum myhtml_tokenizer_state data_parser);

void mytags_set_category(mytags_t* mytags, mytags_ctx_index_t tag_idx,
                         enum myhtml_namespace my_namespace, enum mytags_categories cats);

mytags_index_t * mytags_index_create(mytags_t* mytags);
void mytags_index_init(mytags_t* mytags, mytags_index_t* idx_tags);
void mytags_index_tag_add(mytags_t* mytags, mytags_index_t* idx_tags, myhtml_token_node_t* token);
size_t mytags_index_tag_get_first(mytags_index_t* idx_tags, mytags_ctx_index_t tag_ctx_idx);
size_t mytags_index_tag_get_last(mytags_index_t* idx_tags, mytags_ctx_index_t tag_ctx_idx);
size_t mytags_index_tag_get_from_node_id(mytags_index_t* idx_tags, size_t node_id, long long offset);

void mytags_print(mytags_t* mytags, FILE* fh);


#endif
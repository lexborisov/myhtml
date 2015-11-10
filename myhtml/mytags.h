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
#include "myhtml_parse.h"

#define mytags_get(__mytags__, __idx__, __attr__) __mytags__->context[__idx__].__attr__

#define mytags_context_get_free_id(__mytags__) __mytags__->context_length

#define mytags_context_clean(__mytags__, __idx__)                        \
    __mytags__->context[__idx__].id          = 0;                        \
    __mytags__->context[__idx__].mctree_id   = 0;                        \
    __mytags__->context[__idx__].data_parser = MyHTML_PARSE_STATE_DATA;

#define mytags_context_add(__mytags__)                                         \
    __mytags__->context_length++;                                              \
    if(__mytags__->context_length == __mytags__->context_size) {               \
        __mytags__->context_size += 4096;                                      \
        __mytags__->context = (mytags_context_t*)realloc(__mytags__->context,  \
            sizeof(mytags_context_t) * __mytags__->context_size);              \
    }                                                                          \
    mytags_context_clean(__mytags__, __mytags__->context_length)

struct myhtml_tree;

enum myhtml_parse_stateds;

struct mytags_context {
    size_t id;
    mctree_index_t mctree_id;
    
    int data_parser;
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
void mytags_clean(mytags_t* mytags);
mytags_t * mytags_destroy(mytags_t* mytags);

mytags_ctx_index_t mytags_add(mytags_t* mytags, const char* key, size_t key_size, enum myhtml_parse_state data_parser);

void mytags_print(mytags_t* mytags, FILE* fh);


#endif

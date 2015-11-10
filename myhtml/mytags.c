//
//  mytags.c
//  myhtml
//
//  Created by Alexander Borisov on 29.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#include "mytags.h"


mytags_t * mytags_init(void)
{
    mytags_t* mytags = (mytags_t*)malloc(sizeof(mytags_t));
    
    mytags->context_size = 4096;
    mytags->context = (mytags_context_t*)malloc(sizeof(mytags_context_t) * mytags->context_size);
    
    mytags->cache_name_size = mytags->context_size * 12;
    mytags->cache_name = (char*)malloc(sizeof(char) * mytags->cache_name_size);
    
    mytags->tree = mctree_create(32);
    
    mytags_clean(mytags);
    mytags_init_tags(mytags);
    
    return mytags;
}

void mytags_clean(mytags_t* mytags)
{
    mytags->context_length = 0;
    mytags_context_clean(mytags, mytags->context_length);
    mytags_context_add(mytags);
    
    mytags->cache_name_length = 0;
}

mytags_t * mytags_destroy(mytags_t* mytags)
{
    if(mytags == NULL)
        return NULL;
    
    if(mytags->context)
        free(mytags->context);
    
    if(mytags->cache_name)
        free(mytags->cache_name);
    
    mytags->tree = mctree_destroy(mytags->tree);
    
    free(mytags);
    
    return NULL;
}

mytags_ctx_index_t mytags_add(mytags_t* mytags, const char* key, size_t key_size, enum myhtml_parse_state data_parser)
{
    // cache set
    size_t cache_begin = mytags->cache_name_length;
    
    mytags->cache_name_length += key_size + 1;
    
    if(mytags->cache_name_length >= mytags->cache_name_size) {
        mytags->cache_name_size = mytags->cache_name_length + (4096 * 12);
        mytags->cache_name = (char*)realloc(mytags->cache_name, // char is always 1
                                            sizeof(char) * mytags->cache_name_size);
    }
    
    char* cache = &mytags->cache_name[cache_begin];
    
    size_t i;
    for(i = 0; i < key_size; i++) {
        cache[i] = key[i] > 0x40 && key[i] < 0x5b ? (key[i]|0x60) : key[i];
    }
    cache[i] = '\0';
    
    // add tags
    mytags_ctx_index_t new_ctx_id = mytags_context_get_free_id(mytags);
    
    mytags->context[new_ctx_id].id          = new_ctx_id;
    mytags->context[new_ctx_id].mctree_id   = mctree_insert(mytags->tree, cache, key_size, (void *)new_ctx_id, NULL);
    mytags->context[new_ctx_id].data_parser = data_parser;
    
    mytags_context_add(mytags);
    
    return new_ctx_id;
}

void mytags_print(mytags_t* mytags, FILE* fh)
{
    mytags_context_t* ctx = mytags->context;
    mctree_node_t* mct_nodes = mytags->tree->nodes;
    
    size_t i;
    for(i = MyTAGS_TAG_FIRST_ENTRY; i <= MyTAGS_TAG_LAST_ENTRY; i++)
    {
        fprintf(fh, "<%s id=\"%lu\">\n", mct_nodes[ ctx[i].mctree_id ].str, i);
    }
}




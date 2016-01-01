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
    mytags_t* mytags = (mytags_t*)mymalloc(sizeof(mytags_t));
    
    mytags->context_size = 4096 * 50;
    mytags->context = (mytags_context_t*)mymalloc(sizeof(mytags_context_t) * mytags->context_size);
    
    mytags->cache_name_size = mytags->context_size * 12;
    mytags->cache_name = (char*)mymalloc(sizeof(char) * mytags->cache_name_size);
    
    mytags->tree = mctree_create(32);
    
    mytags_clean(mytags);
    mytags_init_tags(mytags);
    mytags_init_tags_categories(mytags);
    
    mytags->index_nodes = mcobject_async_create(128, 4096, sizeof(mytags_index_tag_node_t));
    
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
    mytags->index_nodes = mcobject_async_destroy(mytags->index_nodes, mytrue);
    
    free(mytags);
    
    return NULL;
}

mytags_index_t * mytags_index_create(mytags_t* mytags)
{
    mytags_index_t* idx_tags = (mytags_index_t*)calloc(1, sizeof(mytags_index_t));
    return idx_tags;
}

void mytags_index_init(mytags_t* mytags, mytags_index_t* idx_tags)
{
    idx_tags->index_id = mcobject_async_node_add(mytags->index_nodes);
    
    idx_tags->tags_size = mytags->context_size;
    idx_tags->tags_length = 0;
    idx_tags->tags = (mytags_index_tag_t*)calloc(idx_tags->tags_size, sizeof(mytags_index_tag_t));
}

void mytags_index_clean(mytags_t* mytags, mytags_index_t* index_tags)
{
    mcobject_async_node_clean(mytags->index_nodes, index_tags->index_id);
    memset(index_tags->tags, 0, sizeof(mytags_index_tag_t) * mytags->context_length);
}

mytags_index_t * mytags_index_destroy(mytags_t* mytags, mytags_index_t* index_tags)
{
    if(index_tags == NULL)
        return NULL;
    
    mcobject_async_node_delete(mytags->index_nodes, index_tags->index_id);
    
    if(index_tags->tags) {
        free(index_tags->tags);
        index_tags->tags = NULL;
    }
    
    free(index_tags);
    
    return NULL;
}

void mytags_index_tag_check_size(mytags_t* mytags, mytags_index_t* index_tags)
{
    if(index_tags->tags_size < mytags->context_size) {
        index_tags->tags = (mytags_index_tag_t*)myrealloc(index_tags->tags,
                                                          sizeof(mytags_index_tag_t) *
                                                          mytags->context_size);
        
        memset(&index_tags->tags[index_tags->tags_size], 0, sizeof(mytags_index_tag_t)
               * (mytags->context_size - index_tags->tags_size));
        
        index_tags->tags_size = mytags->context_size;
    }
}

void mytags_index_tag_add(mytags_t* mytags, mytags_index_t* idx_tags, myhtml_tree_node_t* node)
{
    mytags_index_tag_check_size(mytags, idx_tags);
    
    mytags_index_tag_t* tag = &idx_tags->tags[node->tag_idx];
    
    mytags_index_tag_node_t* new_node = mcobject_async_malloc(mytags->index_nodes, idx_tags->index_id);
    mytags_index_tag_clean(new_node);
    
    if(tag->first == NULL) {
        tag->first = new_node;
        
        new_node->prev = NULL;
    }
    else {
        tag->last->next = new_node;
        new_node->prev = tag->last;
    }
    
    new_node->next = NULL;
    new_node->node = node;
    
    tag->last = new_node;
    
    tag->count++;
}

mytags_index_tag_t * mytags_index_tag_get(mytags_index_t* idx_tags, mytags_ctx_index_t tag_idx)
{
    return &idx_tags->tags[tag_idx];
}

mytags_index_tag_node_t * mytags_index_tag_get_first(mytags_index_t* idx_tags, mytags_ctx_index_t tag_ctx_idx)
{
    return idx_tags->tags[tag_ctx_idx].first;
}

mytags_index_tag_node_t * mytags_index_tag_get_last(mytags_index_t* idx_tags, mytags_ctx_index_t tag_ctx_idx)
{
    return idx_tags->tags[tag_ctx_idx].last;
}

mytags_ctx_index_t mytags_add(mytags_t* mytags, const char* key, size_t key_size,
                              enum myhtml_tokenizer_state data_parser)
{
    // cache set
    size_t cache_begin = mytags->cache_name_length;
    
    mytags->cache_name_length += key_size + 1;
    
    if(mytags->cache_name_length >= mytags->cache_name_size) {
        mytags->cache_name_size = mytags->cache_name_length + (4096 * 12);
        mytags->cache_name = (char*)myrealloc(mytags->cache_name, // char is always 1
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
    
    memset(mytags->context[new_ctx_id].cats, 0,
           sizeof(enum mytags_categories) *
           MyHTML_NAMESPACE_LAST_ENTRY);
    
    mytags_context_add(mytags);
    
    return new_ctx_id;
}

void mytags_set_category(mytags_t* mytags, mytags_ctx_index_t tag_idx,
                                       enum myhtml_namespace my_namespace, enum mytags_categories cats)
{
    mytags->context[tag_idx].cats[my_namespace] = cats;
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




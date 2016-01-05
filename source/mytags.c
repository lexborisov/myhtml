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

#include "mytags.h"


mytags_t * mytags_create(void)
{
    return (mytags_t*)mymalloc(sizeof(mytags_t));
}

myhtml_status_t mytags_init(mytags_t *mytags)
{
    mytags->context_size = 4096 * 50;
    mytags->context = (mytags_context_t*)mymalloc(sizeof(mytags_context_t) * mytags->context_size);
    
    if(mytags->context == NULL) {
        mytags->cache_name = NULL;
        mytags->index_nodes = NULL;
        
        return MyHTML_STATUS_MYTAGS_ERROR_MEMORY_ALLOCATION;
    }
    
    mytags->cache_name_size = mytags->context_size * 12;
    mytags->cache_name = (char*)mymalloc(sizeof(char) * mytags->cache_name_size);
    
    if(mytags->cache_name == NULL) {
        mytags->index_nodes = NULL;
        
        return MyHTML_STATUS_MYTAGS_ERROR_CACHE_MEMORY_ALLOCATION;
    }
    
    mytags->tree = mctree_create(32);
    
    mytags_clean(mytags);
    mytags_init_tags(mytags);
    mytags_init_tags_categories(mytags);
    
    mytags->index_nodes = mcobject_async_create();
    mcobject_async_status_t mcstatus = mcobject_async_init(mytags->index_nodes, 128, 4096, sizeof(mytags_index_tag_node_t));
    
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK)
        return MyHTML_STATUS_MYTAGS_ERROR_MCOBJECT_CREATE;
    
    return MyHTML_STATUS_OK;
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
    return (mytags_index_t*)calloc(1, sizeof(mytags_index_t));
}

myhtml_status_t mytags_index_init(mytags_t* mytags, mytags_index_t* idx_tags)
{
    mcobject_async_status_t mcstatus;
    idx_tags->index_id = mcobject_async_node_add(mytags->index_nodes, &mcstatus);
    
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK) {
        idx_tags->tags = NULL;
        
        return MyHTML_STATUS_MYTAGS_ERROR_MCOBJECT_CREATE_NODE;
    }
    
    idx_tags->tags_size = mytags->context_size;
    idx_tags->tags_length = 0;
    idx_tags->tags = (mytags_index_tag_t*)calloc(idx_tags->tags_size, sizeof(mytags_index_tag_t));
    
    if(idx_tags->tags == NULL)
        return MyHTML_STATUS_MYTAGS_ERROR_INDEX_MEMORY_ALLOCATION;
    
    return MyHTML_STATUS_OK;
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

myhtml_status_t mytags_index_tag_add(mytags_t* mytags, mytags_index_t* idx_tags, myhtml_tree_node_t* node)
{
    mytags_index_tag_check_size(mytags, idx_tags);
    
    mytags_index_tag_t* tag = &idx_tags->tags[node->tag_idx];
    
    mcobject_async_status_t mcstatus;
    mytags_index_tag_node_t* new_node = mcobject_async_malloc(mytags->index_nodes, idx_tags->index_id, &mcstatus);
    
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK)
        return MyHTML_STATUS_MYTAGS_ERROR_MCOBJECT_MALLOC;
    
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
    
    return MyHTML_STATUS_OK;
}

mytags_index_tag_t * mytags_index_tag_get(mytags_index_t* idx_tags, mytags_ctx_index_t tag_idx)
{
    if(idx_tags->tags_size > tag_idx)
        return &idx_tags->tags[tag_idx];
    
    return NULL;
}

mytags_index_tag_node_t * mytags_index_tag_get_first(mytags_index_t* idx_tags, mytags_ctx_index_t tag_idx)
{
    if(idx_tags->tags_size > tag_idx)
        return idx_tags->tags[tag_idx].first;
    
    return NULL;
}

mytags_index_tag_node_t * mytags_index_tag_get_last(mytags_index_t* idx_tags, mytags_ctx_index_t tag_idx)
{
    if(idx_tags->tags_size > tag_idx)
        return idx_tags->tags[tag_idx].last;
    
    return NULL;
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




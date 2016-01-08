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

#include "tag.h"


myhtml_tag_t * myhtml_tag_create(void)
{
    return (myhtml_tag_t*)mymalloc(sizeof(myhtml_tag_t));
}

myhtml_status_t myhtml_tag_init(myhtml_tag_t *tags)
{
    tags->context_size = 4096 * 50;
    tags->context = (myhtml_tag_context_t*)mymalloc(sizeof(myhtml_tag_context_t) * tags->context_size);
    
    if(tags->context == NULL) {
        tags->cache_name = NULL;
        tags->index_nodes = NULL;
        
        return MyHTML_STATUS_TAGS_ERROR_MEMORY_ALLOCATION;
    }
    
    tags->cache_name_size = tags->context_size * 12;
    tags->cache_name = (char*)mymalloc(sizeof(char) * tags->cache_name_size);
    
    if(tags->cache_name == NULL) {
        tags->index_nodes = NULL;
        
        return MyHTML_STATUS_TAGS_ERROR_CACHE_MEMORY_ALLOCATION;
    }
    
    tags->tree = mctree_create(32);
    
    myhtml_tag_clean(tags);
    myhtml_tag_init_tags(tags);
    myhtml_tag_init_tags_categories(tags);
    
    tags->index_nodes = mcobject_async_create();
    mcobject_async_status_t mcstatus = mcobject_async_init(tags->index_nodes, 128, 4096, sizeof(myhtml_tag_index_node_t));
    
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK)
        return MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE;
    
    return MyHTML_STATUS_OK;
}

void myhtml_tag_clean(myhtml_tag_t* tags)
{
    tags->context_length = 0;
    myhtml_tag_context_clean(tags, tags->context_length);
    myhtml_tag_context_add(tags);
    
    tags->cache_name_length = 0;
}

myhtml_tag_t * myhtml_tag_destroy(myhtml_tag_t* tags)
{
    if(tags == NULL)
        return NULL;
    
    if(tags->context)
        free(tags->context);
    
    if(tags->cache_name)
        free(tags->cache_name);
    
    tags->tree = mctree_destroy(tags->tree);
    tags->index_nodes = mcobject_async_destroy(tags->index_nodes, mytrue);
    
    free(tags);
    
    return NULL;
}

myhtml_tag_index_t * myhtml_tag_index_create(void)
{
    return (myhtml_tag_index_t*)calloc(1, sizeof(myhtml_tag_index_t));
}

myhtml_status_t myhtml_tag_index_init(myhtml_tag_t* tags, myhtml_tag_index_t* idx_tags)
{
    mcobject_async_status_t mcstatus;
    idx_tags->index_id = mcobject_async_node_add(tags->index_nodes, &mcstatus);
    
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK) {
        idx_tags->tags = NULL;
        
        return MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE_NODE;
    }
    
    idx_tags->tags_size = tags->context_size;
    idx_tags->tags_length = 0;
    idx_tags->tags = (myhtml_tag_index_entry_t*)calloc(idx_tags->tags_size, sizeof(myhtml_tag_index_entry_t));
    
    if(idx_tags->tags == NULL)
        return MyHTML_STATUS_TAGS_ERROR_INDEX_MEMORY_ALLOCATION;
    
    return MyHTML_STATUS_OK;
}

void myhtml_tag_index_clean(myhtml_tag_t* tags, myhtml_tag_index_t* index_tags)
{
    mcobject_async_node_clean(tags->index_nodes, index_tags->index_id);
    memset(index_tags->tags, 0, sizeof(myhtml_tag_index_entry_t) * tags->context_length);
}

myhtml_tag_index_t * myhtml_tag_index_destroy(myhtml_tag_t* tags, myhtml_tag_index_t* index_tags)
{
    if(index_tags == NULL)
        return NULL;
    
    mcobject_async_node_delete(tags->index_nodes, index_tags->index_id);
    
    if(index_tags->tags) {
        free(index_tags->tags);
        index_tags->tags = NULL;
    }
    
    free(index_tags);
    
    return NULL;
}

void myhtml_tag_index_check_size(myhtml_tag_t* tags, myhtml_tag_index_t* index_tags)
{
    if(index_tags->tags_size < tags->context_size) {
        index_tags->tags = (myhtml_tag_index_entry_t*)myrealloc(index_tags->tags,
                                                          sizeof(myhtml_tag_index_entry_t) *
                                                          tags->context_size);
        
        memset(&index_tags->tags[index_tags->tags_size], 0, sizeof(myhtml_tag_index_entry_t)
               * (tags->context_size - index_tags->tags_size));
        
        index_tags->tags_size = tags->context_size;
    }
}

myhtml_status_t myhtml_tag_index_add(myhtml_tag_t* tags, myhtml_tag_index_t* idx_tags, myhtml_tree_node_t* node)
{
    myhtml_tag_index_check_size(tags, idx_tags);
    
    myhtml_tag_index_entry_t* tag = &idx_tags->tags[node->tag_idx];
    
    mcobject_async_status_t mcstatus;
    myhtml_tag_index_node_t* new_node = mcobject_async_malloc(tags->index_nodes, idx_tags->index_id, &mcstatus);
    
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK)
        return MyHTML_STATUS_TAGS_ERROR_MCOBJECT_MALLOC;
    
    myhtml_tag_index_clean_node(new_node);
    
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

myhtml_tag_index_entry_t * myhtml_tag_index_entry(myhtml_tag_index_t* tag_index, myhtml_tag_id_t tag_id)
{
    if(tag_index->tags_size > tag_id)
        return &tag_index->tags[tag_id];
    
    return NULL;
}

size_t myhtml_tag_index_entry_count(myhtml_tag_index_t* tag_index, myhtml_tag_id_t tag_id)
{
    if(tag_index->tags_size > tag_id)
        return tag_index->tags[tag_id].count;
    
    return 0;
}

myhtml_tag_index_node_t * myhtml_tag_index_first(myhtml_tag_index_t* tag_index, myhtml_tag_id_t tag_id)
{
    if(tag_index->tags_size > tag_id)
        return tag_index->tags[tag_id].first;
    
    return NULL;
}

myhtml_tag_index_node_t * myhtml_tag_index_last(myhtml_tag_index_t* tag_index, myhtml_tag_id_t tag_id)
{
    if(tag_index->tags_size > tag_id)
        return tag_index->tags[tag_id].last;
    
    return NULL;
}

myhtml_tag_index_node_t * myhtml_tag_index_next(myhtml_tag_index_node_t *index_node)
{
    if(index_node)
        return index_node->next;
    
    return NULL;
}

myhtml_tag_index_node_t * myhtml_tag_index_prev(myhtml_tag_index_node_t *index_node)
{
    if(index_node)
        return index_node->prev;
    
    return NULL;
}


myhtml_tree_node_t * myhtml_tag_index_tree_node(myhtml_tag_index_node_t *index_node)
{
    if(index_node)
        return index_node->node;
    
    return NULL;
}

myhtml_tag_id_t myhtml_tag_add(myhtml_tag_t* tags, const char* key, size_t key_size,
                              enum myhtml_tokenizer_state data_parser)
{
    // cache set
    size_t cache_begin = tags->cache_name_length;
    
    tags->cache_name_length += key_size + 1;
    
    if(tags->cache_name_length >= tags->cache_name_size) {
        tags->cache_name_size = tags->cache_name_length + (4096 * 12);
        tags->cache_name = (char*)myrealloc(tags->cache_name, // char is always 1
                                              sizeof(char) * tags->cache_name_size);
    }
    
    char* cache = &tags->cache_name[cache_begin];
    
    size_t i;
    for(i = 0; i < key_size; i++) {
        cache[i] = key[i] > 0x40 && key[i] < 0x5b ? (key[i]|0x60) : key[i];
    }
    cache[i] = '\0';
    
    // add tags
    myhtml_tag_id_t new_ctx_id = myhtml_tag_context_get_free_id(tags);
    
    tags->context[new_ctx_id].id          = new_ctx_id;
    tags->context[new_ctx_id].mctree_id   = mctree_insert(tags->tree, cache, key_size, (void *)new_ctx_id, NULL);
    tags->context[new_ctx_id].data_parser = data_parser;
    
    memset(tags->context[new_ctx_id].cats, 0,
           sizeof(enum myhtml_tag_categories) *
           MyHTML_NAMESPACE_LAST_ENTRY);
    
    myhtml_tag_context_add(tags);
    
    return new_ctx_id;
}

void myhtml_tag_set_category(myhtml_tag_t* tags, myhtml_tag_id_t tag_idx,
                                       enum myhtml_namespace my_namespace, enum myhtml_tag_categories cats)
{
    tags->context[tag_idx].cats[my_namespace] = cats;
}

void myhtml_tag_print(myhtml_tag_t* tags, FILE* fh)
{
    myhtml_tag_context_t* ctx = tags->context;
    mctree_node_t* mct_nodes = tags->tree->nodes;
    
    size_t i;
    for(i = MyHTML_TAG_FIRST_ENTRY; i <= MyHTML_TAG_LAST_ENTRY; i++)
    {
        fprintf(fh, "<%s id=\"%zu\">\n", mct_nodes[ ctx[i].mctree_id ].str, i);
    }
}




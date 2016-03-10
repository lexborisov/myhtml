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

#include "myhtml/tag.h"


myhtml_tag_t * myhtml_tag_create(void)
{
    return (myhtml_tag_t*)mymalloc(sizeof(myhtml_tag_t));
}

myhtml_status_t myhtml_tag_init(myhtml_tree_t *tree, myhtml_tag_t *tags)
{
    tags->context_size = 1024;
    tags->context = (myhtml_tag_context_t*)mymalloc(sizeof(myhtml_tag_context_t) * tags->context_size);
    
    if(tags->context == NULL)
        return MyHTML_STATUS_TAGS_ERROR_MEMORY_ALLOCATION;
    
    tags->mchar_node = mchar_async_node_add(tree->myhtml->mchar);
    tags->tree       = mctree_create(32);
    
    tags->mchar      = tree->myhtml->mchar;
    tags->tag_index  = tree->myhtml->tag_index;
    
    myhtml_tag_clean(tags);
    
    return MyHTML_STATUS_OK;
}

void myhtml_tag_clean(myhtml_tag_t* tags)
{
    tags->context_length = 0;
    myhtml_tag_context_clean(tags, tags->context_length);
    myhtml_tag_context_add(tags);
    
    mchar_async_node_clean(tags->mchar, tags->mchar_node);
}

myhtml_tag_t * myhtml_tag_destroy(myhtml_tag_t* tags)
{
    if(tags == NULL)
        return NULL;
    
    if(tags->context)
        free(tags->context);
    
    tags->tree = mctree_destroy(tags->tree);
    
    mchar_async_destroy(tags->mchar, true);
    
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
    tags->mcobject_node = mcobject_async_node_add(tags->tag_index, &mcstatus);
    
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK)
        return MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE;
    
    idx_tags->tags_size = tags->context_size;
    idx_tags->tags_length = 0;
    idx_tags->tags = (myhtml_tag_index_entry_t*)calloc(idx_tags->tags_size, sizeof(myhtml_tag_index_entry_t));
    
    if(idx_tags->tags == NULL)
        return MyHTML_STATUS_TAGS_ERROR_INDEX_MEMORY_ALLOCATION;
    
    return MyHTML_STATUS_OK;
}

void myhtml_tag_index_clean(myhtml_tag_t* tags, myhtml_tag_index_t* index_tags)
{
    mcobject_async_node_clean(tags->tag_index, tags->mcobject_node);
    memset(index_tags->tags, 0, sizeof(myhtml_tag_index_entry_t) * tags->context_length);
}

myhtml_tag_index_t * myhtml_tag_index_destroy(myhtml_tag_t* tags, myhtml_tag_index_t* index_tags)
{
    mcobject_async_node_delete(tags->tag_index, tags->mcobject_node);
    
    if(index_tags == NULL)
        return NULL;
    
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
    myhtml_tag_index_node_t* new_node = mcobject_async_malloc(tags->tag_index, tags->mcobject_node, &mcstatus);
    
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
                              enum myhtml_tokenizer_state data_parser, bool to_lcase)
{
    char* cache = mchar_async_malloc(tags->mchar, tags->mchar_node, (key_size + 1));
    
    if(to_lcase) {
        size_t i;
        for(i = 0; i < key_size; i++) {
            cache[i] = key[i] > 0x40 && key[i] < 0x5b ? (key[i]|0x60) : key[i];
        }
        cache[i] = '\0';
    }
    else {
        strncpy(cache, key, key_size);
        cache[key_size] = '\0';
    }
    
    // add tags
    myhtml_tag_id_t new_ctx_id = myhtml_tag_context_get_free_id(tags);
    
    mctree_insert(tags->tree, cache, key_size, (void *)new_ctx_id, NULL);
    
    tags->context[new_ctx_id].id          = new_ctx_id + MyHTML_TAG_LAST_ENTRY;
    tags->context[new_ctx_id].name        = cache;
    tags->context[new_ctx_id].name_length = key_size;
    tags->context[new_ctx_id].data_parser = data_parser;
    
    memset(tags->context[new_ctx_id].cats, 0,
           sizeof(enum myhtml_tag_categories) *
           MyHTML_NAMESPACE_LAST_ENTRY);
    
    myhtml_tag_context_add(tags);
    
    return tags->context[new_ctx_id].id;
}

void myhtml_tag_set_category(myhtml_tag_t* tags, myhtml_tag_id_t tag_idx,
                                       enum myhtml_namespace my_namespace, enum myhtml_tag_categories cats)
{
    tags->context[(tag_idx - MyHTML_TAG_LAST_ENTRY)].cats[my_namespace] = cats;
}

const myhtml_tag_context_t * myhtml_tag_get_by_id(myhtml_tag_t* tags, myhtml_tag_id_t tag_id)
{
    if(tag_id > MyHTML_TAG_LAST_ENTRY) {
        tag_id -= MyHTML_TAG_LAST_ENTRY;
        return &tags->context[tag_id];
    }
    
    return myhtml_tag_static_get_by_id(tag_id);
}

const myhtml_tag_context_t * myhtml_tag_get_by_name(myhtml_tag_t* tags, const char* name, size_t length)
{
    const myhtml_tag_context_t *ctx = myhtml_tag_static_search(name, length);
    
    if(ctx)
        return ctx;
    
    mctree_index_t idx = mctree_search_lowercase(tags->tree, name, length);
    size_t ctx_idx = (size_t)tags->tree->nodes[idx].value;
    
    if(ctx_idx && ctx_idx < tags->context_length)
        return &tags->context[ctx_idx];
    
    return NULL;
}

void myhtml_tag_print(myhtml_tag_t* tags, FILE* fh)
{
    size_t i;
    for(i = MyHTML_TAG_FIRST_ENTRY; i < MyHTML_TAG_LAST_ENTRY; i++)
    {
        const myhtml_tag_context_t *ctx = myhtml_tag_get_by_id(tags, i);
        
        fprintf(fh, "<%s id=\"%zu\">\n", ctx->name, i);
    }
    
    for(i = (MyHTML_TAG_LAST_ENTRY + 1); i < tags->context_length; i++)
    {
        const myhtml_tag_context_t *ctx = myhtml_tag_get_by_id(tags, i);
        
        fprintf(fh, "<%s id=\"%zu\">\n", ctx->name, i);
    }
}




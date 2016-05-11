/*
 Copyright (C) 2015-2016 Alexander Borisov
 
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

#include "myhtml/tag.h"


myhtml_tag_t * myhtml_tag_create(void)
{
    return (myhtml_tag_t*)mymalloc(sizeof(myhtml_tag_t));
}

myhtml_status_t myhtml_tag_init(myhtml_tree_t *tree, myhtml_tag_t *tags)
{
    tags->mcsimple_context = mcsimple_create();
    
    if(tags->mcsimple_context == NULL)
        return MyHTML_STATUS_TAGS_ERROR_MEMORY_ALLOCATION;
    
    mcsimple_init(tags->mcsimple_context, 128, 1024, sizeof(myhtml_tag_context_t));
    
    tags->mchar_node = mchar_async_node_add(tree->myhtml->mchar);
    tags->tree       = mctree_create(32);
    
    tags->mchar      = tree->myhtml->mchar;
    tags->tag_index  = tree->myhtml->tag_index;
    
    tags->tags_count = MyHTML_TAG_LAST_ENTRY;
    
    myhtml_tag_clean(tags);
    
    return MyHTML_STATUS_OK;
}

void myhtml_tag_clean(myhtml_tag_t* tags)
{
    tags->tags_count = MyHTML_TAG_LAST_ENTRY;
    
    mcsimple_clean(tags->mcsimple_context);
    mchar_async_node_clean(tags->mchar, tags->mchar_node);
    mctree_clean(tags->tree);
}

myhtml_tag_t * myhtml_tag_destroy(myhtml_tag_t* tags)
{
    if(tags == NULL)
        return NULL;
    
    tags->tree = mctree_destroy(tags->tree);
    tags->mcsimple_context = mcsimple_destroy(tags->mcsimple_context, true);
    
    mchar_async_node_delete(tags->mchar, tags->mchar_node);
    
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
    
    idx_tags->tags_size = tags->tags_count + 128;
    idx_tags->tags_length = 0;
    idx_tags->tags = (myhtml_tag_index_entry_t*)calloc(idx_tags->tags_size, sizeof(myhtml_tag_index_entry_t));
    
    if(idx_tags->tags == NULL)
        return MyHTML_STATUS_TAGS_ERROR_INDEX_MEMORY_ALLOCATION;
    
    return MyHTML_STATUS_OK;
}

void myhtml_tag_index_clean(myhtml_tag_t* tags, myhtml_tag_index_t* index_tags)
{
    mcobject_async_node_clean(tags->tag_index, tags->mcobject_node);
    memset(index_tags->tags, 0, sizeof(myhtml_tag_index_entry_t) * index_tags->tags_size);
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

void myhtml_tag_index_check_size(myhtml_tag_t* tags, myhtml_tag_index_t* index_tags, myhtml_tag_id_t tag_id)
{
    if(tag_id >= index_tags->tags_size) {
        size_t new_size = tag_id + 128;
        
        myhtml_tag_index_entry_t *index_entries = (myhtml_tag_index_entry_t*)myrealloc(index_tags->tags,
                                                                             sizeof(myhtml_tag_index_entry_t) *
                                                                             new_size);
        
        if(index_entries) {
            index_tags->tags = index_entries;
            
            memset(&index_tags->tags[index_tags->tags_size], 0, sizeof(myhtml_tag_index_entry_t)
                   * (new_size - index_tags->tags_size));
            
            index_tags->tags_size = new_size;
        }
        else {
            // TODO: error
        }
    }
}

myhtml_status_t myhtml_tag_index_add(myhtml_tag_t* tags, myhtml_tag_index_t* idx_tags, myhtml_tree_node_t* node)
{
    myhtml_tag_index_check_size(tags, idx_tags, node->tag_idx);
    
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
    
    myhtml_tag_context_t *tag_ctx = mcsimple_malloc(tags->mcsimple_context);
    
    mctree_insert(tags->tree, cache, key_size, (void *)tag_ctx, NULL);
    
    tag_ctx->id          = tags->tags_count;
    tag_ctx->name        = cache;
    tag_ctx->name_length = key_size;
    tag_ctx->data_parser = data_parser;
    
    tags->tags_count++;
    
    memset(tag_ctx->cats, 0, sizeof(enum myhtml_tag_categories) * MyHTML_NAMESPACE_LAST_ENTRY);
    
    return tag_ctx->id;
}

void myhtml_tag_set_category(myhtml_tag_t* tags, myhtml_tag_id_t tag_idx,
                                       enum myhtml_namespace my_namespace, enum myhtml_tag_categories cats)
{
    if(tag_idx < MyHTML_TAG_LAST_ENTRY)
        return;
    
    myhtml_tag_context_t *tag_ctx = mcsimple_get_by_absolute_position(tags->mcsimple_context, (tag_idx - MyHTML_TAG_LAST_ENTRY));
    tag_ctx->cats[my_namespace] = cats;
}

const myhtml_tag_context_t * myhtml_tag_get_by_id(myhtml_tag_t* tags, myhtml_tag_id_t tag_id)
{
    if(tag_id >= MyHTML_TAG_LAST_ENTRY) {
        return mcsimple_get_by_absolute_position(tags->mcsimple_context, (tag_id - MyHTML_TAG_LAST_ENTRY));
    }
    
    return myhtml_tag_static_get_by_id(tag_id);
}

const myhtml_tag_context_t * myhtml_tag_get_by_name(myhtml_tag_t* tags, const char* name, size_t length)
{
    const myhtml_tag_context_t *ctx = myhtml_tag_static_search(name, length);
    
    if(ctx)
        return ctx;
    
    mctree_index_t idx = mctree_search_lowercase(tags->tree, name, length);
    
    return (myhtml_tag_context_t*)tags->tree->nodes[idx].value;
}

void myhtml_tag_print(myhtml_tag_t* tags, FILE* fh)
{
    size_t i;
    for(i = MyHTML_TAG_FIRST_ENTRY; i < MyHTML_TAG_LAST_ENTRY; i++)
    {
        const myhtml_tag_context_t *ctx = myhtml_tag_get_by_id(tags, i);
        
        fprintf(fh, "<%s id=\"%zu\">\n", ctx->name, i);
    }
    
    for(i = (MyHTML_TAG_LAST_ENTRY + 1); i < tags->tags_count; i++)
    {
        const myhtml_tag_context_t *ctx = myhtml_tag_get_by_id(tags, i);
        
        fprintf(fh, "<%s id=\"%zu\">\n", ctx->name, i);
    }
}




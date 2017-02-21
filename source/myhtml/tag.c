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
    return (myhtml_tag_t*)myhtml_malloc(sizeof(myhtml_tag_t));
}

myhtml_status_t myhtml_tag_init(myhtml_tree_t *tree, myhtml_tag_t *tags)
{
    tags->mcsimple_context = mcsimple_create();
    
    if(tags->mcsimple_context == NULL)
        return MyHTML_STATUS_TAGS_ERROR_MEMORY_ALLOCATION;
    
    mcsimple_init(tags->mcsimple_context, 128, 1024, sizeof(myhtml_tag_context_t));
    
    tags->mchar_node         = mchar_async_node_add(tree->mchar);
    tags->tree               = mctree_create(2);
    tags->mchar              = tree->mchar;
    tags->tags_count         = MyHTML_TAG_LAST_ENTRY;
    
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
    
    myhtml_free(tags);
    
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
                                       enum myhtml_namespace ns, enum myhtml_tag_categories cats)
{
    if(tag_idx < MyHTML_TAG_LAST_ENTRY)
        return;
    
    myhtml_tag_context_t *tag_ctx = mcsimple_get_by_absolute_position(tags->mcsimple_context, (tag_idx - MyHTML_TAG_LAST_ENTRY));
    tag_ctx->cats[ns] = cats;
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
        
        fprintf(fh, "<%s id=\"" MyHTML_FMT_Z "\">\n", ctx->name, i);
    }
    
    for(i = (MyHTML_TAG_LAST_ENTRY + 1); i < tags->tags_count; i++)
    {
        const myhtml_tag_context_t *ctx = myhtml_tag_get_by_id(tags, i);
        
        fprintf(fh, "<%s id=\"" MyHTML_FMT_Z "\">\n", ctx->name, i);
    }
}




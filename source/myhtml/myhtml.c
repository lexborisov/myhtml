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

#include "myhtml/myhtml.h"

void myhtml_init_marker(myhtml_t* myhtml)
{
    myhtml->marker = (myhtml_tree_node_t*)mymalloc(sizeof(myhtml_tree_node_t));
    
    if(myhtml->marker)
        myhtml_tree_node_clean(myhtml->marker);
}

void myhtml_destroy_marker(myhtml_t* myhtml)
{
    if(myhtml->marker)
        free(myhtml->marker);
}

myhtml_t * myhtml_create(void)
{
    return (myhtml_t*)mymalloc(sizeof(myhtml_t));
}

myhtml_status_t myhtml_init(myhtml_t* myhtml, enum myhtml_options opt, size_t thread_count, size_t queue_size)
{
    myhtml_status_t status;
    
    myhtml_init_marker(myhtml);
    
    myhtml->async_incoming_buf = mcobject_async_create();
    if(myhtml->async_incoming_buf == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    mcobject_async_status_t mcstatus = mcobject_async_init(myhtml->async_incoming_buf, 32, 1024, sizeof(myhtml_incoming_buffer_t));
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    // for tags node and other
    myhtml->mchar = mchar_async_create(64, 4096);
    
    myhtml->tag_index = mcobject_async_create();
    mcstatus = mcobject_async_init(myhtml->tag_index, 128, 1024, sizeof(myhtml_tag_index_node_t));
    
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK)
        return MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE;
    
    status = myhtml_tokenizer_state_init(myhtml);
    if(status) {
        myhtml->insertion_func = NULL;
        myhtml->thread = NULL;
        
        return status;
    }
    
    status = myhtml_rules_init(myhtml);
    if(status) {
        myhtml->thread = NULL;
        
        return status;
    }
    
    myhtml->opt = opt;
    myhtml->thread = mythread_create();
    
#ifdef MyHTML_BUILD_WITHOUT_THREADS
    
    status = mythread_init(myhtml->thread, NULL, thread_count, queue_size);
    
    if(status)
        return status;
    
#else /* MyHTML_BUILD_WITHOUT_THREADS */
    switch (opt) {
        case MyHTML_OPTIONS_PARSE_MODE_SINGLE:
            status = mythread_init(myhtml->thread, "lastmac", 0, queue_size);
            if(status)
                return status;
            
            break;
            
        case MyHTML_OPTIONS_PARSE_MODE_ALL_IN_ONE:
            status = mythread_init(myhtml->thread, "lastmac", 1, queue_size);
            if(status)
                return status;
            
            myhread_create_stream(myhtml->thread, myhtml_parser_worker_index_stream, &status);
            break;
            
        case MyHTML_OPTIONS_PARSE_MODE_WORKER_TREE:
            status = mythread_init(myhtml->thread, "lastmac", 2, queue_size);
            if(status)
                return status;
            
            myhread_create_stream(myhtml->thread, myhtml_parser_index, &status);
            myhread_create_stream(myhtml->thread, myhtml_parser_worker_stream, &status);
            break;
            
        case MyHTML_OPTIONS_PARSE_MODE_WORKER_INDEX:
            status = mythread_init(myhtml->thread, "lastmac", 2, queue_size);
            if(status)
                return status;
            
            myhread_create_stream(myhtml->thread, myhtml_parser_worker_index, &status);
            myhread_create_stream(myhtml->thread, myhtml_parser_stream, &status);
            break;
            
        case MyHTML_OPTIONS_PARSE_MODE_TREE_INDEX:
            if(thread_count == 0)
                thread_count = 1;
            
            status = mythread_init(myhtml->thread, "lastmac", (thread_count + 1), queue_size);
            if(status)
                return status;
            
            myhread_create_stream(myhtml->thread, myhtml_parser_stream_index, &status);
            myhread_create_batch(myhtml->thread, myhtml_parser_worker, &status, thread_count);
            break;
            
        default:
            // default MyHTML_OPTIONS_PARSE_MODE_SEPARATELY
            if(thread_count == 0)
                thread_count = 1;
            
            status = mythread_init(myhtml->thread, "lastmac", (thread_count + 2), queue_size);
            if(status)
                return status;
            
            myhread_create_stream(myhtml->thread, myhtml_parser_index, &status);
            myhread_create_stream(myhtml->thread, myhtml_parser_stream, &status);
            myhread_create_batch(myhtml->thread, myhtml_parser_worker, &status, thread_count);
            break;
    }
    
#endif /* MyHTML_BUILD_WITHOUT_THREADS */
    
    myhtml_clean(myhtml);
    
    return status;
}

void myhtml_clean(myhtml_t* myhtml)
{
    mythread_clean(myhtml->thread);
    
    mcobject_async_node_all_clean(myhtml->async_incoming_buf);
}

myhtml_t* myhtml_destroy(myhtml_t* myhtml)
{
    if(myhtml == NULL)
        return NULL;
    
    myhtml_destroy_marker(myhtml);
    
    mythread_destroy(myhtml->thread, true);
    myhtml_tokenizer_state_destroy(myhtml);
    
    myhtml->async_incoming_buf = mcobject_async_destroy(myhtml->async_incoming_buf, true);
    myhtml->tag_index = mcobject_async_destroy(myhtml->tag_index, true);
    myhtml->mchar = mchar_async_destroy(myhtml->mchar, true);
    
    if(myhtml->insertion_func)
        free(myhtml->insertion_func);
    
    free(myhtml);
    
    return NULL;
}

myhtml_status_t myhtml_parse(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    myhtml_encoding_set(tree, encoding);
    
    myhtml_tokenizer_begin(tree, html, html_size);
    myhtml_tokenizer_end(tree);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_parse_fragment(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    if(tag_id == 0)
        tag_id = MyHTML_TAG_DIV;
    
    if(my_namespace == 0)
        my_namespace = MyHTML_NAMESPACE_HTML;
    
    myhtml_tokenizer_fragment_init(tree, tag_id, my_namespace);
    myhtml_encoding_set(tree, encoding);
    
    myhtml_tokenizer_begin(tree, html, html_size);
    myhtml_tokenizer_end(tree);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_parse_single(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    myhtml_encoding_set(tree, encoding);
    
    myhtml_tokenizer_begin(tree, html, html_size);
    myhtml_tokenizer_end(tree);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_parse_fragment_single(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    if(tag_id == 0)
        tag_id = MyHTML_TAG_DIV;
    
    if(my_namespace == 0)
        my_namespace = MyHTML_NAMESPACE_HTML;
    
    tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    myhtml_tokenizer_fragment_init(tree, tag_id, my_namespace);
    myhtml_encoding_set(tree, encoding);
    
    myhtml_tokenizer_begin(tree, html, html_size);
    myhtml_tokenizer_end(tree);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_parse_chunk(myhtml_tree_t* tree, const char* html, size_t html_size)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    myhtml_tokenizer_chunk(tree, html, html_size);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_parse_chunk_fragment(myhtml_tree_t* tree, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    if(tag_id == 0)
        tag_id = MyHTML_TAG_DIV;
    
    if(my_namespace == 0)
        my_namespace = MyHTML_NAMESPACE_HTML;
    
    myhtml_tokenizer_fragment_init(tree, tag_id, my_namespace);
    
    myhtml_tokenizer_chunk(tree, html, html_size);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_parse_chunk_single(myhtml_tree_t* tree, const char* html, size_t html_size)
{
    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
        tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    return myhtml_parse_chunk(tree, html, html_size);
}

myhtml_status_t myhtml_parse_chunk_fragment_single(myhtml_tree_t* tree, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace)
{
    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
        tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    return myhtml_parse_chunk_fragment(tree, html, html_size, tag_id, my_namespace);
}

myhtml_status_t myhtml_parse_chunk_end(myhtml_tree_t* tree)
{
    myhtml_tokenizer_end(tree);
    return MyHTML_STATUS_OK;
}

void myhtml_encoding_set(myhtml_tree_t* tree, myhtml_encoding_t encoding)
{
    if(encoding >= MyHTML_ENCODING_LAST_ENTRY)
        return;
    
    tree->encoding_usereq = encoding;
    tree->encoding        = encoding;
}

myhtml_encoding_t myhtml_encoding_get(myhtml_tree_t* tree)
{
    return tree->encoding;
}

/*
 * Nodes
 */

myhtml_status_t myhtml_get_nodes_by_tag_id_in_scope_find_recursion(myhtml_tree_node_t *node, myhtml_collection_t *collection, myhtml_tag_id_t tag_id)
{
    while(node) {
        if(node->tag_idx == tag_id) {
            collection->list[ collection->length ] = node;
            collection->length++;
            
            if(collection->length >= collection->size)
            {
                myhtml_status_t mystatus = myhtml_collection_check_size(collection, 1024);
                
                if(mystatus != MyHTML_STATUS_OK)
                    return mystatus;
            }
        }
        
        if(node->child)
            myhtml_get_nodes_by_tag_id_in_scope_find_recursion(node->child, collection, tag_id);
        
        node = node->next;
    }
    
    return MyHTML_STATUS_OK;
}

myhtml_collection_t * myhtml_get_nodes_by_tag_id_in_scope(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tree_node_t *node, myhtml_tag_id_t tag_id, myhtml_status_t *status)
{
    if(node == NULL)
        return NULL;
    
    myhtml_status_t mystatus = MyHTML_STATUS_OK;
    
    if(collection == NULL) {
        collection = myhtml_collection_create(1024, &mystatus);
    }
    
    if(mystatus) {
        if(status)
            *status = mystatus;
        
        return collection;
    }
    
    if(node->child)
        mystatus = myhtml_get_nodes_by_tag_id_in_scope_find_recursion(node->child, collection, tag_id);
    
    collection->list[collection->length] = NULL;
    
    if(status)
        *status = mystatus;
    
    return collection;
}

myhtml_collection_t * myhtml_get_nodes_by_name_in_scope(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tree_node_t *node, const char* html, size_t length, myhtml_status_t *status)
{
    const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_name(tree->tags, html, length);
    return myhtml_get_nodes_by_tag_id_in_scope(tree, collection, node, tag_ctx->id, status);
}

myhtml_collection_t * myhtml_get_nodes_by_tag_id(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tag_id_t tag_id, myhtml_status_t *status)
{
    myhtml_tag_index_entry_t *index_tag = myhtml_tag_index_entry(tree->indexes->tags, tag_id);
    myhtml_tag_index_node_t *index_node = myhtml_tag_index_first(tree->indexes->tags, tag_id);
    
    if(index_tag->count == 0) {
        if(status)
            *status = MyHTML_STATUS_OK;
        
        return collection;
    }
    
    myhtml_status_t mystatus = MyHTML_STATUS_OK;
    size_t idx = 0;
    
    if(collection == NULL) {
        collection = myhtml_collection_create((index_tag->count + 128), &mystatus);
        
        collection->length += index_tag->count;
    }
    else {
        idx = collection->length;
        mystatus = myhtml_collection_check_size(collection, index_tag->count);
    }
    
    if(mystatus) {
        if(status)
            *status = mystatus;
        
        return collection;
    }
    
    while (index_node)
    {
        collection->list[idx] = index_node->node;
        idx++;
        
        index_node = index_node->next;
    }
    
    collection->list[idx] = NULL;
    
    if(status)
        *status = mystatus;
    
    return collection;
}

myhtml_collection_t * myhtml_get_nodes_by_name(myhtml_tree_t* tree, myhtml_collection_t *collection, const char* html, size_t length, myhtml_status_t *status)
{
    const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_name(tree->tags, html, length);
    return myhtml_get_nodes_by_tag_id(tree, collection, tag_ctx->id, status);
}

/*
 * Manipulate Nodes
 */
myhtml_tree_node_t * myhtml_node_first(myhtml_tree_t* tree)
{
    if(tree->fragment) {
        // document -> html -> need element
        if(tree->document && tree->document->child)
            return tree->document->child->child;
    }
    else if(tree->document) {
        // document -> html
        return tree->document->child;
    }
    
    return NULL;
}

myhtml_tree_node_t * myhtml_node_next(myhtml_tree_node_t *node)
{
    return node->next;
}

myhtml_tree_node_t * myhtml_node_prev(myhtml_tree_node_t *node)
{
    return node->prev;
}

myhtml_tree_node_t * myhtml_node_parent(myhtml_tree_node_t *node)
{
    return node->parent;
}

myhtml_tree_node_t * myhtml_node_child(myhtml_tree_node_t *node)
{
    return node->child;
}

myhtml_tree_node_t * myhtml_node_last_child(myhtml_tree_node_t *node)
{
    return node->last_child;
}

myhtml_tree_node_t * myhtml_node_create(myhtml_tree_t* tree, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace)
{
    myhtml_tree_node_t *node = myhtml_tree_node_create(tree);
    
    node->tag_idx      = tag_id;
    node->my_namespace = my_namespace;
    
    return node;
}

myhtml_tree_node_t * myhtml_node_remove(myhtml_tree_node_t *node)
{
    return myhtml_tree_node_remove(node);
}

void myhtml_node_delete(myhtml_tree_t* tree, myhtml_tree_node_t *node)
{
    myhtml_tree_node_delete(tree, node);
}

void myhtml_node_delete_recursive(myhtml_tree_t* tree, myhtml_tree_node_t *node)
{
    myhtml_tree_node_delete_recursive(tree, node);
}

void myhtml_node_free(myhtml_tree_t* tree, myhtml_tree_node_t *node)
{
    myhtml_tree_node_free(tree, node);
}

myhtml_tree_node_t * myhtml_node_insert_before(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node)
{
    if(target == NULL || node == NULL)
        return NULL;
    
    myhtml_tree_node_insert_before(tree, target, node);
    
    return node;
}

myhtml_tree_node_t * myhtml_node_insert_after(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node)
{
    if(target == NULL || node == NULL)
        return NULL;
    
    myhtml_tree_node_insert_after(tree, target, node);
    
    return node;
}

myhtml_tree_node_t * myhtml_node_insert_append_child(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node)
{
    if(target == NULL || node == NULL)
        return NULL;
    
    myhtml_tree_node_add_child(tree, target, node);
    
    return node;
}

myhtml_tree_node_t * myhtml_node_insert_to_appropriate_place(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node)
{
    if(target == NULL || node == NULL)
        return NULL;
    
    enum myhtml_tree_insertion_mode mode;
    
    tree->foster_parenting = true;
    target = myhtml_tree_appropriate_place_inserting_in_tree(tree, target, &mode);
    tree->foster_parenting = false;
    
    myhtml_tree_node_insert_by_mode(tree, target, node, mode);
    
    return node;
}

myhtml_string_t * myhtml_node_text_set(myhtml_tree_t* tree, myhtml_tree_node_t *node, const char* text, size_t length, myhtml_encoding_t encoding)
{
    if(node == NULL)
        return NULL;
    
    if(encoding >= MyHTML_ENCODING_LAST_ENTRY)
        return NULL;
    
    if(node->token == NULL) {
        mcobject_async_status_t mcstatus;
        node->token = (myhtml_token_node_t*)mcobject_async_malloc(tree->token->nodes_obj, tree->mcasync_token_id, &mcstatus);
        
        if(mcstatus)
            return NULL;
        
        myhtml_token_node_clean(node->token);
    }
    
    if(node->token->my_str_tm.data == NULL) {
        myhtml_string_init(tree->mchar, tree->mchar_node_id, &node->token->my_str_tm, (length + 2));
    }
    else {
        if(node->token->my_str_tm.size < length) {
            mchar_async_free(tree->mchar, node->token->my_str_tm.node_idx, node->token->my_str_tm.data);
            myhtml_string_init(tree->mchar, tree->mchar_node_id, &node->token->my_str_tm, length);
        }
        else
            node->token->my_str_tm.length = 0;
    }
    
    if(encoding != MyHTML_ENCODING_UTF_8) {
        myhtml_string_append_with_convert_encoding(&node->token->my_str_tm, text, length, encoding);
    }
    else {
        myhtml_string_append(&node->token->my_str_tm, text, length);
    }
    
    node->token->begin  = 0;
    node->token->length = node->token->my_str_tm.length;
    
    return &node->token->my_str_tm;
}

myhtml_string_t * myhtml_node_text_set_with_charef(myhtml_tree_t* tree, myhtml_tree_node_t *node, const char* text, size_t length, myhtml_encoding_t encoding)
{
    if(node == NULL)
        return NULL;
    
    if(encoding >= MyHTML_ENCODING_LAST_ENTRY)
        return NULL;
    
    if(node->token == NULL) {
        mcobject_async_status_t mcstatus;
        node->token = (myhtml_token_node_t*)mcobject_async_malloc(tree->token->nodes_obj, tree->mcasync_token_id, &mcstatus);
        
        if(mcstatus)
            return NULL;
        
        myhtml_token_node_clean(node->token);
    }
    
    if(node->token->my_str_tm.data == NULL) {
        myhtml_string_init(tree->mchar, tree->mchar_node_id, &node->token->my_str_tm, (length + 2));
    }
    else {
        if(node->token->my_str_tm.size < length) {
            mchar_async_free(tree->mchar, node->token->my_str_tm.node_idx, node->token->my_str_tm.data);
            myhtml_string_init(tree->mchar, tree->mchar_node_id, &node->token->my_str_tm, length);
        }
        else
            node->token->my_str_tm.length = 0;
    }
    
    myhtml_string_char_ref_chunk_t str_chunk = {0, 0, 0, {0}, false, encoding};
    myhtml_encoding_result_clean(&str_chunk.res);
    
    myhtml_string_append_charef(&str_chunk, &node->token->my_str_tm, text, length);
    myhtml_string_append_charef_end(&str_chunk, &node->token->my_str_tm);
    
    node->token->begin  = 0;
    node->token->length = node->token->my_str_tm.length;
    
    return &node->token->my_str_tm;
}

enum myhtml_namespace myhtml_node_namespace(myhtml_tree_node_t *node)
{
    return node->my_namespace;
}

myhtml_tag_id_t myhtml_node_tag_id(myhtml_tree_node_t *node)
{
    return node->tag_idx;
}

const char * myhtml_tag_name_by_id(myhtml_tree_t* tree, myhtml_tag_id_t tag_id, size_t *length)
{
    if(length)
        *length = 0;
    
    if(tree == NULL || tree->tags == NULL)
        return NULL;
    
    const myhtml_tag_context_t *ctx = myhtml_tag_get_by_id(tree->tags, tag_id);
    
    if(ctx == NULL)
        return NULL;
    
    if(length)
        *length = ctx->name_length;
    
    return ctx->name;
}

myhtml_tag_id_t myhtml_tag_id_by_name(myhtml_tree_t* tree, const char *tag_name, size_t length)
{
    if(tree == NULL || tree->tags == NULL)
        return MyHTML_TAG__UNDEF;
    
    const myhtml_tag_context_t *ctx = myhtml_tag_get_by_name(tree->tags, tag_name, length);
    
    if(ctx == NULL)
        return MyHTML_TAG__UNDEF;
    
    return ctx->id;
}

bool myhtml_node_is_close_self(myhtml_tree_node_t *node)
{
    if(node->token)
        return (node->token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF);
    
    return false;
}

myhtml_tree_attr_t * myhtml_node_attribute_first(myhtml_tree_node_t *node)
{
    if(node->token)
        return node->token->attr_first;
    
    return NULL;
}

myhtml_tree_attr_t * myhtml_node_attribute_last(myhtml_tree_node_t *node)
{
    if(node->token)
        return node->token->attr_first;
    
    return NULL;
}

const char * myhtml_node_text(myhtml_tree_node_t *node, size_t *length)
{
    if(node->token && node->token->length && node->token->my_str_tm.data)
    {
        if(length)
            *length = node->token->length;
        
        return &node->token->my_str_tm.data[node->token->begin];
    }
    
    if(length)
        *length = 0;
    
    return NULL;
}

myhtml_string_t * myhtml_node_string(myhtml_tree_node_t *node)
{
    if(node && node->token)
        return &node->token->my_str_tm;
    
    return NULL;
}

/*
 * Attributes
 */
myhtml_tree_attr_t * myhtml_attribute_next(myhtml_tree_attr_t *attr)
{
    return attr->next;
}

myhtml_tree_attr_t * myhtml_attribute_prev(myhtml_tree_attr_t *attr)
{
    return attr->prev;
}

enum myhtml_namespace myhtml_attribute_namespace(myhtml_tree_attr_t *attr)
{
    return attr->my_namespace;
}

const char * myhtml_attribute_name(myhtml_tree_attr_t *attr, size_t *length)
{
    if(attr->entry.data && attr->name_length)
    {
        if(length)
            *length = attr->name_length;
        
        return &attr->entry.data[attr->name_begin];
    }
    
    if(length)
        *length = 0;
    
    return NULL;
}

const char * myhtml_attribute_value(myhtml_tree_attr_t *attr, size_t *length)
{
    if(attr->entry.data && attr->value_length)
    {
        if(length)
            *length = attr->value_length;
        
        return &attr->entry.data[attr->value_begin];
    }
    
    if(length)
        *length = 0;
    
    return NULL;
}

myhtml_tree_attr_t * myhtml_attribute_by_key(myhtml_tree_node_t *node, const char *key, size_t key_len)
{
    if(node == NULL || node->token == NULL)
        return NULL;
    
    return myhtml_token_attr_by_name(node->token, key, key_len);
}

myhtml_tree_attr_t * myhtml_attribute_add(myhtml_tree_t *tree, myhtml_tree_node_t *node, const char *key, size_t key_len, const char *value, size_t value_len, myhtml_encoding_t encoding)
{
    if(node == NULL)
        return NULL;
    
    if(node->token == NULL) {
        mcobject_async_status_t mcstatus;
        node->token = (myhtml_token_node_t*)mcobject_async_malloc(tree->token->nodes_obj, tree->mcasync_token_id, &mcstatus);
        
        if(mcstatus)
            return NULL;
        
        myhtml_token_node_clean(node->token);
    }
    
    return myhtml_token_node_attr_append_with_convert_encoding(tree->token, node->token, key, key_len,
                                                               value, value_len, tree->mcasync_token_id, encoding);
}

myhtml_tree_attr_t * myhtml_attribute_remove(myhtml_tree_node_t *node, myhtml_tree_attr_t *attr)
{
    if(node == NULL || node->token == NULL)
        return NULL;
    
    return myhtml_token_attr_remove(node->token, attr);
}

myhtml_tree_attr_t * myhtml_attribute_remove_by_key(myhtml_tree_node_t *node, const char *key, size_t key_len)
{
    if(node == NULL || node->token == NULL)
        return NULL;
    
    return myhtml_token_attr_remove_by_name(node->token, key, key_len);
}

void myhtml_attribute_delete(myhtml_tree_t *tree, myhtml_tree_node_t *node, myhtml_tree_attr_t *attr)
{
    if(node == NULL || node->token == NULL)
        return;
    
    myhtml_token_attr_remove(node->token, attr);
    myhtml_attribute_free(tree, attr);
}

void myhtml_attribute_free(myhtml_tree_t *tree, myhtml_tree_attr_t *attr)
{
    mchar_async_free(attr->entry.mchar, attr->entry.node_idx, attr->entry.data);
    mcobject_async_free(tree->token->attr_obj, attr);
}

/*
 * Collections
 */
myhtml_collection_t * myhtml_collection_create(size_t size, myhtml_status_t *status)
{
    myhtml_collection_t *collection = (myhtml_collection_t*)malloc(sizeof(myhtml_collection_t));
    
    if(collection == NULL) {
        if(status)
            *status = MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
        
        return NULL;
    }
    
    collection->size   = size;
    collection->length = 0;
    collection->list   = (myhtml_tree_node_t **)mymalloc(sizeof(myhtml_tree_node_t*) * size);
    
    if(collection->list == NULL) {
        free(collection);
        
        if(status)
            *status = MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
        
        return NULL;
    }
    
    if(status)
        *status = MyHTML_STATUS_OK;
    
    return collection;
}

myhtml_status_t myhtml_collection_check_size(myhtml_collection_t *collection, size_t upto_length)
{
    upto_length = collection->length + upto_length;
    
    if(upto_length > collection->size)
    {
        size_t tmp_size = collection->size + upto_length;
        myhtml_tree_node_t **tmp = (myhtml_tree_node_t **)myrealloc(collection->list, sizeof(myhtml_tree_node_t*) * tmp_size);
        
        if(tmp) {
            collection->length  = upto_length;
            collection->size    = tmp_size;
            collection->list    = tmp;
        }
        else
            return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    }
    
    return MyHTML_STATUS_OK;
}

void myhtml_collection_clean(myhtml_collection_t *collection)
{
    if(collection)
        collection->length = 0;
}

myhtml_collection_t * myhtml_collection_destroy(myhtml_collection_t *collection)
{
    if(collection == NULL)
        return NULL;
    
    if(collection->list)
        free(collection->list);
    
    free(collection);
    
    return NULL;
}

/* queue */
void myhtml_queue_add(myhtml_tree_t *tree, const char *html, size_t begin, mythread_queue_node_t *qnode)
{
#ifndef MyHTML_BUILD_WITHOUT_THREADS
    
    if(tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) {
        myhtml_parser_worker(0, tree->current_qnode);
        while(myhtml_rules_tree_dispatcher(tree, tree->current_qnode->token)){};
        
        tree->current_qnode = mythread_queue_node_malloc_limit(tree->myhtml->thread, tree->queue, html, (tree->global_offset + begin), 4, NULL);
    }
    else {
        tree->current_qnode = mythread_queue_node_malloc_round(tree->myhtml->thread, tree->queue_entry, html, (tree->global_offset + begin), NULL);
    }

#else
    
    myhtml_parser_worker(0, tree->current_qnode);
    while(myhtml_rules_tree_dispatcher(tree, tree->current_qnode->token)){};
    
    tree->current_qnode = mythread_queue_node_malloc_limit(tree->myhtml->thread, tree->queue, html, (tree->global_offset + begin), 4, NULL);
    
#endif /* MyHTML_BUILD_WITHOUT_THREADS */
    
    tree->current_qnode->tree = tree;
    tree->current_qnode->prev = qnode;
    
    if(qnode)
        myhtml_tokenizer_calc_current_namespace(tree, qnode);
    
    myhtml_token_node_malloc(tree->token, tree->current_qnode->token, tree->token->mcasync_token_id);
}

bool myhtml_utils_strcmp(const char* ab, const char* to_lowercase, size_t size)
{
    size_t i = 0;
    
    for(;;) {
        if(i == size)
            return true;
        
        if((const unsigned char)(to_lowercase[i] > 0x40 && to_lowercase[i] < 0x5b ?
                                 (to_lowercase[i]|0x60) : to_lowercase[i]) != (const unsigned char)ab[i])
        {
            return false;
        }
        
        i++;
    }
    
    return false;
}

bool myhtml_is_html_node(myhtml_tree_node_t *node, myhtml_tag_id_t tag_id)
{
    if(node == NULL)
        return false;
    
    return node->tag_idx == tag_id && node->my_namespace == MyHTML_NAMESPACE_HTML;
}


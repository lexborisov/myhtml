/*
 Copyright 2015-2016 Alexander Borisov
 
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

#include "myhtml.h"

myhtml_t * myhtml_create(void)
{
    return (myhtml_t*)mymalloc(sizeof(myhtml_t));
}

myhtml_status_t myhtml_init(myhtml_t* myhtml, enum myhtml_options opt, size_t thread_count, size_t queue_size)
{
    myhtml_status_t status;
    
    myhtml->async_incoming_buf = mcobject_async_create();
    if(myhtml->async_incoming_buf == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    mcobject_async_status_t mcstatus = mcobject_async_init(myhtml->async_incoming_buf, 32, 1024, sizeof(myhtml_incoming_buf_t));
    if(mcstatus != MCOBJECT_ASYNC_STATUS_OK)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    myhtml->tags = myhtml_tag_create();
    if(myhtml->tags == NULL) {
        myhtml->parse_state_func = NULL;
        myhtml->insertion_func = NULL;
        myhtml->thread = NULL;
        
        return MyHTML_STATUS_TAGS_ERROR_MEMORY_ALLOCATION;
    }
    
    status = myhtml_tag_init(myhtml->tags);
    if(status) {
        myhtml->parse_state_func = NULL;
        myhtml->insertion_func = NULL;
        myhtml->thread = NULL;
        
        return status;
    }
    
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
    
    myhtml_clean(myhtml);
    
    return status;
}

void myhtml_clean(myhtml_t* myhtml)
{
    mythread_queue_clean(myhtml->thread->queue);
    mythread_clean(myhtml->thread);
    mcobject_async_node_all_clean(myhtml->async_incoming_buf);
}

myhtml_t* myhtml_destroy(myhtml_t* myhtml)
{
    if(myhtml == NULL)
        return NULL;
    
    mythread_destroy(myhtml->thread, mytrue);
    myhtml_tokenizer_state_destroy(myhtml);
    
    myhtml->async_incoming_buf  = mcobject_async_destroy(myhtml->async_incoming_buf, mytrue);
    myhtml->tags                = myhtml_tag_destroy(myhtml->tags);
    
    if(myhtml->insertion_func)
        free(myhtml->insertion_func);
    
    free(myhtml);
    
    return NULL;
}

myhtml_status_t myhtml_parse(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
        mythread_queue_clean(tree->queue);
        mythread_clean(tree->myhtml->thread);
    }
    
    myhtml_encoding_set(tree, encoding);
    
    myhtml_tokenizer_begin(tree, html, html_size);
    myhtml_tokenizer_end(tree);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_parse_fragment(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END)
    {
        myhtml_tree_clean(tree);
        mythread_queue_clean(tree->queue);
        mythread_clean(tree->myhtml->thread);
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
        mythread_queue_clean(tree->queue);
        mythread_clean(tree->myhtml->thread);
    }
    
    tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    myhtml_encoding_set(tree, encoding);
    
    myhtml_tokenizer_begin(tree, html, html_size);
    myhtml_tokenizer_end(tree);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_parse_fragment_single(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END)
    {
        myhtml_tree_clean(tree);
        mythread_queue_clean(tree->queue);
        mythread_clean(tree->myhtml->thread);
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
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END)
    {
        myhtml_tree_clean(tree);
        mythread_queue_clean(tree->queue);
        mythread_clean(tree->myhtml->thread);
    }
    
    myhtml_tokenizer_chunk(tree, html, html_size);
    
    return MyHTML_STATUS_OK;
}

myhtml_status_t myhtml_parse_chunk_fragment(myhtml_tree_t* tree, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END)
    {
        myhtml_tree_clean(tree);
        mythread_queue_clean(tree->queue);
        mythread_clean(tree->myhtml->thread);
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
 * Helpers
 */
myhtml_tag_t * myhtml_get_tag(myhtml_t* myhtml)
{
    if(myhtml)
        return myhtml->tags;
    
    return NULL;
}

/*
 * Nodes
 */
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
    mctree_index_t tag_ctx_idx = mctree_search_lowercase(tree->myhtml->tags->tree, html, length);
    
    return myhtml_get_nodes_by_tag_id(tree, collection, tag_ctx_idx, status);
}

/*
 * Manipulate Nodes
 */
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
    
    tree->foster_parenting = mytrue;
    target = myhtml_tree_appropriate_place_inserting_in_tree(tree, target, &mode);
    tree->foster_parenting = myfalse;
    
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
    
    myhtml_string_char_ref_chunk_t str_chunk = {0, 0, 0, NULL, encoding};
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
    if(tree == NULL || tree->myhtml == NULL || tree->myhtml->tags == NULL ||
       tree->myhtml->tags->context_length <= tag_id)
    {
        if(length)
            *length = 0;
        
        return NULL;
    }
    
    mctree_node_t* mctree_nodes = tree->myhtml->tags->tree->nodes;
    size_t mcid = tree->myhtml->tags->context[tag_id].mctree_id;
    
    if(length)
        *length = mctree_nodes[mcid].str_size;
    
    return mctree_nodes[mcid].str;
}

mybool_t myhtml_node_is_close_self(myhtml_tree_node_t *node)
{
    if(node->token)
        return (node->token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF);
    
    return myfalse;
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
        size_t tmp_size = collection->length + 1024;
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

// incoming buffer
void myhtml_incomming_buf_add(myhtml_t* myhtml, myhtml_tree_t* tree, myhtml_incoming_buf_t *current, const char *html, size_t html_size)
{
    tree->incoming_buf = mcobject_async_malloc(myhtml->async_incoming_buf, tree->mcasync_incoming_buf_id, NULL);
    
    tree->incoming_buf->size   = html_size;
    tree->incoming_buf->length = 0;
    tree->incoming_buf->data   = html;
    tree->incoming_buf->offset = tree->global_offset;
    
    if(current)
        current->next = tree->incoming_buf;
    
    tree->incoming_buf->prev = current;
    tree->incoming_buf->next = NULL;
}

void myhtml_tree_incomming_buf_clean(myhtml_tree_t* tree, myhtml_incoming_buf_t *current)
{
    memset(current, 0, sizeof(myhtml_incoming_buf_t));
}

const char * myhtml_tree_incomming_buf_get_last(myhtml_tree_t *tree, myhtml_incoming_buf_t *inc_buf, size_t current_offset, size_t len)
{
    if(current_offset >= len) {
        return &tree->incoming_buf->data[ (current_offset - len) ];
    }
    
    if(tree->temp_tag_name.data == NULL)
        myhtml_tree_temp_tag_name_init(&tree->temp_tag_name);
    else
        myhtml_tree_temp_tag_name_clean(&tree->temp_tag_name);
    
    inc_buf = inc_buf->prev;
    
    size_t tmp_len = len - current_offset;
    while (tmp_len)
    {
        if(inc_buf->size >= tmp_len)
            break;
        
        tmp_len -= inc_buf->size;
        inc_buf = inc_buf->prev;
    }
    
    myhtml_tree_temp_tag_name_append(&tree->temp_tag_name, &inc_buf->data[ (inc_buf->size - tmp_len) ], tmp_len);
    tmp_len = len - tmp_len;
    inc_buf = inc_buf->next;
    
    while (inc_buf && tmp_len)
    {
        if(tmp_len > inc_buf->size) {
            myhtml_tree_temp_tag_name_append(&tree->temp_tag_name, inc_buf->data, inc_buf->size);
            tmp_len -= inc_buf->size;
        }
        else {
            myhtml_tree_temp_tag_name_append(&tree->temp_tag_name, inc_buf->data, tmp_len);
            break;
        }
        
        inc_buf = inc_buf->next;
    }
    
    return tree->temp_tag_name.data;
}

const char * myhtml_tree_incomming_buf_make_data(myhtml_tree_t *tree, mythread_queue_node_t *qnode, size_t len)
{
    if(tree->incoming_buf->offset <= qnode->begin &&
       (len + qnode->begin) <= (tree->global_offset + tree->incoming_buf->size))
    {
        return &tree->incoming_buf->data[ (qnode->begin - tree->incoming_buf->offset) ];
    }
    
    if(tree->temp_tag_name.data == NULL)
        myhtml_tree_temp_tag_name_init(&tree->temp_tag_name);
    else
        myhtml_tree_temp_tag_name_clean(&tree->temp_tag_name);
    
    myhtml_incoming_buf_t *inc_buf = tree->incoming_buf;
    
    if(inc_buf == NULL)
        return NULL;
    
    // find original chunk
    while (inc_buf->offset > qnode->begin) {
        if(inc_buf->prev)
            inc_buf = inc_buf->prev;
        else
            break;
    }
    
    size_t relative_current_buf_offset = qnode->begin - inc_buf->offset;
    
    if((relative_current_buf_offset + len) <= inc_buf->size) {
        return &inc_buf->data[relative_current_buf_offset];
    }
    
    size_t relative_how_buf_left = inc_buf->size - relative_current_buf_offset;
    myhtml_tree_temp_tag_name_append(&tree->temp_tag_name, &inc_buf->data[relative_current_buf_offset], relative_how_buf_left);
    
    inc_buf = inc_buf->next;
    
    while (inc_buf)
    {
        if((len - relative_how_buf_left) <= inc_buf->size) {
            myhtml_tree_temp_tag_name_append(&tree->temp_tag_name, inc_buf->data, (len - relative_how_buf_left));
            break;
        }
        else {
            relative_how_buf_left += inc_buf->size;
            myhtml_tree_temp_tag_name_append(&tree->temp_tag_name, inc_buf->data, inc_buf->size);
        }
        
        inc_buf = inc_buf->next;
    }
    
    return tree->temp_tag_name.data;
}

mybool_t myhtml_utils_strcmp(const char* ab, const char* to_lowercase, size_t size)
{
    size_t i = 0;
    
    for(;;) {
        if(i == size)
            return mytrue;
        
        if((const unsigned char)(to_lowercase[i] > 0x40 && to_lowercase[i] < 0x5b ?
                                 (to_lowercase[i]|0x60) : to_lowercase[i]) != (const unsigned char)ab[i])
        {
            return myfalse;
        }
        
        i++;
    }
    
    return myfalse;
}

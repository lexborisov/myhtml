/*
 Copyright (C) 2015-2017 Alexander Borisov
 
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
    myhtml->marker = (myhtml_tree_node_t*)mycore_malloc(sizeof(myhtml_tree_node_t));
    
    if(myhtml->marker)
        myhtml_tree_node_clean(myhtml->marker);
}

void myhtml_destroy_marker(myhtml_t* myhtml)
{
    if(myhtml->marker)
        mycore_free(myhtml->marker);
}

#ifndef MyCORE_BUILD_WITHOUT_THREADS
mystatus_t myhtml_stream_create(myhtml_t* myhtml, mystatus_t* status, size_t count, size_t id_increase)
{
    if(count == 0) {
        myhtml->thread_stream = NULL;
        
        *status = MyHTML_STATUS_OK;
        return *status;
    }
    
    myhtml->thread_stream = mythread_create();
    if(myhtml->thread_stream == NULL)
        *status = MyCORE_STATUS_THREAD_ERROR_MEMORY_ALLOCATION;
    
    *status = mythread_init(myhtml->thread_stream, MyTHREAD_TYPE_STREAM, count, id_increase);
    
    if(*status)
        myhtml->thread_stream = mythread_destroy(myhtml->thread_stream, NULL, NULL, true);
    
    return *status;
}

mystatus_t myhtml_batch_create(myhtml_t* myhtml, mystatus_t* status, size_t count, size_t id_increase)
{
    if(count == 0) {
        myhtml->thread_batch = NULL;
        
        *status = MyHTML_STATUS_OK;
        return *status;
    }
    
    myhtml->thread_batch = mythread_create();
    if(myhtml->thread_stream == NULL) {
        myhtml->thread_stream = mythread_destroy(myhtml->thread_stream, NULL, NULL, true);
        *status = MyCORE_STATUS_THREAD_ERROR_MEMORY_ALLOCATION;
    }
    
    *status = mythread_init(myhtml->thread_batch, MyTHREAD_TYPE_BATCH, count, id_increase);
    
    if(*status)
        myhtml->thread_batch = mythread_destroy(myhtml->thread_batch , NULL, NULL, true);
    
    return *status;
}

mystatus_t myhtml_create_stream_and_batch(myhtml_t* myhtml, size_t stream_count, size_t batch_count)
{
    mystatus_t status;
    
    /* stream */
    if(myhtml_stream_create(myhtml, &status, stream_count, 0)) {
        return status;
    }
    
    /* batch */
    if(myhtml_batch_create(myhtml, &status, batch_count, stream_count)) {
        myhtml->thread_stream = mythread_destroy(myhtml->thread_stream, NULL, NULL, true);
        return status;
    }
    
    return status;
}
#endif /* if undef MyCORE_BUILD_WITHOUT_THREADS */

myhtml_t * myhtml_create(void)
{
    return (myhtml_t*)mycore_calloc(1, sizeof(myhtml_t));
}

mystatus_t myhtml_init(myhtml_t* myhtml, enum myhtml_options opt, size_t thread_count, size_t queue_size)
{
    mystatus_t status;
    
    myhtml->opt = opt;
    myhtml_init_marker(myhtml);
    
    status = myhtml_tokenizer_state_init(myhtml);
    if(status)
        return status;
    
    status = myhtml_rules_init(myhtml);

#ifdef MyCORE_BUILD_WITHOUT_THREADS
    
    myhtml->thread_stream = NULL;
    myhtml->thread_batch  = NULL;
    myhtml->thread_total  = 0;
    
#else /* if undef MyCORE_BUILD_WITHOUT_THREADS */
    if(status)
        return status;

    if(thread_count == 0) {
        thread_count = 1;
    }

    switch (opt) {
        case MyHTML_OPTIONS_PARSE_MODE_SINGLE:
            if((status = myhtml_create_stream_and_batch(myhtml, 0, 0)))
                return status;
            
            break;
            
        case MyHTML_OPTIONS_PARSE_MODE_ALL_IN_ONE:
            if((status = myhtml_create_stream_and_batch(myhtml, 1, 0)))
                return status;
            
            myhtml->thread_stream->context = mythread_queue_list_create(&status);
            status = myhread_entry_create(myhtml->thread_stream, mythread_function_queue_stream, myhtml_parser_worker_stream, MyTHREAD_OPT_STOP);
            
            break;
            
        default:
            // default MyHTML_OPTIONS_PARSE_MODE_SEPARATELY
            if(thread_count < 2)
                thread_count = 2;
            
            if((status = myhtml_create_stream_and_batch(myhtml, 1, (thread_count - 1))))
                return status;
            
            myhtml->thread_stream->context = mythread_queue_list_create(&status);
            myhtml->thread_batch->context  = myhtml->thread_stream->context;
            
            status = myhread_entry_create(myhtml->thread_stream, mythread_function_queue_stream, myhtml_parser_stream, MyTHREAD_OPT_STOP);
            if(status)
                return status;
            
            for(size_t i = 0; i < myhtml->thread_batch->entries_size; i++) {
                status = myhread_entry_create(myhtml->thread_batch, mythread_function_queue_batch, myhtml_parser_worker, MyTHREAD_OPT_STOP);
                
                if(status)
                    return status;
            }
            
            break;
    }
    
    myhtml->thread_total = thread_count;
    
    myhtml->thread_list[0] = myhtml->thread_stream;
    myhtml->thread_list[1] = myhtml->thread_batch;
    myhtml->thread_list[2] = NULL;
    
#endif /* if undef MyCORE_BUILD_WITHOUT_THREADS */
    
    if(status)
        return status;
    
    myhtml_clean(myhtml);
    
    return status;
}

void myhtml_clean(myhtml_t* myhtml)
{
    /* some code */
}

myhtml_t* myhtml_destroy(myhtml_t* myhtml)
{
    if(myhtml == NULL)
        return NULL;
    
    myhtml_destroy_marker(myhtml);
    
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    if(myhtml->thread_stream) {
        mythread_queue_list_t* queue_list = myhtml->thread_stream->context;

        if(queue_list)
            mythread_queue_list_wait_for_done(myhtml->thread_stream, queue_list);
        
        myhtml->thread_stream = mythread_destroy(myhtml->thread_stream, mythread_callback_quit, NULL, true);
        
        if(myhtml->thread_batch)
            myhtml->thread_batch = mythread_destroy(myhtml->thread_batch, mythread_callback_quit, NULL, true);
        
        if(queue_list)
            mythread_queue_list_destroy(queue_list);
    }
#endif /* if undef MyCORE_BUILD_WITHOUT_THREADS */
    
    myhtml_tokenizer_state_destroy(myhtml);
    
    if(myhtml->insertion_func)
        mycore_free(myhtml->insertion_func);
    
    mycore_free(myhtml);
    
    return NULL;
}

mystatus_t myhtml_parse(myhtml_tree_t* tree, myencoding_t encoding, const char* html, size_t html_size)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    myhtml_encoding_set(tree, encoding);
    mystatus_t status = myhtml_tokenizer_begin(tree, html, html_size);
    
    if(status)
        return status;
    
    return myhtml_tokenizer_end(tree);
}

mystatus_t myhtml_parse_fragment(myhtml_tree_t* tree, myencoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace ns)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    if(tag_id == 0)
        tag_id = MyHTML_TAG_DIV;
    
    if(ns == 0)
        ns = MyHTML_NAMESPACE_HTML;
    
    if(myhtml_tokenizer_fragment_init(tree, tag_id, ns) == NULL)
        return MyHTML_STATUS_TOKENIZER_ERROR_FRAGMENT_INIT;
    
    myhtml_encoding_set(tree, encoding);
    mystatus_t status = myhtml_tokenizer_begin(tree, html, html_size);
    
    if(status)
        return status;
    
    return myhtml_tokenizer_end(tree);
}

mystatus_t myhtml_parse_single(myhtml_tree_t* tree, myencoding_t encoding, const char* html, size_t html_size)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    myhtml_encoding_set(tree, encoding);
    
    mystatus_t status = myhtml_tokenizer_begin(tree, html, html_size);
    
    if(status)
        return status;
    
    return myhtml_tokenizer_end(tree);
}

mystatus_t myhtml_parse_fragment_single(myhtml_tree_t* tree, myencoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace ns)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    if(tag_id == 0)
        tag_id = MyHTML_TAG_DIV;
    
    if(ns == 0)
        ns = MyHTML_NAMESPACE_HTML;
    
    tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    if(myhtml_tokenizer_fragment_init(tree, tag_id, ns) == NULL)
        return MyHTML_STATUS_TOKENIZER_ERROR_FRAGMENT_INIT;
    
    myhtml_encoding_set(tree, encoding);
    
    mystatus_t status = myhtml_tokenizer_begin(tree, html, html_size);
    
    if(status)
        return status;
    
    return myhtml_tokenizer_end(tree);
}

mystatus_t myhtml_parse_chunk(myhtml_tree_t* tree, const char* html, size_t html_size)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    return  myhtml_tokenizer_chunk(tree, html, html_size);
}

mystatus_t myhtml_parse_chunk_fragment(myhtml_tree_t* tree, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace ns)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_END) {
        myhtml_tree_clean(tree);
    }
    
    if(tag_id == 0)
        tag_id = MyHTML_TAG_DIV;
    
    if(ns == 0)
        ns = MyHTML_NAMESPACE_HTML;
    
    if(myhtml_tokenizer_fragment_init(tree, tag_id, ns) == NULL)
        return MyHTML_STATUS_TOKENIZER_ERROR_FRAGMENT_INIT;
    
    return myhtml_tokenizer_chunk(tree, html, html_size);
}

mystatus_t myhtml_parse_chunk_single(myhtml_tree_t* tree, const char* html, size_t html_size)
{
    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
        tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    return myhtml_parse_chunk(tree, html, html_size);
}

mystatus_t myhtml_parse_chunk_fragment_single(myhtml_tree_t* tree, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace ns)
{
    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
        tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    return myhtml_parse_chunk_fragment(tree, html, html_size, tag_id, ns);
}

mystatus_t myhtml_parse_chunk_end(myhtml_tree_t* tree)
{
    return myhtml_tokenizer_end(tree);
}

void myhtml_encoding_set(myhtml_tree_t* tree, myencoding_t encoding)
{
    if(encoding >= MyENCODING_LAST_ENTRY)
        return;
    
    tree->encoding_usereq = encoding;
    tree->encoding        = encoding;
}

myencoding_t myhtml_encoding_get(myhtml_tree_t* tree)
{
    return tree->encoding;
}

/*
 * Nodes
 */

mystatus_t myhtml_get_nodes_by_tag_id_in_scope_find_recursion(myhtml_tree_node_t *node, myhtml_collection_t *collection, myhtml_tag_id_t tag_id)
{
    while(node) {
        if(node->tag_id == tag_id) {
            collection->list[ collection->length ] = node;
            collection->length++;
            
            if(collection->length >= collection->size)
            {
                mystatus_t mystatus = myhtml_collection_check_size(collection, 1024, 0);
                
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

myhtml_collection_t * myhtml_get_nodes_by_tag_id_in_scope(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tree_node_t *node, myhtml_tag_id_t tag_id, mystatus_t *status)
{
    if(node == NULL)
        return NULL;
    
    mystatus_t mystatus = MyHTML_STATUS_OK;
    
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

myhtml_collection_t * myhtml_get_nodes_by_name_in_scope(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tree_node_t *node, const char* html, size_t length, mystatus_t *status)
{
    const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_name(tree->tags, html, length);
    if(tag_ctx == NULL) {
        return NULL;
    }
    return myhtml_get_nodes_by_tag_id_in_scope(tree, collection, node, tag_ctx->id, status);
}

myhtml_collection_t * myhtml_get_nodes_by_tag_id(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tag_id_t tag_id, mystatus_t *status)
{
    if(collection == NULL) {
        collection = myhtml_collection_create(1024, NULL);
        
        if(collection == NULL)
            return NULL;
    }
    
    myhtml_tree_node_t *node = tree->node_html;
    
    while(node)
    {
        if(node->tag_id == tag_id)
        {
            if(myhtml_collection_check_size(collection, 1, 1024) == MyHTML_STATUS_OK) {
                collection->list[ collection->length ] = node;
                collection->length++;
            }
            else {
                if(status)
                    *status = MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
                
                return collection;
            }
        }
        
        if(node->child)
            node = node->child;
        else {
            while(node != tree->node_html && node->next == NULL)
                node = node->parent;
            
            if(node == tree->node_html)
                break;
            
            node = node->next;
        }
    }
    
    if(myhtml_collection_check_size(collection, 1, 1024) == MyHTML_STATUS_OK) {
        collection->list[ collection->length ] = NULL;
    }
    else if(status) {
        *status = MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    }
    
    return collection;
}

myhtml_collection_t * myhtml_get_nodes_by_name(myhtml_tree_t* tree, myhtml_collection_t *collection, const char* html, size_t length, mystatus_t *status)
{
    const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_name(tree->tags, html, length);
    
    if(tag_ctx == NULL)
        return NULL;
    
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

myhtml_tree_node_t * myhtml_node_create(myhtml_tree_t* tree, myhtml_tag_id_t tag_id, enum myhtml_namespace ns)
{
    myhtml_tree_node_t *node = myhtml_tree_node_create(tree);
    
    node->tag_id      = tag_id;
    node->ns = ns;
    
    return node;
}

myhtml_tree_node_t * myhtml_node_remove(myhtml_tree_node_t *node)
{
    return myhtml_tree_node_remove(node);
}

void myhtml_node_delete(myhtml_tree_node_t *node)
{
    myhtml_tree_node_delete(node);
}

void myhtml_node_delete_recursive(myhtml_tree_node_t *node)
{
    myhtml_tree_node_delete_recursive(node);
}

void myhtml_node_free(myhtml_tree_node_t *node)
{
    myhtml_tree_node_free(node);
}

myhtml_tree_node_t * myhtml_node_insert_before(myhtml_tree_node_t *target, myhtml_tree_node_t *node)
{
    if(target == NULL || node == NULL)
        return NULL;
    
    myhtml_tree_node_insert_before(target, node);
    
    return node;
}

myhtml_tree_node_t * myhtml_node_insert_after(myhtml_tree_node_t *target, myhtml_tree_node_t *node)
{
    if(target == NULL || node == NULL)
        return NULL;
    
    myhtml_tree_node_insert_after(target, node);
    
    return node;
}

myhtml_tree_node_t * myhtml_node_append_child(myhtml_tree_node_t *target, myhtml_tree_node_t *node)
{
    if(target == NULL || node == NULL)
        return NULL;
    
    myhtml_tree_node_add_child(target, node);
    
    return node;
}

myhtml_tree_node_t * myhtml_node_insert_to_appropriate_place(myhtml_tree_node_t *target, myhtml_tree_node_t *node)
{
    if(target == NULL || node == NULL)
        return NULL;
    
    enum myhtml_tree_insertion_mode mode;
    
    target->tree->foster_parenting = true;
    target = myhtml_tree_appropriate_place_inserting_in_tree(target, &mode);
    target->tree->foster_parenting = false;
    
    myhtml_tree_node_insert_by_mode(target, node, mode);
    
    return node;
}

mycore_string_t * myhtml_node_text_set(myhtml_tree_node_t *node, const char* text, size_t length, myencoding_t encoding)
{
    if(node == NULL)
        return NULL;
    
    if(encoding >= MyENCODING_LAST_ENTRY)
        return NULL;
    
    myhtml_tree_t* tree = node->tree;
    
    if(node->token == NULL) {
        node->token = myhtml_token_node_create(tree->token, tree->mcasync_rules_token_id);
        
        if(node->token == NULL)
            return NULL;
     
        node->token->type |= MyHTML_TOKEN_TYPE_DONE;
    }
    
    if(node->token->str.data == NULL) {
        mycore_string_init(tree->mchar, tree->mchar_node_id, &node->token->str, (length + 2));
    }
    else {
        if(node->token->str.size < length) {
            mchar_async_free(tree->mchar, node->token->str.node_idx, node->token->str.data);
            mycore_string_init(tree->mchar, tree->mchar_node_id, &node->token->str, length);
        }
        else
            node->token->str.length = 0;
    }
    
    if(encoding != MyENCODING_UTF_8) {
        myencoding_string_append(&node->token->str, text, length, encoding);
    }
    else {
        mycore_string_append(&node->token->str, text, length);
    }
    
    node->token->raw_begin  = 0;
    node->token->raw_length = 0;
    
    return &node->token->str;
}

mycore_string_t * myhtml_node_text_set_with_charef(myhtml_tree_node_t *node, const char* text, size_t length, myencoding_t encoding)
{
    if(node == NULL)
        return NULL;
    
    if(encoding >= MyENCODING_LAST_ENTRY)
        return NULL;
    
    myhtml_tree_t* tree = node->tree;
    
    if(node->token == NULL) {
        node->token = myhtml_token_node_create(tree->token, tree->mcasync_rules_token_id);
        
        if(node->token == NULL)
            return NULL;
        
        node->token->type |= MyHTML_TOKEN_TYPE_DONE;
    }
    
    if(node->token->str.data == NULL) {
        mycore_string_init(tree->mchar, tree->mchar_node_id, &node->token->str, (length + 2));
    }
    else {
        if(node->token->str.size < length) {
            mchar_async_free(tree->mchar, node->token->str.node_idx, node->token->str.data);
            mycore_string_init(tree->mchar, tree->mchar_node_id, &node->token->str, length);
        }
        else
            node->token->str.length = 0;
    }
    
    myhtml_data_process_entry_t proc_entry;
    myhtml_data_process_entry_clean(&proc_entry);
    
    proc_entry.encoding = encoding;
    myencoding_result_clean(&proc_entry.res);
    
    myhtml_data_process(&proc_entry, &node->token->str, text, length);
    myhtml_data_process_end(&proc_entry, &node->token->str);
    
    node->token->raw_begin  = 0;
    node->token->raw_length = 0;
    
    return &node->token->str;
}

myhtml_token_node_t* myhtml_node_token(myhtml_tree_node_t *node)
{
    return node->token;
}

myhtml_namespace_t myhtml_node_namespace(myhtml_tree_node_t *node)
{
    return node->ns;
}

void myhtml_node_namespace_set(myhtml_tree_node_t *node, myhtml_namespace_t ns)
{
    node->ns = ns;
}

myhtml_tag_id_t myhtml_node_tag_id(myhtml_tree_node_t *node)
{
    return node->tag_id;
}

const char * myhtml_tag_name_by_id(myhtml_tree_t* tree, myhtml_tag_id_t tag_id, size_t *length)
{
    if(length)
        *length = 0;
    
    if(tree == NULL || tree->tags == NULL)
        return NULL;
    
    const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, tag_id);
    
    if(tag_ctx == NULL)
        return NULL;
    
    if(length)
        *length = tag_ctx->name_length;
    
    return tag_ctx->name;
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

bool myhtml_node_is_void_element(myhtml_tree_node_t *node)
{
    // http://w3c.github.io/html-reference/syntax.html#void-elements
    switch (node->tag_id)
    {
        case MyHTML_TAG_AREA:
        case MyHTML_TAG_BASE:
        case MyHTML_TAG_BR:
        case MyHTML_TAG_COL:
        case MyHTML_TAG_COMMAND:
        case MyHTML_TAG_EMBED:
        case MyHTML_TAG_HR:
        case MyHTML_TAG_IMG:
        case MyHTML_TAG_INPUT:
        case MyHTML_TAG_KEYGEN:
        case MyHTML_TAG_LINK:
        case MyHTML_TAG_META:
        case MyHTML_TAG_PARAM:
        case MyHTML_TAG_SOURCE:
        case MyHTML_TAG_TRACK:
        case MyHTML_TAG_WBR:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
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
        return node->token->attr_last;
    
    return NULL;
}

const char * myhtml_node_text(myhtml_tree_node_t *node, size_t *length)
{
    if(node->token && node->token->str.length && node->token->str.data)
    {
        if(length)
            *length = node->token->str.length;
        
        return node->token->str.data;
    }
    
    if(length)
        *length = 0;
    
    return NULL;
}

mycore_string_t * myhtml_node_string(myhtml_tree_node_t *node)
{
    if(node && node->token)
        return &node->token->str;
    
    return NULL;
}

myhtml_position_t myhtml_node_raw_position(myhtml_tree_node_t *node)
{
    if(node && node->token)
        return (myhtml_position_t){node->token->raw_begin, node->token->raw_length};
    
    return (myhtml_position_t){0, 0};
}

myhtml_position_t myhtml_node_element_position(myhtml_tree_node_t *node)
{
    if(node && node->token)
        return (myhtml_position_t){node->token->element_begin, node->token->element_length};
    
    return (myhtml_position_t){0, 0};
}

void myhtml_node_set_data(myhtml_tree_node_t *node, void* data)
{
    node->data = data;
}

void * myhtml_node_get_data(myhtml_tree_node_t *node)
{
    return node->data;
}

myhtml_tree_t * myhtml_node_tree(myhtml_tree_node_t *node)
{
    return node->tree;
}

mystatus_t myhtml_get_nodes_by_attribute_key_recursion(myhtml_tree_node_t* node, myhtml_collection_t* collection, const char* key, size_t key_len)
{
    myhtml_tree_node_t *root = node;

    while(node != NULL) {
        if(node->token && node->token->attr_first) {
            myhtml_tree_attr_t* attr = node->token->attr_first;

            while(attr) {
                mycore_string_t* str_key = &attr->key;

                if(str_key->length == key_len && mycore_strncasecmp(str_key->data, key, key_len) == 0) {
                    collection->list[ collection->length ] = node;

                    collection->length++;
                    if(collection->length >= collection->size) {
                        mystatus_t status = myhtml_collection_check_size(collection, 1024, 0);

                        if(status)
                            return status;
                    }
                }

                attr = attr->next;
            }
        }

        if(node->child != NULL) {
            node = node->child;
        }
        else {
            while(node->next == NULL) {
                node = node->parent;

                if(node == root) {
                    return MyHTML_STATUS_OK;
                }
            }

            node = node->next;
        }
    }

    return MyHTML_STATUS_OK;
}

myhtml_collection_t * myhtml_get_nodes_by_attribute_key(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* scope_node, const char* key, size_t key_len, mystatus_t* status)
{
    if(collection == NULL) {
        collection = myhtml_collection_create(1024, status);
        
        if((status && *status) || collection == NULL)
            return NULL;
    }
    
    if(scope_node == NULL)
        scope_node = tree->node_html;
    
    mystatus_t rec_status = myhtml_get_nodes_by_attribute_key_recursion(scope_node, collection, key, key_len);
    
    if(rec_status && status)
        *status = rec_status;
    
    return collection;
}

/* find by attribute value; case-sensitivity */
bool myhtml_get_nodes_by_attribute_value_recursion_eq(mycore_string_t* str, const char* value, size_t value_len)
{
    return str->length == value_len && mycore_strncmp(str->data, value, value_len) == 0;
}

bool myhtml_get_nodes_by_attribute_value_recursion_whitespace_separated(mycore_string_t* str, const char* value, size_t value_len)
{
    if(str->length < value_len)
        return false;
    
    const char *data = str->data;
    
    if(mycore_strncmp(data, value, value_len) == 0) {
        if((str->length > value_len && mycore_utils_whithspace(data[value_len], ==, ||)) || str->length == value_len)
            return true;
    }
    
    for(size_t i = 1; (str->length - i) >= value_len; i++)
    {
        if(mycore_utils_whithspace(data[(i - 1)], ==, ||)) {
            if(mycore_strncmp(&data[i], value, value_len) == 0) {
                if((i > value_len && mycore_utils_whithspace(data[(i + value_len)], ==, ||)) || (str->length - i) == value_len)
                    return true;
            }
        }
    }
    
    return false;
}

bool myhtml_get_nodes_by_attribute_value_recursion_begin(mycore_string_t* str, const char* value, size_t value_len)
{
    if(str->length < value_len)
        return false;
    
    return mycore_strncmp(str->data, value, value_len) == 0;
}

bool myhtml_get_nodes_by_attribute_value_recursion_end(mycore_string_t* str, const char* value, size_t value_len)
{
    if(str->length < value_len)
        return false;
    
    return mycore_strncmp(&str->data[ (str->length - value_len) ], value, value_len) == 0;
}

bool myhtml_get_nodes_by_attribute_value_recursion_contain(mycore_string_t* str, const char* value, size_t value_len)
{
    if(str->length < value_len)
        return false;
    
    const char *data = str->data;
    
    for(size_t i = 0; (str->length - i) >= value_len; i++)
    {
        if(mycore_strncmp(&data[i], value, value_len) == 0) {
            return true;
        }
    }
    
    return false;
}

bool myhtml_get_nodes_by_attribute_value_recursion_hyphen_separated(mycore_string_t* str, const char* value, size_t value_len)
{
    const char *data = str->data;
    
    if(str->length < value_len)
        return false;
    else if(str->length == value_len && mycore_strncmp(data, value, value_len) == 0) {
        return true;
    }
    else if(mycore_strncmp(data, value, value_len) == 0 && data[value_len] == '-') {
        return true;
    }
    
    return false;
}

/* find by attribute value; case-insensitive */
bool myhtml_get_nodes_by_attribute_value_recursion_eq_i(mycore_string_t* str, const char* value, size_t value_len)
{
    return str->length == value_len && mycore_strncasecmp(str->data, value, value_len) == 0;
}

bool myhtml_get_nodes_by_attribute_value_recursion_whitespace_separated_i(mycore_string_t* str, const char* value, size_t value_len)
{
    if(str->length < value_len)
        return false;
    
    const char *data = str->data;
    
    if(mycore_strncasecmp(data, value, value_len) == 0) {
        if((str->length > value_len && mycore_utils_whithspace(data[value_len], ==, ||)) || str->length == value_len)
            return true;
    }
    
    for(size_t i = 1; (str->length - i) >= value_len; i++)
    {
        if(mycore_utils_whithspace(data[(i - 1)], ==, ||)) {
            if(mycore_strncasecmp(&data[i], value, value_len) == 0) {
                if((i > value_len && mycore_utils_whithspace(data[(i + value_len)], ==, ||)) || (str->length - i) == value_len)
                    return true;
            }
        }
    }
    
    return false;
}

bool myhtml_get_nodes_by_attribute_value_recursion_begin_i(mycore_string_t* str, const char* value, size_t value_len)
{
    if(str->length < value_len)
        return false;
    
    return mycore_strncasecmp(str->data, value, value_len) == 0;
}

bool myhtml_get_nodes_by_attribute_value_recursion_end_i(mycore_string_t* str, const char* value, size_t value_len)
{
    if(str->length < value_len)
        return false;
    
    return mycore_strncasecmp(&str->data[ (str->length - value_len) ], value, value_len) == 0;
}

bool myhtml_get_nodes_by_attribute_value_recursion_contain_i(mycore_string_t* str, const char* value, size_t value_len)
{
    if(str->length < value_len)
        return false;
    
    const char *data = str->data;
    
    for(size_t i = 0; (str->length - i) >= value_len; i++)
    {
        if(mycore_strncasecmp(&data[i], value, value_len) == 0) {
            return true;
        }
    }
    
    return false;
}

bool myhtml_get_nodes_by_attribute_value_recursion_hyphen_separated_i(mycore_string_t* str, const char* value, size_t value_len)
{
    const char *data = str->data;
    
    if(str->length < value_len)
        return false;
    else if(str->length == value_len && mycore_strncasecmp(data, value, value_len) == 0) {
        return true;
    }
    else if(mycore_strncasecmp(data, value, value_len) == 0 && data[value_len] == '-') {
        return true;
    }
    
    return false;
}

/* find by attribute value; basic functions */
mystatus_t myhtml_get_nodes_by_attribute_value_recursion(myhtml_tree_node_t* node, myhtml_collection_t* collection,
                                                         myhtml_attribute_value_find_f func_eq,
                                                         const char* value, size_t value_len)
{
    myhtml_tree_node_t *root = node;
    
    while(node != NULL) {
        if(node->token && node->token->attr_first) {
            myhtml_tree_attr_t* attr = node->token->attr_first;

            while(attr) {
                mycore_string_t* str = &attr->value;

                if(func_eq(str, value, value_len)) {
                    collection->list[ collection->length ] = node;

                    collection->length++;
                    if(collection->length >= collection->size) {
                        mystatus_t status = myhtml_collection_check_size(collection, 1024, 0);

                        if(status)
                            return status;
                    }
                }
                
                attr = attr->next;
            }
        }

        if(node->child != NULL) {
            node = node->child;
        }
        else {
            while(node->next == NULL) {
                node = node->parent;

                if(node == root) {
                    return MyHTML_STATUS_OK;
                }
            }

            node = node->next;
        }
    }

    return MyHTML_STATUS_OK;
}

/* TODO: need to rename function. Remove recursion word */
mystatus_t myhtml_get_nodes_by_attribute_value_recursion_by_key(myhtml_tree_node_t* node, myhtml_collection_t* collection,
                                                                myhtml_attribute_value_find_f func_eq,
                                                                const char* key, size_t key_len,
                                                                const char* value, size_t value_len)
{
    myhtml_tree_node_t *root = node;
    
    while(node != NULL) {
        if(node->token && node->token->attr_first) {
            myhtml_tree_attr_t* attr = node->token->attr_first;

            while(attr) {
                mycore_string_t* str_key = &attr->key;
                mycore_string_t* str = &attr->value;

                if(str_key->length == key_len && mycore_strncasecmp(str_key->data, key, key_len) == 0)
                {
                    if(func_eq(str, value, value_len)) {
                        collection->list[ collection->length ] = node;

                        collection->length++;
                        if(collection->length >= collection->size) {
                            mystatus_t status = myhtml_collection_check_size(collection, 1024, 0);

                            if(status)
                                return status;
                        }
                    }
                }

                attr = attr->next;
            }
        }

        if(node->child != NULL) {
            node = node->child;
        }
        else {
            while(node->next == NULL) {
                node = node->parent;
                
                if(node == root) {
                    return MyHTML_STATUS_OK;
                }
            }
            
            node = node->next;
        }
    }

    return MyHTML_STATUS_OK;
}

myhtml_collection_t * _myhtml_get_nodes_by_attribute_value(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                           myhtml_attribute_value_find_f func_eq,
                                                           const char* key, size_t key_len,
                                                           const char* value, size_t value_len,
                                                           mystatus_t* status)
{
    if(collection == NULL) {
        collection = myhtml_collection_create(1024, status);
        
        if((status && *status) || collection == NULL)
            return NULL;
    }
    
    if(node == NULL)
        node = tree->node_html;
    
    mystatus_t rec_status;
    
    if(key && key_len)
        rec_status = myhtml_get_nodes_by_attribute_value_recursion_by_key(node, collection, func_eq, key, key_len, value, value_len);
    else
        rec_status = myhtml_get_nodes_by_attribute_value_recursion(node, collection, func_eq, value, value_len);
    
    if(rec_status && status)
        *status = rec_status;
    
    return collection;
}

myhtml_collection_t * myhtml_get_nodes_by_attribute_value(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                          bool case_insensitive,
                                                          const char* key, size_t key_len,
                                                          const char* value, size_t value_len,
                                                          mystatus_t* status)
{
    if(case_insensitive) {
        return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                    myhtml_get_nodes_by_attribute_value_recursion_eq_i,
                                                    key, key_len, value, value_len, status);
    }
    
    return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                myhtml_get_nodes_by_attribute_value_recursion_eq,
                                                key, key_len, value, value_len, status);
}

myhtml_collection_t * myhtml_get_nodes_by_attribute_value_whitespace_separated(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                                               bool case_insensitive,
                                                                               const char* key, size_t key_len,
                                                                               const char* value, size_t value_len,
                                                                               mystatus_t* status)
{
    if(case_insensitive) {
        return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                    myhtml_get_nodes_by_attribute_value_recursion_whitespace_separated_i,
                                                    key, key_len, value, value_len, status);
    }
    
    return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                myhtml_get_nodes_by_attribute_value_recursion_whitespace_separated,
                                                key, key_len, value, value_len, status);
}

myhtml_collection_t * myhtml_get_nodes_by_attribute_value_begin(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                                bool case_insensitive,
                                                                const char* key, size_t key_len,
                                                                const char* value, size_t value_len,
                                                                mystatus_t* status)
{
    if(case_insensitive) {
        return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                    myhtml_get_nodes_by_attribute_value_recursion_begin_i,
                                                    key, key_len, value, value_len, status);
    }
    
    return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                myhtml_get_nodes_by_attribute_value_recursion_begin,
                                                key, key_len, value, value_len, status);
}

myhtml_collection_t * myhtml_get_nodes_by_attribute_value_end(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                              bool case_insensitive,
                                                              const char* key, size_t key_len,
                                                              const char* value, size_t value_len,
                                                              mystatus_t* status)
{
    if(case_insensitive) {
        return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                    myhtml_get_nodes_by_attribute_value_recursion_end_i,
                                                    key, key_len, value, value_len, status);
    }
    
    return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                myhtml_get_nodes_by_attribute_value_recursion_end,
                                                key, key_len, value, value_len, status);
}

myhtml_collection_t * myhtml_get_nodes_by_attribute_value_contain(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                                  bool case_insensitive,
                                                                  const char* key, size_t key_len,
                                                                  const char* value, size_t value_len,
                                                                  mystatus_t* status)
{
    if(case_insensitive) {
        return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                    myhtml_get_nodes_by_attribute_value_recursion_contain_i,
                                                    key, key_len, value, value_len, status);
    }
    
    return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                myhtml_get_nodes_by_attribute_value_recursion_contain,
                                                key, key_len, value, value_len, status);
}

myhtml_collection_t * myhtml_get_nodes_by_attribute_value_hyphen_separated(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                                           bool case_insensitive,
                                                                           const char* key, size_t key_len,
                                                                           const char* value, size_t value_len,
                                                                           mystatus_t* status)
{
    if(case_insensitive) {
        return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                    myhtml_get_nodes_by_attribute_value_recursion_hyphen_separated_i,
                                                    key, key_len, value, value_len, status);
    }
    
    return _myhtml_get_nodes_by_attribute_value(tree, collection, node,
                                                myhtml_get_nodes_by_attribute_value_recursion_hyphen_separated,
                                                key, key_len, value, value_len, status);
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
    return attr->ns;
}

void myhtml_attribute_namespace_set(myhtml_tree_attr_t *attr, myhtml_namespace_t ns)
{
    attr->ns = ns;
}

const char * myhtml_attribute_key(myhtml_tree_attr_t *attr, size_t *length)
{
    if(attr->key.data && attr->key.length)
    {
        if(length)
            *length = attr->key.length;
        
        return attr->key.data;
    }
    
    if(length)
        *length = 0;
    
    return NULL;
}

const char * myhtml_attribute_value(myhtml_tree_attr_t *attr, size_t *length)
{
    if(attr->value.data && attr->value.length)
    {
        if(length)
            *length = attr->value.length;
        
        return attr->value.data;
    }
    
    if(length)
        *length = 0;
    
    return NULL;
}

mycore_string_t * myhtml_attribute_key_string(myhtml_tree_attr_t* attr)
{
    if(attr)
        return &attr->key;
    
    return NULL;
}

mycore_string_t * myhtml_attribute_value_string(myhtml_tree_attr_t* attr)
{
    if(attr)
        return &attr->value;
    
    return NULL;
}

myhtml_tree_attr_t * myhtml_attribute_by_key(myhtml_tree_node_t *node, const char *key, size_t key_len)
{
    if(node == NULL || node->token == NULL)
        return NULL;
    
    return myhtml_token_attr_by_name(node->token, key, key_len);
}

myhtml_tree_attr_t * myhtml_attribute_add(myhtml_tree_node_t *node, const char *key, size_t key_len, const char *value, size_t value_len, myencoding_t encoding)
{
    if(node == NULL)
        return NULL;
    
    myhtml_tree_t *tree = node->tree;
    
    if(node->token == NULL) {
        node->token = myhtml_token_node_create(tree->token, tree->mcasync_rules_token_id);
        
        if(node->token == NULL)
            return NULL;
        
        node->token->type |= MyHTML_TOKEN_TYPE_DONE;
    }
    
    return myhtml_token_node_attr_append_with_convert_encoding(tree->token, node->token, key, key_len,
                                                               value, value_len, tree->mcasync_rules_token_id, encoding);
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
    if(attr->key.data)
        mchar_async_free(attr->key.mchar, attr->key.node_idx, attr->key.data);
    if(attr->value.data)
        mchar_async_free(attr->value.mchar, attr->value.node_idx, attr->value.data);
    
    mcobject_async_free(tree->token->attr_obj, attr);
}

myhtml_position_t myhtml_attribute_key_raw_position(myhtml_tree_attr_t *attr)
{
    if(attr)
        return (myhtml_position_t){attr->raw_key_begin, attr->raw_key_length};
    
    return (myhtml_position_t){0, 0};
}

myhtml_position_t myhtml_attribute_value_raw_position(myhtml_tree_attr_t *attr)
{
    if(attr)
        return (myhtml_position_t){attr->raw_value_begin, attr->raw_value_length};
    
    return (myhtml_position_t){0, 0};
}

/*
 * Collections
 */
myhtml_collection_t * myhtml_collection_create(size_t size, mystatus_t *status)
{
    myhtml_collection_t *collection = (myhtml_collection_t*)mycore_malloc(sizeof(myhtml_collection_t));
    
    if(collection == NULL) {
        if(status)
            *status = MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
        
        return NULL;
    }
    
    collection->size   = size;
    collection->length = 0;
    collection->list   = (myhtml_tree_node_t **)mycore_malloc(sizeof(myhtml_tree_node_t*) * size);
    
    if(collection->list == NULL) {
        mycore_free(collection);
        
        if(status)
            *status = MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
        
        return NULL;
    }
    
    if(status)
        *status = MyHTML_STATUS_OK;
    
    return collection;
}

mystatus_t myhtml_collection_check_size(myhtml_collection_t *collection, size_t need, size_t upto_length)
{
    if((collection->length + need) >= collection->size)
    {
        size_t tmp_size = collection->length + need + upto_length + 1;
        myhtml_tree_node_t **tmp = (myhtml_tree_node_t **)mycore_realloc(collection->list, sizeof(myhtml_tree_node_t*) * tmp_size);
        
        if(tmp) {
            collection->size = tmp_size;
            collection->list = tmp;
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
        mycore_free(collection->list);
    
    mycore_free(collection);
    
    return NULL;
}

/* queue */
mystatus_t myhtml_queue_add(myhtml_tree_t *tree, size_t begin, myhtml_token_node_t* token)
{
    // TODO: need refactoring this code
    // too many conditions
    mythread_queue_node_t *qnode = tree->current_qnode;
    
    if(tree->parse_flags & MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN) {
        if(token && token->tag_id == MyHTML_TAG__TEXT && token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
        {
            myhtml_token_node_clean(token);
            token->raw_begin = token->element_begin = (tree->global_offset + begin);
            
            return MyHTML_STATUS_OK;
        }
    }
    
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    
    if(tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) {
        if(qnode && token) {
            qnode->args = token;
            
            myhtml_parser_worker(0, qnode);
            myhtml_parser_stream(0, qnode);
        }
        
        tree->current_qnode = mythread_queue_node_malloc_limit(tree->myhtml->thread_stream, tree->queue, 4, NULL);
    }
    else {
        if(qnode)
            qnode->args = token;
        
        tree->current_qnode = mythread_queue_node_malloc_round(tree->myhtml->thread_stream, tree->queue_entry);
        
        /* we have a clean queue list */
        if(tree->queue_entry->queue->nodes_length == 0) {
            mythread_queue_list_entry_make_batch(tree->myhtml->thread_batch, tree->queue_entry);
            mythread_queue_list_entry_make_stream(tree->myhtml->thread_stream, tree->queue_entry);
        }
    }
    
#else
    
    if(qnode && token) {
        qnode->args = token;
        
        myhtml_parser_worker(0, qnode);
        myhtml_parser_stream(0, qnode);
    }
    
    tree->current_qnode = mythread_queue_node_malloc_limit(tree->myhtml->thread_stream, tree->queue, 4, NULL);
    
#endif /* MyCORE_BUILD_WITHOUT_THREADS */
    
    if(tree->current_qnode == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    tree->current_qnode->context = tree;
    tree->current_qnode->prev = qnode;
    
    if(qnode && token)
        myhtml_tokenizer_calc_current_namespace(tree, token);
    
    tree->current_token_node = myhtml_token_node_create(tree->token, tree->token->mcasync_token_id);
    if(tree->current_token_node == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    tree->current_token_node->raw_begin = tree->current_token_node->element_begin = (tree->global_offset + begin);
    
    return MyHTML_STATUS_OK;
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
    
    return node->tag_id == tag_id && node->ns == MyHTML_NAMESPACE_HTML;
}

/* version */
myhtml_version_t myhtml_version(void)
{
    return (myhtml_version_t){MyHTML_VERSION_MAJOR, MyHTML_VERSION_MINOR, MyHTML_VERSION_PATCH};
}





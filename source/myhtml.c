//
//  main.c
//  myhtml
//
//  Created by Alexander Borisov on 24.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml.h"

myhtml_t * myhtml_create(void)
{
    return (myhtml_t*)mymalloc(sizeof(myhtml_t));
}

myhtml_status_t myhtml_init(myhtml_t* myhtml, enum myhtml_options opt, size_t thread_count, size_t queue_size)
{
    myhtml_status_t status;
    
    myhtml->tags = mytags_create();
    if(myhtml->tags == NULL) {
        myhtml->parse_state_func = NULL;
        myhtml->insertion_func = NULL;
        myhtml->thread = NULL;
        
        return MyHTML_STATUS_MYTAGS_ERROR_MEMORY_ALLOCATION;
    }
    
    status = mytags_init(myhtml->tags);
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
    
    // set ref
    myhtml->queue = myhtml->thread->queue;
    
    myhtml_clean(myhtml);
    
    return status;
}

void myhtml_clean(myhtml_t* myhtml)
{
    mythread_queue_clean(myhtml->queue);
    mythread_clean(myhtml->thread);
}

myhtml_t* myhtml_destroy(myhtml_t* myhtml)
{
    if(myhtml == NULL)
        return NULL;
    
    mythread_destroy(myhtml->thread, mytrue);
    myhtml_tokenizer_state_destroy(myhtml);
    
    myhtml->tags     = mytags_destroy(myhtml->tags);
    myhtml->queue    = NULL;
    
    if(myhtml->insertion_func)
        free(myhtml->insertion_func);
    
    free(myhtml);
    
    return NULL;
}

void myhtml_parse(myhtml_tree_t* tree, const char* html, size_t html_size)
{
    myhtml_tree_clean(tree);
    mythread_queue_clean(tree->myhtml->queue);
    mythread_clean(tree->myhtml->thread);
    
    myhtml_tokenizer_begin(tree, html, html_size);
    myhtml_tokenizer_end(tree, html, html_size);
}

void myhtml_parse_fragment(myhtml_tree_t* tree, const char* html, size_t html_size)
{
    myhtml_tree_clean(tree);
    
    myhtml_tokenizer_fragment_init(tree, MyTAGS_TAG_DIV, MyHTML_NAMESPACE_HTML);
    
    myhtml_tokenizer_begin(tree, html, html_size);
    myhtml_tokenizer_end(tree, html, html_size);
}

void myhtml_parse_single(myhtml_tree_t* tree, const char* html, size_t html_size)
{
    myhtml_tree_clean(tree);
    
    myhtml_tokenizer_begin(tree, html, html_size);
    myhtml_tokenizer_end(tree, html, html_size);
}

/*
 * Nodes
 */
myhtml_collection_t * myhtml_get_nodes_by_tag_id(myhtml_tree_t* tree, myhtml_collection_t *collection, mytags_ctx_index_t tag_id, myhtml_status_t *status)
{
    mytags_index_tag_t *index_tag = mytags_index_tag_get(tree->indexes->tags, tag_id);
    mytags_index_tag_node_t *index_node = mytags_index_tag_get_first(tree->indexes->tags, tag_id);
    
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

enum myhtml_namespace myhtml_node_namespace(myhtml_tree_node_t *node)
{
    return node->namespace;
}

myhtml_tag_id_t myhtml_node_tag_id(myhtml_tree_node_t *node)
{
    return node->tag_idx;
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
    return attr->namespace;
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

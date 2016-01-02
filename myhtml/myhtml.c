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
    myhtml_t* myhtml = (myhtml_t*)mymalloc(sizeof(myhtml_t));
    //if(myhtml == NULL)
    // check this
    return myhtml;
}

void myhtml_init(myhtml_t* myhtml, enum myhtml_options opt, size_t thread_count, size_t queue_size, size_t token_nodes_size)
{
    myhtml->tags = mytags_init();
    
    myhtml_tokenizer_state_init(myhtml);
    myhtml_rules_init(myhtml);
    
    myhtml->thread = mythread_create();
    
    myhtml_status_t status;
    
    switch (opt) {
        case MyHTML_OPTIONS_PARSE_MODE_SINGLE:
            status = mythread_init(myhtml->thread, "lastmac", 0);
            break;
            
        case MyHTML_OPTIONS_PARSE_MODE_ALL_IN_ONE:
            status = mythread_init(myhtml->thread, "lastmac", 1);
            myhread_create_stream(myhtml->thread, myhtml_parser_worker_index_stream, &status);
            break;
            
        case MyHTML_OPTIONS_PARSE_MODE_WORKER_TREE:
            status = mythread_init(myhtml->thread, "lastmac", 2);
            
            myhread_create_stream(myhtml->thread, myhtml_parser_index, &status);
            myhread_create_stream(myhtml->thread, myhtml_parser_worker_stream, &status);
            break;
            
        case MyHTML_OPTIONS_PARSE_MODE_WORKER_INDEX:
            status = mythread_init(myhtml->thread, "lastmac", 2);
            
            myhread_create_stream(myhtml->thread, myhtml_parser_worker_index, &status);
            myhread_create_stream(myhtml->thread, myhtml_parser_stream, &status);
            break;
            
        case MyHTML_OPTIONS_PARSE_MODE_TREE_INDEX:
            if(thread_count == 0)
                thread_count = 1;
            
            status = mythread_init(myhtml->thread, "lastmac", (thread_count + 1));
            
            myhread_create_stream(myhtml->thread, myhtml_parser_stream_index, &status);
            myhread_create_batch(myhtml->thread, myhtml_parser_worker, &status, thread_count);
            break;
            
        default:
            // default MyHTML_OPTIONS_PARSE_MODE_SEPARATELY
            if(thread_count == 0)
                thread_count = 1;
            
            status = mythread_init(myhtml->thread, "lastmac", (thread_count + 2));
            
            myhread_create_stream(myhtml->thread, myhtml_parser_index, &status);
            myhread_create_stream(myhtml->thread, myhtml_parser_stream, &status);
            myhread_create_batch(myhtml->thread, myhtml_parser_worker, &status, thread_count);
            break;
    }
    
    // set ref
    myhtml->queue = myhtml->thread->queue;
    
    myhtml_clean(myhtml);
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

myhtml_tree_node_t ** myhtml_get_elements_by_tag_id(myhtml_tree_t* tree, mytags_ctx_index_t tag_id, size_t* return_length)
{
    mytags_index_tag_t *index_tag = mytags_index_tag_get(tree->indexes->tags, tag_id);
    mytags_index_tag_node_t *index_node = mytags_index_tag_get_first(tree->indexes->tags, tag_id);
    
    if(return_length)
        *return_length = index_tag->count;
    
    if(index_tag->count == 0)
        return NULL;
    
    myhtml_tree_node_t **list = (myhtml_tree_node_t**)malloc(sizeof(myhtml_tree_node_t*) * (index_tag->count + 1));
    
    size_t idx = 0;
    while (index_node)
    {
        list[idx] = index_node->node;
        idx++;
        
        index_node = index_node->next;
    }
    
    list[idx] = NULL;
    
    return list;
}

myhtml_tree_node_t ** myhtml_get_elements_by_name(myhtml_tree_t* tree, const char* html, size_t length, size_t* return_length)
{
    mctree_index_t tag_ctx_idx = mctree_search_lowercase(tree->myhtml->tags->tree, html, length);
    
    return myhtml_get_elements_by_tag_id(tree, tag_ctx_idx, return_length);
}

void myhtml_destroy_node_list(myhtml_tree_node_t **node_list)
{
    if(node_list)
        free(node_list);
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

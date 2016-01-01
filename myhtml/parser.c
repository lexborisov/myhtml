//
//  myhtml_parser.c
//  myhtml
//
//  Created by Alexander Borisov on 17.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "parser.h"

void myhtml_parser_index(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
}

void myhtml_parser_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
    while(myhtml_rules_tree_dispatcher(qnode->tree, qnode->token)){}
}

void myhtml_parser_worker(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
    myhtml_token_node_t* token = qnode->token;
    
    size_t mchar_node_id = qnode->tree->async_args[thread_id].mchar_node_id;
    
    if(token->tag_ctx_idx == MyTAGS_TAG__TEXT ||
       token->tag_ctx_idx == MyTAGS_TAG__COMMENT)
    {
        myhtml_string_init(&token->my_str_tm, qnode->tree->mchar, mchar_node_id, (qnode->length + 32));
        
        myhtml_string_t* string = &token->my_str_tm;
        
        token->begin      = string->length;
        token->length     = qnode->length;
        token->attr_first = NULL;
        token->attr_last  = NULL;
        
        myhtml_string_append_with_null(string,
                                       &qnode->text[qnode->begin],
                                       qnode->length);
    }
    else if(token->attr_first)
    {
        token->my_str_tm.data     = NULL;
        token->my_str_tm.mchar    = NULL;
        token->my_str_tm.node_idx = 0;
        token->my_str_tm.length   = 0;
        token->my_str_tm.size     = 0;
        
        token->begin  = 0;
        token->length = 0;
        
        myhtml_token_attr_t* attr = token->attr_first;
        
        while(attr)
        {
            myhtml_string_init(&attr->entry, qnode->tree->mchar, mchar_node_id, (attr->name_length + attr->value_length + 32));
            
            if(attr->name_length)
            {
                size_t begin = attr->name_begin;
                attr->name_begin = attr->entry.length;
                
                size_t len = attr->name_length;
                
                myhtml_string_append_lowercase_with_null(&attr->entry,
                                                         &qnode->text[begin],
                                                         len);
            }
            
            if(attr->value_length)
            {
                size_t begin = attr->value_begin;
                attr->value_begin = attr->entry.length;
                
                myhtml_string_append_with_null(&attr->entry,
                                               &qnode->text[begin],
                                               attr->value_length);
            }
            
            attr = attr->next;
        }
    }
    else {
        token->begin      = 0;
        token->length     = 0;
        token->attr_first = NULL;
        token->attr_last  = NULL;
        
        token->my_str_tm.data     = NULL;
        token->my_str_tm.mchar    = NULL;
        token->my_str_tm.node_idx = 0;
        token->my_str_tm.length   = 0;
        token->my_str_tm.size     = 0;
    }
    
    token->type |= MyHTML_TOKEN_TYPE_DONE;
}

void myhtml_parser_worker_index_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
    myhtml_parser_worker(thread_id, qnode);
    myhtml_parser_index(thread_id, qnode);
    myhtml_parser_stream(thread_id, qnode);
}

void myhtml_parser_worker_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
    myhtml_parser_worker(thread_id, qnode);
    myhtml_parser_stream(thread_id, qnode);
}

void myhtml_parser_worker_index(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
    myhtml_parser_worker(thread_id, qnode);
    myhtml_parser_index(thread_id, qnode);
}

void myhtml_parser_stream_index(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
    myhtml_parser_stream(thread_id, qnode);
    myhtml_parser_index(thread_id, qnode);
}




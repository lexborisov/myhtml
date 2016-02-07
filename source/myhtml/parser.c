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

#include "parser.h"

void myhtml_parser_index(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
    // TODO: Create index for attributes
}

void myhtml_parser_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
    while(myhtml_rules_tree_dispatcher(qnode->tree, qnode->token)){}
}

myhtml_incoming_buf_t * myhtml_parser_find_first_buf(myhtml_tree_t *tree, size_t begin)
{
    myhtml_incoming_buf_t *inc_buf = tree->incoming_buf_first;
    while (inc_buf && inc_buf->next && inc_buf->next->offset < begin) {
        inc_buf = inc_buf->next;
    }
    
    return inc_buf;
}

size_t myhtml_parser_add_text(myhtml_tree_t *tree, myhtml_string_t* string, const char *text, size_t begin, size_t length)
{
    myhtml_incoming_buf_t *inc_buf = myhtml_parser_find_first_buf(tree, begin);
    
    size_t current_buf_offset = begin - inc_buf->offset;
    size_t save_str_len = string->length;
    
    if((current_buf_offset + length) <= inc_buf->size)
    {
        if(tree->encoding == MyHTML_ENCODING_UTF_8)
            myhtml_string_append(string, &inc_buf->data[current_buf_offset], length);
        else
            myhtml_string_append_with_convert_encoding(string,
                                                       &inc_buf->data[current_buf_offset],
                                                       length, tree->encoding);
        
        return (string->length - save_str_len);
    }
    
    size_t buf_next_offset = inc_buf->size - current_buf_offset;
    
    myhtml_encoding_result_t res;
    myhtml_encoding_result_clean(&res);
    
    if(tree->encoding == MyHTML_ENCODING_UTF_8)
        myhtml_string_append(string, &inc_buf->data[current_buf_offset], buf_next_offset);
    else
        myhtml_string_append_chunk_with_convert_encoding(string, &res, &inc_buf->data[current_buf_offset],
                                                         buf_next_offset, tree->encoding);
    
    length = length - buf_next_offset;
    inc_buf = inc_buf->next;
    
    if(tree->encoding == MyHTML_ENCODING_UTF_8) {
        while (inc_buf && length)
        {
            if(length > inc_buf->size) {
                myhtml_string_append(string, inc_buf->data, inc_buf->size);
                length -= inc_buf->size;
            }
            else {
                myhtml_string_append(string, inc_buf->data, length);
                break;
            }
            
            inc_buf = inc_buf->next;
        }
    }
    else {
        while (inc_buf && length)
        {
            if(length > inc_buf->size) {
                myhtml_string_append_chunk_with_convert_encoding(string, &res, inc_buf->data,
                                                                 inc_buf->size, tree->encoding);
                length -= inc_buf->size;
            }
            else {
                myhtml_string_append_chunk_with_convert_encoding(string, &res, inc_buf->data,
                                                                 length, tree->encoding);
                break;
            }
            
            inc_buf = inc_buf->next;
        }
    }
    
    return (string->length - save_str_len);
}

size_t myhtml_parser_add_text_lowercase(myhtml_tree_t *tree, myhtml_string_t* string, const char *text, size_t begin, size_t length)
{
    myhtml_incoming_buf_t *inc_buf = myhtml_parser_find_first_buf(tree, begin);
    
    myhtml_encoding_result_t str_res;
    myhtml_encoding_result_clean(&str_res);
    
    size_t current_buf_offset = begin - inc_buf->offset;
    size_t save_str_len = string->length;
    
    if((current_buf_offset + length) <= inc_buf->size)
    {
        if(tree->encoding == MyHTML_ENCODING_UTF_8)
            myhtml_string_append_lowercase(string, &inc_buf->data[current_buf_offset], length);
        else
            myhtml_string_append_chunk_lowercase_ascii_with_convert_encoding(string, &str_res, &inc_buf->data[current_buf_offset], length, tree->encoding);
        
        return (string->length - save_str_len);
    }
    
    size_t buf_next_offset = inc_buf->size - current_buf_offset;
    
    if(tree->encoding == MyHTML_ENCODING_UTF_8)
        myhtml_string_append_lowercase(string, &inc_buf->data[current_buf_offset], buf_next_offset);
    else
        myhtml_string_append_chunk_lowercase_ascii_with_convert_encoding(string, &str_res, &inc_buf->data[current_buf_offset], buf_next_offset, tree->encoding);
    
    length = length - buf_next_offset;
    inc_buf = inc_buf->next;
    
    if(tree->encoding == MyHTML_ENCODING_UTF_8) {
        while (inc_buf && length)
        {
            if(length > inc_buf->size) {
                myhtml_string_append_lowercase(string, inc_buf->data, inc_buf->size);
                length -= inc_buf->size;
            }
            else {
                myhtml_string_append_lowercase(string, inc_buf->data, length);
                break;
            }
            
            inc_buf = inc_buf->next;
        }
    }
    else {
        while (inc_buf && length)
        {
            if(length > inc_buf->size) {
                myhtml_string_append_chunk_lowercase_ascii_with_convert_encoding(string, &str_res, inc_buf->data, inc_buf->size, tree->encoding);
                length -= inc_buf->size;
            }
            else {
                myhtml_string_append_chunk_lowercase_ascii_with_convert_encoding(string, &str_res, inc_buf->data, length, tree->encoding);
                break;
            }
            
            inc_buf = inc_buf->next;
        }
    }
    
    return (string->length - save_str_len);
}

size_t myhtml_parser_add_text_with_charef(myhtml_tree_t *tree, myhtml_string_t* string, const char *text, size_t begin, size_t length)
{
    myhtml_incoming_buf_t *inc_buf = myhtml_parser_find_first_buf(tree, begin);
    
    myhtml_string_char_ref_chunk_t str_chunk = {0, 0, 0, NULL, tree->encoding};
    myhtml_encoding_result_clean(&str_chunk.res);
    
    size_t current_buf_offset = begin - inc_buf->offset;
    size_t save_str_len = string->length;
    
    if((current_buf_offset + length) <= inc_buf->size)
    {
        myhtml_string_append_charef(&str_chunk, string, &inc_buf->data[current_buf_offset], length);
        myhtml_string_append_charef_end(&str_chunk, string);
        return (string->length - save_str_len);
    }
    
    size_t buf_next_offset = inc_buf->size - current_buf_offset;
    
    myhtml_string_append_charef(&str_chunk, string, &inc_buf->data[current_buf_offset], buf_next_offset);
    
    length = length - buf_next_offset;
    inc_buf = inc_buf->next;
    
    while (inc_buf && length)
    {
        if(length > inc_buf->size) {
            myhtml_string_append_charef(&str_chunk, string, inc_buf->data, inc_buf->size);
            length -= inc_buf->size;
        }
        else {
            myhtml_string_append_charef(&str_chunk, string, inc_buf->data, length);
            break;
        }
        
        inc_buf = inc_buf->next;
    }
    
    myhtml_string_append_charef_end(&str_chunk, string);
    return (string->length - save_str_len);
}

void myhtml_parser_worker(mythread_id_t thread_id, mythread_queue_node_t *qnode)
{
    myhtml_token_node_t* token = qnode->token;
    
    size_t mchar_node_id = qnode->tree->async_args[thread_id].mchar_node_id;
    
    if(token->tag_ctx_idx == MyHTML_TAG__TEXT ||
       token->tag_ctx_idx == MyHTML_TAG__COMMENT)
    {
        myhtml_string_init(qnode->tree->mchar, mchar_node_id, &token->my_str_tm, (qnode->length + 2));
        
        token->begin      = token->my_str_tm.length;
        token->length     = qnode->length;
        token->attr_first = NULL;
        token->attr_last  = NULL;
        
        if(token->type & MyHTML_TOKEN_TYPE_RCDATA ||
           token->type & MyHTML_TOKEN_TYPE_CDATA ||
           token->type & MyHTML_TOKEN_TYPE_DATA)
        {
            token->length = myhtml_parser_add_text_with_charef(qnode->tree, &token->my_str_tm, qnode->text, qnode->begin, qnode->length);
        }
        else
            token->length = myhtml_parser_add_text(qnode->tree, &token->my_str_tm, qnode->text, qnode->begin, qnode->length);
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
            myhtml_string_init(qnode->tree->mchar, mchar_node_id, &attr->entry, (attr->name_length + attr->value_length + 8));
            
            if(attr->name_length)
            {
                size_t begin = attr->name_begin;
                attr->name_begin = attr->entry.length;
                
                attr->name_length = myhtml_parser_add_text_lowercase(qnode->tree, &attr->entry, qnode->text, begin, attr->name_length);
            }
            
            if(attr->value_length)
            {
                size_t begin = attr->value_begin;
                attr->value_begin = attr->entry.length;
                
                attr->value_length = myhtml_parser_add_text_with_charef(qnode->tree, &attr->entry, qnode->text, begin, attr->value_length);
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


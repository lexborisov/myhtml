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

#include "myhtml/parser.h"

void myhtml_parser_stream(mythread_id_t thread_id, void* ctx)
{
    mythread_queue_node_t *qnode = (mythread_queue_node_t*)ctx;
    
    if((((myhtml_tree_t*)(qnode->context))->parse_flags & MyHTML_TREE_PARSE_FLAGS_WITHOUT_BUILD_TREE) == 0) {
        while(myhtml_rules_tree_dispatcher(qnode->context, qnode->args)){}
    }
}

size_t myhtml_parser_token_data_to_string_lowercase(myhtml_tree_t *tree, mycore_string_t* str, myhtml_data_process_entry_t* proc_entry, size_t begin, size_t length)
{
    mycore_incoming_buffer_t *buffer = mycore_incoming_buffer_find_by_position(tree->incoming_buf_first, begin);
    size_t relative_begin = begin - buffer->offset;
    
    // if token data length in one buffer then print them all at once
    if((relative_begin + length) <= buffer->size) {
        if(tree->encoding == MyENCODING_UTF_8)
            myhtml_string_append_lowercase_with_preprocessing(str, &buffer->data[relative_begin], length, proc_entry->emit_null_char);
        else
            myhtml_string_append_lowercase_chunk_with_convert_encoding_with_preprocessing(str, &proc_entry->res,
                                                                                          &buffer->data[relative_begin], length,
                                                                                          proc_entry->encoding, proc_entry->emit_null_char);
        
        return str->length;
    }
    
    size_t save_position = 0;
    // if the data are spread across multiple buffers that join them
    while(buffer) {
        if((relative_begin + length) > buffer->size)
        {
            size_t relative_end = (buffer->size - relative_begin);
            length -= relative_end;
            
            size_t tmp_offset = myhtml_string_before_append_any_preprocessing(str, &buffer->data[relative_begin], relative_end, save_position);
            
            if(relative_end > 0) {
                if(tree->encoding == MyENCODING_UTF_8)
                    save_position = myhtml_string_append_lowercase_with_preprocessing(str, &buffer->data[(relative_begin + tmp_offset)], (relative_end - tmp_offset), proc_entry->emit_null_char);
                else
                    save_position = myhtml_string_append_lowercase_chunk_with_convert_encoding_with_preprocessing(str, &proc_entry->res,
                                                                                                                  &buffer->data[(relative_begin + tmp_offset)], (relative_end - tmp_offset),
                                                                                                                  proc_entry->encoding, proc_entry->emit_null_char);
            }
            
            relative_begin = 0;
            buffer         = buffer->next;
        }
        else {
            size_t tmp_offset = myhtml_string_before_append_any_preprocessing(str, &buffer->data[relative_begin], length, save_position);
            
            if(length > 0) {
                if(tree->encoding == MyENCODING_UTF_8)
                    myhtml_string_append_lowercase_with_preprocessing(str, &buffer->data[(relative_begin + tmp_offset)], (length - tmp_offset), proc_entry->emit_null_char);
                else
                    myhtml_string_append_lowercase_chunk_with_convert_encoding_with_preprocessing(str, &proc_entry->res,
                                                                                                                  &buffer->data[(relative_begin + tmp_offset)], (length - tmp_offset),
                                                                                                                  proc_entry->encoding, proc_entry->emit_null_char);
            }
            
            break;
        }
    }
    
    return str->length;
}

size_t myhtml_parser_token_data_to_string(myhtml_tree_t *tree, mycore_string_t* str, myhtml_data_process_entry_t* proc_entry, size_t begin, size_t length)
{
    mycore_incoming_buffer_t *buffer = mycore_incoming_buffer_find_by_position(tree->incoming_buf_first, begin);
    size_t relative_begin = begin - buffer->offset;
    
    // if token data length in one buffer then print them all at once
    if((relative_begin + length) <= buffer->size) {
        if(tree->encoding == MyENCODING_UTF_8)
            myhtml_string_append_with_preprocessing(str, &buffer->data[relative_begin], length, proc_entry->emit_null_char);
        else
            myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &proc_entry->res,
                                                                                &buffer->data[relative_begin], length,
                                                                                proc_entry->encoding, proc_entry->emit_null_char);
        
        return str->length;
    }
    
    size_t save_position = 0;
    // if the data are spread across multiple buffers that join them
    while(buffer) {
        if((relative_begin + length) > buffer->size)
        {
            size_t relative_end = (buffer->size - relative_begin);
            length -= relative_end;
            
            size_t tmp_offset = myhtml_string_before_append_any_preprocessing(str, &buffer->data[relative_begin], relative_end, save_position);
            
            if(relative_end > 0) {
                if(tree->encoding == MyENCODING_UTF_8)
                    save_position = myhtml_string_append_with_preprocessing(str, &buffer->data[(relative_begin + tmp_offset)], (relative_end - tmp_offset), proc_entry->emit_null_char);
                else
                    save_position = myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &proc_entry->res,
                                                                                                        &buffer->data[(relative_begin + tmp_offset)],
                                                                                                        (relative_end - tmp_offset),
                                                                                                        proc_entry->encoding, proc_entry->emit_null_char);
            }
            
            relative_begin = 0;
            buffer         = buffer->next;
        }
        else {
            size_t tmp_offset = myhtml_string_before_append_any_preprocessing(str, &buffer->data[relative_begin], length, save_position);
            
            if(length > 0) {
                if(tree->encoding == MyENCODING_UTF_8)
                    myhtml_string_append_with_preprocessing(str, &buffer->data[(relative_begin + tmp_offset)], (length - tmp_offset), proc_entry->emit_null_char);
                else
                    myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &proc_entry->res,
                                                                                        &buffer->data[(relative_begin + tmp_offset)], (length - tmp_offset),
                                                                                        proc_entry->encoding, proc_entry->emit_null_char);
            }
            
            break;
        }
    }
    
    return str->length;
}

size_t myhtml_parser_token_data_to_string_charef(myhtml_tree_t *tree, mycore_string_t* str, myhtml_data_process_entry_t* proc_entry, size_t begin, size_t length)
{
    mycore_incoming_buffer_t *buffer = mycore_incoming_buffer_find_by_position(tree->incoming_buf_first, begin);
    size_t relative_begin = begin - buffer->offset;
    
    // if token data length in one buffer then print them all at once
    if((relative_begin + length) <= buffer->size) {
        myhtml_data_process(proc_entry, str, &buffer->data[relative_begin], length);
        myhtml_data_process_end(proc_entry, str);
        
        return str->length;
    }
    
    // if the data are spread across multiple buffers that join them
    while(buffer) {
        if((relative_begin + length) > buffer->size)
        {
            size_t relative_end = (buffer->size - relative_begin);
            length -= relative_end;
            
            myhtml_data_process(proc_entry, str, &buffer->data[relative_begin], relative_end);
            
            relative_begin = 0;
            buffer         = buffer->next;
        }
        else {
            myhtml_data_process(proc_entry, str, &buffer->data[relative_begin], length);
            break;
        }
    }
    
    myhtml_data_process_end(proc_entry, str);
    
    return str->length;
}

void myhtml_parser_worker(mythread_id_t thread_id, void* ctx)
{
    mythread_queue_node_t *qnode = (mythread_queue_node_t*)ctx;
    
    myhtml_tree_t* tree = qnode->context;
    myhtml_token_node_t* token = qnode->args;
    
    /* 
     * Tree can not be built without tokens
     *
     * MyHTML_TREE_PARSE_FLAGS_WITHOUT_PROCESS_TOKEN == 3
     * MyHTML_TREE_PARSE_FLAGS_WITHOUT_BUILD_TREE    == 1
     *
     * MyHTML_TREE_PARSE_FLAGS_WITHOUT_PROCESS_TOKEN include MyHTML_TREE_PARSE_FLAGS_WITHOUT_BUILD_TREE 
     *
     * if set only MyHTML_TREE_PARSE_FLAGS_WITHOUT_BUILD_TREE and check only for MyHTML_TREE_PARSE_FLAGS_WITHOUT_PROCESS_TOKEN
     *   return true
     * we need check both, 1 and 2
     */
    if((tree->parse_flags & MyHTML_TREE_PARSE_FLAGS_WITHOUT_PROCESS_TOKEN) &&
       (tree->parse_flags & 2))
    {
        if(tree->callback_before_token)
            tree->callback_before_token_ctx = tree->callback_before_token(tree, token, tree->callback_before_token_ctx);
        
        token->type |= MyHTML_TOKEN_TYPE_DONE;
        
        if(tree->callback_after_token)
            tree->callback_after_token_ctx = tree->callback_after_token(tree, token, tree->callback_after_token_ctx);
        
        return;
    }
    
    size_t mchar_node_id;
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    if(tree->myhtml->thread_batch)
        mchar_node_id = tree->async_args[(thread_id + tree->myhtml->thread_batch->id_increase)].mchar_node_id;
    else
#endif
        mchar_node_id = tree->async_args[thread_id].mchar_node_id;
    
    if(tree->callback_before_token)
        tree->callback_before_token_ctx = tree->callback_before_token(tree, token, tree->callback_before_token_ctx);
    
    if(token->tag_id == MyHTML_TAG__TEXT ||
       token->tag_id == MyHTML_TAG__COMMENT)
    {
        mycore_string_init(tree->mchar, mchar_node_id, &token->str, (token->raw_length + 1));
        
        token->attr_first = NULL;
        token->attr_last  = NULL;
        
        myhtml_data_process_entry_t proc_entry;
        myhtml_data_process_entry_clean(&proc_entry);
        
        proc_entry.encoding = tree->encoding;
        
        if(token->type & MyHTML_TOKEN_TYPE_DATA) {
            proc_entry.emit_null_char = true;
            
            myhtml_parser_token_data_to_string_charef(tree, &token->str, &proc_entry, token->raw_begin, token->raw_length);
        }
        else if(token->type & MyHTML_TOKEN_TYPE_RCDATA || token->type & MyHTML_TOKEN_TYPE_CDATA) {
            myhtml_parser_token_data_to_string_charef(tree, &token->str, &proc_entry, token->raw_begin, token->raw_length);
        }
        else
            myhtml_parser_token_data_to_string(tree, &token->str, &proc_entry, token->raw_begin, token->raw_length);
    }
    else if(token->attr_first)
    {
        mycore_string_clean_all(&token->str);
        
        myhtml_token_attr_t* attr = token->attr_first;
        myhtml_data_process_entry_t proc_entry;
        
        while(attr)
        {
            if(attr->raw_key_length) {
                myhtml_data_process_entry_clean(&proc_entry);
                proc_entry.encoding = tree->encoding;
                
                mycore_string_init(tree->mchar, mchar_node_id, &attr->key, (attr->raw_key_length + 1));
                myhtml_parser_token_data_to_string_lowercase(tree, &attr->key, &proc_entry, attr->raw_key_begin, attr->raw_key_length);
            }
            else
                mycore_string_clean_all(&attr->key);
            
            if(attr->raw_value_length) {
                myhtml_data_process_entry_clean(&proc_entry);
                proc_entry.encoding = tree->encoding;
                proc_entry.is_attributes = true;
                
                mycore_string_init(tree->mchar, mchar_node_id, &attr->value, (attr->raw_value_length + 1));
                myhtml_parser_token_data_to_string_charef(tree, &attr->value, &proc_entry, attr->raw_value_begin, attr->raw_value_length);
            }
            else
                mycore_string_clean_all(&attr->value);
            
            attr = attr->next;
        }
    }
    else {
        token->attr_first = NULL;
        token->attr_last  = NULL;
        
        mycore_string_clean_all(&token->str);
    }
    
    token->type |= MyHTML_TOKEN_TYPE_DONE;
    
    if(tree->callback_after_token)
        tree->callback_after_token_ctx = tree->callback_after_token(tree, token, tree->callback_after_token_ctx);
}

void myhtml_parser_worker_stream(mythread_id_t thread_id, void* ctx)
{
    mythread_queue_node_t *qnode = (mythread_queue_node_t*)ctx;
    
    myhtml_parser_worker(thread_id, qnode);
    myhtml_parser_stream(thread_id, qnode);
}



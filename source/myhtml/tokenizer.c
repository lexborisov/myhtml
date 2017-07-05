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

#include "myhtml/tokenizer.h"
#include "mycore/utils/resources.h"

mystatus_t myhtml_tokenizer_set_first_settings(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    tree->current_qnode = mythread_queue_get_current_node(tree->queue);
    mythread_queue_node_clean(tree->current_qnode);
    
    tree->current_qnode->context = tree;
    tree->current_token_node = myhtml_token_node_create(tree->token, tree->token->mcasync_token_id);
    
    if(tree->current_token_node == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    tree->incoming_buf_first = tree->incoming_buf;
    
    return MyHTML_STATUS_OK;
}

mystatus_t myhtml_tokenizer_begin(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    return myhtml_tokenizer_chunk(tree, html, html_length);
}

mystatus_t myhtml_tokenizer_chunk_process(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    myhtml_t* myhtml = tree->myhtml;
    myhtml_tokenizer_state_f* state_f = myhtml->parse_state_func;
    
    // add for a chunk
    tree->incoming_buf = mycore_incoming_buffer_add(tree->incoming_buf, tree->mcobject_incoming_buf, html, html_length);
    
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    
    if(myhtml->opt & MyHTML_OPTIONS_PARSE_MODE_SINGLE)
        tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
    {
        if(tree->queue_entry == NULL) {
            mystatus_t status = MyHTML_STATUS_OK;
            tree->queue_entry = mythread_queue_list_entry_push(myhtml->thread_list, 2,
                                                               myhtml->thread_stream->context, tree->queue,
                                                               myhtml->thread_total, &status);
            
            if(status)
                return status;
        }
        
        myhtml_tokenizer_post(tree);
    }
    
#else
    
    tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
#endif
    
    if(tree->current_qnode == NULL) {
        mystatus_t status = myhtml_tokenizer_set_first_settings(tree, html, html_length);
        if(status)
            return status;
    }
    
    size_t offset = 0;
    
    while (offset < html_length) {
        offset = state_f[tree->state](tree, tree->current_token_node, html, offset, html_length);
    }
    
    tree->global_offset += html_length;
    
    return MyHTML_STATUS_OK;
}

mystatus_t myhtml_tokenizer_chunk(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    if(tree->encoding_usereq == MyENCODING_UTF_16LE ||
       tree->encoding_usereq == MyENCODING_UTF_16BE)
    {
        return myhtml_tokenizer_chunk_with_stream_buffer(tree, html, html_length);
    }
    
    return myhtml_tokenizer_chunk_process(tree, html, html_length);
}

mystatus_t myhtml_tokenizer_chunk_with_stream_buffer(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    unsigned const char* u_html = (unsigned const char*)html;
    const myencoding_custom_f func = myencoding_get_function_by_id(tree->encoding);
    
    if(tree->stream_buffer == NULL) {
        tree->stream_buffer = myhtml_stream_buffer_create();
        
        if(tree->stream_buffer == NULL)
            return MyHTML_STATUS_STREAM_BUFFER_ERROR_CREATE;
        
        mystatus_t status = myhtml_stream_buffer_init(tree->stream_buffer, 1024);
        
        if(status)
            return status;
        
        if(myhtml_stream_buffer_add_entry(tree->stream_buffer, (4096 * 4)) == NULL)
            return MyHTML_STATUS_STREAM_BUFFER_ERROR_ADD_ENTRY;
    }
    
    myhtml_stream_buffer_t *stream_buffer = tree->stream_buffer;
    myhtml_stream_buffer_entry_t *stream_entry = myhtml_stream_buffer_current_entry(stream_buffer);
    
    size_t temp_curr_pos = stream_entry->length;
    
    for (size_t i = 0; i < html_length; i++)
    {
        if(func(u_html[i], &stream_buffer->res) == MyENCODING_STATUS_OK)
        {
            if((stream_entry->length + 4) >= stream_entry->size)
            {
                tree->encoding = MyENCODING_UTF_8;
                myhtml_tokenizer_chunk_process(tree, &stream_entry->data[temp_curr_pos], (stream_entry->length - temp_curr_pos));
                
                stream_entry = myhtml_stream_buffer_add_entry(stream_buffer, (4096 * 4));
                
                if(stream_entry == NULL)
                    return MyHTML_STATUS_STREAM_BUFFER_ERROR_ADD_ENTRY;
                
                temp_curr_pos = stream_entry->length;
            }
            
            stream_entry->length += myencoding_codepoint_to_ascii_utf_8(stream_buffer->res.result, &stream_entry->data[ stream_entry->length ]);
        }
    }
    
    if((stream_entry->length - temp_curr_pos)) {
        tree->encoding = MyENCODING_UTF_8;
        myhtml_tokenizer_chunk_process(tree, &stream_entry->data[temp_curr_pos], (stream_entry->length - temp_curr_pos));
    }
    
    return MyHTML_STATUS_OK;
}

mystatus_t myhtml_tokenizer_end(myhtml_tree_t* tree)
{
    if(tree->incoming_buf)
    {
        tree->global_offset -= tree->incoming_buf->size;
        
        tree->myhtml->parse_state_func[(tree->state + MyHTML_TOKENIZER_STATE_LAST_ENTRY)]
        (tree, tree->current_token_node, tree->incoming_buf->data, tree->incoming_buf->size, tree->incoming_buf->size);
    }
    
    tree->current_token_node->tag_id = MyHTML_TAG__END_OF_FILE;
    
    if(myhtml_queue_add(tree, 0, tree->current_token_node) != MyHTML_STATUS_OK) {
        tree->tokenizer_status = MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    }
    
    mystatus_t status = tree->tokenizer_status;
    
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    
    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
    {
        mythread_queue_list_entry_wait_for_done(tree->myhtml->thread_stream, tree->queue_entry);
        
        tree->queue_entry = mythread_queue_list_entry_delete(tree->myhtml->thread_list, 2,
                                                             tree->myhtml->thread_stream->context,
                                                             tree->queue_entry, false);
        
        /* Further, any work with tree... */
        if(mythread_queue_list_get_count(tree->myhtml->thread_stream->context) == 0)
            myhtml_tokenizer_pause(tree);
        
        if(status == MyHTML_STATUS_OK)
            status = mythread_check_status(tree->myhtml->thread_stream);
    }
    
#endif
    
    tree->flags |= MyHTML_TREE_FLAGS_PARSE_END;
    
    return status;
}

myhtml_tree_node_t * myhtml_tokenizer_fragment_init(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, enum myhtml_namespace ns)
{
    // step 3
    tree->fragment = myhtml_tree_node_create(tree);
    tree->fragment->ns = ns;
    tree->fragment->tag_id = tag_idx;
    
    // step 4, is already done
    if(ns == MyHTML_NAMESPACE_HTML) {
        if(tag_idx == MyHTML_TAG_NOSCRIPT) {
            if(tree->flags & MyHTML_TREE_FLAGS_SCRIPT) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            }
        }
        else {
            const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, tag_idx);
            myhtml_tokenizer_state_set(tree) = tag_ctx->data_parser;
        }
    }
    
    tree->fragment->token = myhtml_token_node_create(tree->token, tree->token->mcasync_token_id);
    
    if(tree->fragment->token == NULL)
        return NULL;
    
    myhtml_token_set_done(tree->fragment->token);
    tree->token_namespace = tree->fragment->token;
    
    // step 5-7
    myhtml_tree_node_t* root = myhtml_tree_node_insert_root(tree, NULL, MyHTML_NAMESPACE_HTML);
    
    if(tag_idx == MyHTML_TAG_TEMPLATE)
        myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_TEMPLATE);
    
    myhtml_tree_reset_insertion_mode_appropriately(tree);
    
    return root;
}

void myhtml_tokenizer_wait(myhtml_tree_t* tree)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    if(tree->myhtml->thread_stream)
        mythread_queue_list_entry_wait_for_done(tree->myhtml->thread_stream, tree->queue_entry);
#endif
}

void myhtml_tokenizer_post(myhtml_tree_t* tree)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    if(tree->myhtml->thread_stream)
        mythread_resume(tree->myhtml->thread_stream, MyTHREAD_OPT_UNDEF);
    
    if(tree->myhtml->thread_batch)
        mythread_resume(tree->myhtml->thread_batch, MyTHREAD_OPT_UNDEF);
#endif
}

void myhtml_tokenizer_pause(myhtml_tree_t* tree)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    if(tree->myhtml->thread_stream)
        mythread_stop(tree->myhtml->thread_stream);
    
    if(tree->myhtml->thread_batch)
        mythread_stop(tree->myhtml->thread_batch);
#endif
}

void myhtml_tokenizer_calc_current_namespace(myhtml_tree_t* tree, myhtml_token_node_t* token_node)
{
    if((tree->parse_flags & MyHTML_TREE_PARSE_FLAGS_WITHOUT_BUILD_TREE) == 0) {
        if(tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE)
        {
            myhtml_tokenizer_state_set(tree) = tree->state_of_builder;
        }
        else {
            if(token_node->tag_id == MyHTML_TAG_MATH ||
               token_node->tag_id == MyHTML_TAG_SVG ||
               token_node->tag_id == MyHTML_TAG_FRAMESET)
            {
                tree->token_namespace = token_node;
            }
            else if(tree->token_namespace && (token_node->type & MyHTML_TOKEN_TYPE_CLOSE) == 0) {
                const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, token_node->tag_id);
                
                if(tag_ctx->data_parser != MyHTML_TOKENIZER_STATE_DATA)
                {
                    myhtml_tree_wait_for_last_done_token(tree, token_node);
                    myhtml_tokenizer_state_set(tree) = tree->state_of_builder;
                }
            }
        }
    }
}

void myhtml_check_tag_parser(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset)
{
    myhtml_tag_t* tags = tree->tags;
    const myhtml_tag_context_t *tag_ctx = NULL;
    
    if(html_offset < token_node->raw_length) {
        const char *tagname = myhtml_tree_incomming_buffer_make_data(tree, token_node->raw_begin, token_node->raw_length);
        tag_ctx = myhtml_tag_get_by_name(tags, tagname, token_node->raw_length);
    }
    else {
        tag_ctx = myhtml_tag_get_by_name(tags, &html[ (token_node->raw_begin - tree->global_offset) ], token_node->raw_length);
    }
    
    if(tag_ctx) {
        token_node->tag_id = tag_ctx->id;
    }
    else {
        if(html_offset < token_node->raw_length) {
            const char *tagname = myhtml_tree_incomming_buffer_make_data(tree, token_node->raw_begin, token_node->raw_length);
            token_node->tag_id = myhtml_tag_add(tags, tagname, token_node->raw_length, MyHTML_TOKENIZER_STATE_DATA, true);
        }
        else {
            token_node->tag_id = myhtml_tag_add(tags, &html[ (token_node->raw_begin - tree->global_offset) ], token_node->raw_length, MyHTML_TOKENIZER_STATE_DATA, true);
        }
        
        myhtml_tag_set_category(tags, token_node->tag_id, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_ORDINARY);
    }
}

////
myhtml_token_node_t * myhtml_tokenizer_queue_create_text_node_if_need(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t absolute_html_offset, enum myhtml_token_type type)
{
    if(token_node->tag_id == MyHTML_TAG__UNDEF)
    {
        if(absolute_html_offset > token_node->raw_begin)
        {
            size_t tmp_begin = token_node->element_begin;
            
            token_node->type |= type;
            token_node->tag_id = MyHTML_TAG__TEXT;
            token_node->element_begin = token_node->raw_begin;
            token_node->raw_length = token_node->element_length = absolute_html_offset - token_node->raw_begin;
            
            if(myhtml_queue_add(tree, tmp_begin, token_node) != MyHTML_STATUS_OK) {
                return NULL;
            }
            
            return tree->current_token_node;
        }
    }
    
    return token_node;
}

void myhtml_tokenizer_set_state(myhtml_tree_t* tree, myhtml_token_node_t* token_node)
{
    if((token_node->type & MyHTML_TOKEN_TYPE_CLOSE) == 0)
    {
        if(token_node->tag_id == MyHTML_TAG_NOSCRIPT &&
           (tree->flags & MyHTML_TREE_FLAGS_SCRIPT) == 0)
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
        }
        else {
            const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, token_node->tag_id);
            myhtml_tokenizer_state_set(tree) = tag_ctx->data_parser;
        }
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
}

/////////////////////////////////////////////////////////
//// RCDATA
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_rcdata(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(tree->tmp_tag_id == 0) {
        token_node->raw_begin = (html_offset + tree->global_offset);
        
        mythread_queue_node_t* prev_qnode = mythread_queue_get_prev_node(tree->current_qnode);
        
        if(prev_qnode && prev_qnode->args) {
            tree->tmp_tag_id = ((myhtml_token_node_t*)(prev_qnode->args))->tag_id;
        }
        else if(tree->fragment) {
            tree->tmp_tag_id = tree->fragment->tag_id;
        }
    }
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            token_node->element_begin = (html_offset + tree->global_offset);
            
            html_offset++;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA_LESS_THAN_SIGN;
            
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rcdata_less_than_sign(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '/')
    {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_OPEN;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rcdata_end_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyCORE_STRING_MAP_CHAR_A_Z_a_z)
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_NAME;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
    }
    
    return html_offset;
}

bool _myhtml_tokenizer_state_andata_end_tag_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t *html_offset, size_t tmp_begin, enum myhtml_token_type type)
{
    token_node->raw_length = (*html_offset + tree->global_offset) - token_node->raw_begin;
    myhtml_check_tag_parser(tree, token_node, html, *html_offset);
    
    if(token_node->tag_id != tree->tmp_tag_id)
    {
        token_node->raw_begin  = tmp_begin;
        token_node->raw_length = 0;
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
        
        (*html_offset)++;
        return false;
    }
    
    if((token_node->raw_begin - 2) > tmp_begin)
    {
        size_t tmp_element_begin = token_node->element_begin;
        size_t tmp_raw_begin     = token_node->raw_begin;
        
        token_node->raw_length      = (token_node->raw_begin - 2) - tmp_begin;
        token_node->raw_begin       = tmp_begin;
        token_node->element_begin   = tmp_begin;
        token_node->element_length  = token_node->raw_length;
        token_node->type           |= type;
        token_node->type           ^= (token_node->type & MyHTML_TOKEN_TYPE_WHITESPACE);
        token_node->tag_id          = MyHTML_TAG__TEXT;
        
        /* TODO: return error */
        myhtml_queue_add(tree, *html_offset, token_node);
        
        /* return true values */
        token_node = tree->current_token_node;
        token_node->element_begin = tmp_element_begin;
        token_node->raw_begin = tmp_raw_begin;
    }
    
    token_node->tag_id         = tree->tmp_tag_id;
    token_node->type          |= MyHTML_TOKEN_TYPE_CLOSE;
    token_node->raw_length     = (tree->global_offset + *html_offset) - token_node->raw_begin;
    
    return true;
}

size_t myhtml_tokenizer_state_rcdata_end_tag_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    size_t tmp_begin = token_node->raw_begin;
    token_node->raw_begin = html_offset + tree->global_offset;
    
    while(html_offset < html_size)
    {
        if(mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyCORE_STRING_MAP_CHAR_WHITESPACE)
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, token_node, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RCDATA)) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
                
                tree->tmp_tag_id = 0;
                html_offset++;
                
                return html_offset;
            }
            
            break;
        }
        else if(html[html_offset] == '>')
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, token_node, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RCDATA)) {
                html_offset++;
                
                token_node = tree->current_token_node;
                token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
                
                if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                    myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                    return 0;
                }
                
                tree->tmp_tag_id = 0;
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
                
                return html_offset;
            }
            
            break;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, token_node, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RCDATA)) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
                
                tree->tmp_tag_id = 0;
                html_offset++;
                
                return html_offset;
            }
            
            break;
        }
        else if (mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] != MyCORE_STRING_MAP_CHAR_A_Z_a_z) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
            break;
        }
        
        html_offset++;
    }
    
    token_node->raw_begin = tmp_begin;
    return html_offset;
}

/////////////////////////////////////////////////////////
//// RAWTEXT
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_rawtext(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(tree->tmp_tag_id == 0) {
        token_node->raw_begin = (html_offset + tree->global_offset);
        
        mythread_queue_node_t* prev_qnode = mythread_queue_get_prev_node(tree->current_qnode);
        
        if(prev_qnode && prev_qnode->args) {
            tree->tmp_tag_id = ((myhtml_token_node_t*)prev_qnode->args)->tag_id;
        }
        else if(tree->fragment) {
            tree->tmp_tag_id = tree->fragment->tag_id;
        }
    }

    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            token_node->element_begin = (html_offset + tree->global_offset);
            
            html_offset++;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT_LESS_THAN_SIGN;
            
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rawtext_less_than_sign(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '/')
    {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_OPEN;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rawtext_end_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyCORE_STRING_MAP_CHAR_A_Z_a_z)
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_NAME;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rawtext_end_tag_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    size_t tmp_begin = token_node->raw_begin;
    token_node->raw_begin = html_offset + tree->global_offset;
    
    while(html_offset < html_size)
    {
        if(mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyCORE_STRING_MAP_CHAR_WHITESPACE)
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, token_node, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RAWTEXT)) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
                
                tree->tmp_tag_id = 0;
                html_offset++;
            }
            
            return html_offset;
        }
        else if(html[html_offset] == '>')
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, token_node, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RAWTEXT)) {
                html_offset++;
                
                token_node = tree->current_token_node;
                token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
                
                if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                    myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                    return 0;
                }
                
                tree->tmp_tag_id = 0;
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            }
            
            return html_offset;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, token_node, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RAWTEXT)) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
                
                tree->tmp_tag_id = 0;
                html_offset++;
            }
            
            return html_offset;
        }
        else if (mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] != MyCORE_STRING_MAP_CHAR_A_Z_a_z) {
            token_node->raw_begin = tmp_begin;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
            
            return html_offset;
        }
        
        html_offset++;
    }
    
    token_node->raw_begin = tmp_begin;
    return html_offset;
}

/////////////////////////////////////////////////////////
//// PLAINTEXT
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_plaintext(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if((token_node->type & MyHTML_TOKEN_TYPE_PLAINTEXT) == 0)
        token_node->type |= MyHTML_TOKEN_TYPE_PLAINTEXT;
    
    token_node->type      ^= (token_node->type & MyHTML_TOKEN_TYPE_WHITESPACE);
    token_node->raw_begin  = (html_offset + tree->global_offset);
    token_node->raw_length = token_node->element_length = (html_size + tree->global_offset) - token_node->raw_begin;
    token_node->tag_id     = MyHTML_TAG__TEXT;
    
    myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    
    if(myhtml_queue_add(tree, html_size, token_node) != MyHTML_STATUS_OK) {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
        return 0;
    }
    
    return html_size;
}

/////////////////////////////////////////////////////////
//// CDATA
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_cdata_section(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if((token_node->type & MyHTML_TOKEN_TYPE_CDATA) == 0)
        token_node->type |= MyHTML_TOKEN_TYPE_CDATA;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>')
        {
            const char *tagname;
            if(html_offset < 2)
                tagname = myhtml_tree_incomming_buffer_make_data(tree,((html_offset + tree->global_offset) - 2), 2);
            else
                tagname = &html[html_offset - 2];
            
            if(tagname[0] == ']' && tagname[1] == ']')
            {
               token_node->raw_length = (((html_offset + tree->global_offset) - 2) - token_node->raw_begin);
                html_offset++;
                
                if(token_node->raw_length) {
                    token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
                    
                    if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                        return 0;
                    }
                    
                }
                else {
                    token_node->raw_begin = html_offset + tree->global_offset;
                }
                
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
                break;
            }
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// outside of tag
//// %HERE%<div>%HERE%</div>%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_data(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            token_node->element_begin = (tree->global_offset + html_offset);
            
            html_offset++;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_TAG_OPEN;
            
            break;
        }
        else if(html[html_offset] == '\0' && (token_node->type & MyHTML_TOKEN_TYPE_NULL) == 0) {
            // parse error
            /* %EXTERNAL% VALIDATOR:TOKENIZER POSITION STATUS:CHAR_NULL LEVEL:ERROR BEGIN:html_offset LENGTH:1 */
            
            token_node->type |= MyHTML_TOKEN_TYPE_NULL;
        }
        else if(token_node->type & MyHTML_TOKEN_TYPE_WHITESPACE &&
                mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] != MyCORE_STRING_MAP_CHAR_WHITESPACE) {
            token_node->type ^= (token_node->type & MyHTML_TOKEN_TYPE_WHITESPACE);
            token_node->type |= MyHTML_TOKEN_TYPE_DATA;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// <%HERE%div></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyCORE_STRING_MAP_CHAR_A_Z_a_z)
    {
        token_node = myhtml_tokenizer_queue_create_text_node_if_need(tree, token_node, html, ((tree->global_offset + html_offset) - 1), MyHTML_TOKEN_TYPE_DATA);
        if(token_node == NULL) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
        
        token_node->raw_begin = tree->global_offset + html_offset;
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_TAG_NAME;
    }
    else if(html[html_offset] == '!')
    {
        token_node = myhtml_tokenizer_queue_create_text_node_if_need(tree, token_node, html, ((tree->global_offset + html_offset) - 1), MyHTML_TOKEN_TYPE_DATA);
        if(token_node == NULL) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
        
        html_offset++;
        token_node->raw_begin = tree->global_offset + html_offset;
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_MARKUP_DECLARATION_OPEN;
    }
    else if(html[html_offset] == '/')
    {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_END_TAG_OPEN;
    }
    else if(html[html_offset] == '?')
    {
        // parse error
        /* %EXTERNAL% VALIDATOR:TOKENIZER POSITION STATUS:CHAR_BAD LEVEL:ERROR BEGIN:html_offset LENGTH:1 */
        
        token_node = myhtml_tokenizer_queue_create_text_node_if_need(tree, token_node, html, ((tree->global_offset + html_offset) - 1), MyHTML_TOKEN_TYPE_DATA);
        if(token_node == NULL) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
        
        token_node->raw_begin = tree->global_offset + html_offset;
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
    }
    else {
        // parse error
        /* %EXTERNAL% VALIDATOR:TOKENIZER POSITION STATUS:NOT_EXPECTED LEVEL:ERROR BEGIN:html_offset LENGTH:1 */
        
        token_node->type ^= (token_node->type & MyHTML_TOKEN_TYPE_WHITESPACE);
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// </%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_end_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyCORE_STRING_MAP_CHAR_A_Z_a_z)
    {
        token_node = myhtml_tokenizer_queue_create_text_node_if_need(tree, token_node, html, ((tree->global_offset + html_offset) - 2), MyHTML_TOKEN_TYPE_DATA);
        if(token_node == NULL) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
        
        token_node->raw_begin = tree->global_offset + html_offset;
        token_node->type = MyHTML_TOKEN_TYPE_CLOSE;
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_TAG_NAME;
    }
    else if(html[html_offset] == '>')
    {
        // parse error
        /* %EXTERNAL% VALIDATOR:TOKENIZER POSITION STATUS:CHAR_BAD LEVEL:ERROR BEGIN:html_offset LENGTH:1 */
        
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else {
        // parse error
        /* %EXTERNAL% VALIDATOR:TOKENIZER POSITION STATUS:CHAR_BAD LEVEL:ERROR BEGIN:html_offset LENGTH:1 */
        
        token_node = myhtml_tokenizer_queue_create_text_node_if_need(tree, token_node, html, ((tree->global_offset + html_offset) - 2), MyHTML_TOKEN_TYPE_DATA);
        if(token_node == NULL) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
        
        token_node->raw_begin = tree->global_offset + html_offset;
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// <!%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_markup_declaration_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if((token_node->raw_begin + 2) > (html_size + tree->global_offset)) {
        tree->incoming_buf->length = html_offset;
        return html_size;
    }
    
    const char *tagname = myhtml_tree_incomming_buffer_make_data(tree, token_node->raw_begin, 2);
    
    // for a comment
    if(tagname[0] == '-' && tagname[1] == '-')
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_START;
        
        html_offset += 2;
        
        token_node->raw_begin  = html_offset + tree->global_offset;
        token_node->raw_length = 0;
        
        return html_offset;
    }
    
    if((token_node->raw_begin + 7) > (html_size + tree->global_offset)) {
        tree->incoming_buf->length = html_offset;
        return html_size;
    }
    
    tagname = myhtml_tree_incomming_buffer_make_data(tree, token_node->raw_begin, 7);
    
    if(mycore_strncasecmp(tagname, "DOCTYPE", 7) == 0)
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DOCTYPE;
        
        html_offset = (token_node->raw_begin + 7) - tree->incoming_buf->offset;
        
        token_node->raw_length  = 7;
        token_node->tag_id = MyHTML_TAG__DOCTYPE;
        
        return html_offset;
    }
    
    // CDATA sections can only be used in foreign content (MathML or SVG)
    if(strncmp(tagname, "[CDATA[", 7) == 0) {
        if(tree->current_qnode->prev && tree->current_qnode->prev->args)
        {
            myhtml_tree_wait_for_last_done_token(tree, tree->current_qnode->prev->args);
            myhtml_tree_node_t *adjusted_current_node = myhtml_tree_adjusted_current_node(tree);
            
            if(adjusted_current_node &&
               adjusted_current_node->ns != MyHTML_NAMESPACE_HTML)
            {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_CDATA_SECTION;
                
                html_offset = (token_node->raw_begin + 7) - tree->incoming_buf->offset;
                
                token_node->raw_begin += 7;
                token_node->raw_length = 0;
                token_node->tag_id = MyHTML_TAG__TEXT;
                token_node->type ^= (token_node->type & MyHTML_TOKEN_TYPE_WHITESPACE);
                
                return html_offset;
            }
        }
    }
    
    token_node->raw_length = 0;
    
    myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// <%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_tag_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(mycore_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyCORE_STRING_MAP_CHAR_WHITESPACE)
        {
            token_node->raw_length = (tree->global_offset + html_offset) - token_node->raw_begin;
            myhtml_check_tag_parser(tree, token_node, html, html_offset);
            
            html_offset++;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            break;
        }
        else if(html[html_offset] == '/')
        {
            token_node->raw_length = (tree->global_offset + html_offset) - token_node->raw_begin;
            myhtml_check_tag_parser(tree, token_node, html, html_offset);
            
            html_offset++;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SELF_CLOSING_START_TAG;
            
            break;
        }
        else if(html[html_offset] == '>')
        {
            token_node->raw_length = (tree->global_offset + html_offset) - token_node->raw_begin;
            
            myhtml_check_tag_parser(tree, token_node, html, html_offset);
            myhtml_tokenizer_set_state(tree, token_node);
            
            html_offset++;
            
            token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
            
            if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// <%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_self_closing_start_tag(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '>') {
        token_node->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
        myhtml_tokenizer_set_state(tree, token_node);
        
        html_offset++;
        
        // TODO: ??????
        token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
        
        if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, after tag name
//// <div%HERE% class="bla"></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_before_attribute_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    // skip WS
    myhtml_parser_skip_whitespace()
    
    if(html_offset >= html_size) {
        return html_offset;
    }
    
    if(html[html_offset] == '>')
    {
        myhtml_tokenizer_set_state(tree, token_node);
        
        html_offset++;
        
        token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
        
        if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
    }
    else if(html[html_offset] == '/') {
        token_node->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
        
        html_offset++;
    }
    else {
        myhtml_parser_queue_set_attr(tree, token_node)
        
        tree->attr_current->raw_key_begin    = html_offset + tree->global_offset;
        tree->attr_current->raw_key_length   = 0;
        tree->attr_current->raw_value_begin  = 0;
        tree->attr_current->raw_value_length = 0;
        
        if(html[html_offset] == '=') {
            // parse error
            /* %EXTERNAL% VALIDATOR:TOKENIZER POSITION STATUS:NOT_EXPECTED LEVEL:ERROR BEGIN:html_offset LENGTH:1 */
            
            html_offset++;
        }
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, inside of attr key
//// <div cla%HERE%ss="bla"></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_attribute_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            tree->attr_current->raw_key_length = (tree->global_offset + html_offset) - tree->attr_current->raw_key_begin;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_NAME;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '=')
        {
            tree->attr_current->raw_key_length = (tree->global_offset + html_offset) - tree->attr_current->raw_key_begin;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            tree->attr_current->raw_key_length = (tree->global_offset + html_offset) - tree->attr_current->raw_key_begin;
            myhtml_tokenizer_set_state(tree, token_node);
            
            html_offset++;
            
            token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
            
            if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            break;
        }
        else if(html[html_offset] == '/')
        {
            tree->attr_current->raw_key_length = (tree->global_offset + html_offset) - tree->attr_current->raw_key_begin;
            
            token_node->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, after attr key
//// <div class%HERE%="bla"></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_after_attribute_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '=')
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            myhtml_tokenizer_set_state(tree, token_node);
            
            html_offset++;
            
            token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
            
            if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            break;
        }
        else if(html[html_offset] == '"' || html[html_offset] == '\'' || html[html_offset] == '<')
        {
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            myhtml_parser_queue_set_attr(tree, token_node)
            
            tree->attr_current->raw_key_begin   = (tree->global_offset + html_offset);
            tree->attr_current->raw_key_length  = 0;
            tree->attr_current->raw_value_begin  = 0;
            tree->attr_current->raw_value_length = 0;
            
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME;
            break;
        }
        else if(myhtml_whithspace(html[html_offset], !=, &&))
        {
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            myhtml_parser_queue_set_attr(tree, token_node)
            
            tree->attr_current->raw_key_begin   = (html_offset + tree->global_offset);
            tree->attr_current->raw_key_length  = 0;
            tree->attr_current->raw_value_begin  = 0;
            tree->attr_current->raw_value_length = 0;
            
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, after attr key
//// <div class=%HERE%"bla"></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_before_attribute_value(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>') {
            // parse error
            /* %EXTERNAL% VALIDATOR:TOKENIZER POSITION STATUS:NOT_EXPECTED LEVEL:ERROR BEGIN:html_offset LENGTH:1 */
            
            myhtml_tokenizer_set_state(tree, token_node);
            
            html_offset++;
            
            token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
            
            if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            break;
        }
        else if(myhtml_whithspace(html[html_offset], !=, &&))
        {
            if(html[html_offset] == '"') {
                html_offset++;
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED;
            }
            else if(html[html_offset] == '\'') {
                html_offset++;
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED;
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_UNQUOTED;
            }
            
            tree->attr_current->raw_value_begin = (tree->global_offset + html_offset);
            
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, inside of attr value
//// <div class="bla%HERE%"></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_attribute_value_double_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '"')
        {
            tree->attr_current->raw_value_length = (tree->global_offset + html_offset) - tree->attr_current->raw_value_begin;
            
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_VALUE_QUOTED;
            
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, inside of attr value
//// <div class="bla%HERE%"></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_attribute_value_single_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '\'')
        {
            tree->attr_current->raw_value_length = (tree->global_offset + html_offset) - tree->attr_current->raw_value_begin;
            
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_VALUE_QUOTED;
            
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, inside of attr value
//// <div class="bla%HERE%"></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_attribute_value_unquoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->attr_current->raw_value_length = (tree->global_offset + html_offset) - tree->attr_current->raw_value_begin;
            
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>') {
            // parse error
            /* %EXTERNAL% VALIDATOR:TOKENIZER POSITION STATUS:UNSAFE_USE LEVEL:INFO BEGIN:html_offset LENGTH:1 */
            
            tree->attr_current->raw_value_length = (tree->global_offset + html_offset) - tree->attr_current->raw_value_begin;
            
            myhtml_tokenizer_set_state(tree, token_node);
            
            html_offset++;
            
            token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
            
            if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
            if(tree->attr_current == NULL) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_after_attribute_value_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(myhtml_whithspace(html[html_offset], ==, ||)) {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
        html_offset++;
    }
    else if(html[html_offset] == '/') {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SELF_CLOSING_START_TAG;
        html_offset++;
    }
    else if(html[html_offset] == '>') {
        myhtml_tokenizer_set_state(tree, token_node);
        
        html_offset++;
        
        token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
        
        if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// COMMENT
//// <!--%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_comment_start(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    token_node->tag_id = MyHTML_TAG__COMMENT;
    
    if(html[html_offset] == '-')
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_START_DASH;
    }
    else if(html[html_offset] == '>')
    {
        html_offset++;
        
        token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
        token_node->raw_length = 0;
        
        if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
        
        return html_offset;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment_start_dash(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    token_node->tag_id = MyHTML_TAG__COMMENT;
    
    if(html[html_offset] == '-')
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END;
    }
    else if(html[html_offset] == '>')
    {
        html_offset++;
        
        token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
        token_node->raw_length = 0;
        
        if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
        
        return html_offset;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    token_node->tag_id = MyHTML_TAG__COMMENT;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '-')
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END_DASH;
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment_end_dash(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '-')
    {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END;
    }
    else {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment_end(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '>')
    {
        token_node->raw_length = ((tree->global_offset + html_offset) - token_node->raw_begin);
        
        if(token_node->raw_length >= 2)
            token_node->raw_length -= 2;
        else
            token_node->raw_length = 0;
        
        html_offset++;
        
        token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
        
        if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
            return 0;
        }
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else if(html[html_offset] == '!') {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END_BANG;
    }
    else if(html[html_offset] == '-') {
        html_offset++;
    }
    else {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment_end_bang(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '>')
    {
        if(((tree->global_offset + html_offset) - 3) >= token_node->raw_begin) {
            token_node->raw_length = ((tree->global_offset + html_offset) - token_node->raw_begin) - 3;
            
            html_offset++;
            
            token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
            
            if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
        }
        else {
            html_offset++;
            
            token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
            token_node->raw_length = 0;
            
            if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
        }
        
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else if(html[html_offset] == '-') {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END_DASH;
    }
    else {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// BOGUS COMMENT
//// find >
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_bogus_comment(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    token_node->tag_id = MyHTML_TAG__COMMENT;
    token_node->type |= MyHTML_TOKEN_TYPE_COMMENT;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>')
        {
            token_node->raw_length = ((tree->global_offset + html_offset) - token_node->raw_begin);
            
            html_offset++;
            
            token_node->element_length = (tree->global_offset + html_offset) - token_node->element_begin;
            
            if(myhtml_queue_add(tree, html_offset, token_node) != MyHTML_STATUS_OK) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP;
                return 0;
            }
            
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// Parse error
//// find >
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_parse_error_stop(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size)
{
    tree->tokenizer_status = MyHTML_STATUS_TOKENIZER_ERROR_MEMORY_ALLOCATION;
    return html_size;
}

mystatus_t myhtml_tokenizer_state_init(myhtml_t* myhtml)
{
    myhtml->parse_state_func = (myhtml_tokenizer_state_f*)mycore_malloc(sizeof(myhtml_tokenizer_state_f) *
                                                                   ((MyHTML_TOKENIZER_STATE_LAST_ENTRY *
                                                                     MyHTML_TOKENIZER_STATE_LAST_ENTRY) + 1));
    
    if(myhtml->parse_state_func == NULL)
        return MyHTML_STATUS_TOKENIZER_ERROR_MEMORY_ALLOCATION;
    
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_DATA]                          = myhtml_tokenizer_state_data;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_TAG_OPEN]                      = myhtml_tokenizer_state_tag_open;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_TAG_NAME]                      = myhtml_tokenizer_state_tag_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_END_TAG_OPEN]                  = myhtml_tokenizer_state_end_tag_open;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SELF_CLOSING_START_TAG]        = myhtml_tokenizer_state_self_closing_start_tag;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_MARKUP_DECLARATION_OPEN]       = myhtml_tokenizer_state_markup_declaration_open;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME]         = myhtml_tokenizer_state_before_attribute_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME]                = myhtml_tokenizer_state_attribute_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_NAME]          = myhtml_tokenizer_state_after_attribute_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE]        = myhtml_tokenizer_state_before_attribute_value;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_VALUE_QUOTED]  = myhtml_tokenizer_state_after_attribute_value_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED] = myhtml_tokenizer_state_attribute_value_double_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED] = myhtml_tokenizer_state_attribute_value_single_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_UNQUOTED]      = myhtml_tokenizer_state_attribute_value_unquoted;
    
    // comments
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_COMMENT_START]                 = myhtml_tokenizer_state_comment_start;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_COMMENT_START_DASH]            = myhtml_tokenizer_state_comment_start_dash;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_COMMENT]                       = myhtml_tokenizer_state_comment;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_COMMENT_END]                   = myhtml_tokenizer_state_comment_end;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_COMMENT_END_DASH]              = myhtml_tokenizer_state_comment_end_dash;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_COMMENT_END_BANG]              = myhtml_tokenizer_state_comment_end_bang;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BOGUS_COMMENT]                 = myhtml_tokenizer_state_bogus_comment;
    
    // cdata
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_CDATA_SECTION]                 = myhtml_tokenizer_state_cdata_section;
    
    // rcdata
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RCDATA]                        = myhtml_tokenizer_state_rcdata;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RCDATA_LESS_THAN_SIGN]         = myhtml_tokenizer_state_rcdata_less_than_sign;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_OPEN]           = myhtml_tokenizer_state_rcdata_end_tag_open;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_NAME]           = myhtml_tokenizer_state_rcdata_end_tag_name;
    
    // rawtext
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RAWTEXT]                        = myhtml_tokenizer_state_rawtext;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RAWTEXT_LESS_THAN_SIGN]         = myhtml_tokenizer_state_rawtext_less_than_sign;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_OPEN]           = myhtml_tokenizer_state_rawtext_end_tag_open;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_NAME]           = myhtml_tokenizer_state_rawtext_end_tag_name;
    
    // plaintext
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_PLAINTEXT]                     = myhtml_tokenizer_state_plaintext;
    
    // doctype
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_DOCTYPE]                                 = myhtml_tokenizer_state_doctype;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_NAME]                     = myhtml_tokenizer_state_before_doctype_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_DOCTYPE_NAME]                            = myhtml_tokenizer_state_doctype_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_NAME]                      = myhtml_tokenizer_state_after_doctype_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_CUSTOM_AFTER_DOCTYPE_NAME_A_Z]           = myhtml_tokenizer_state_custom_after_doctype_name_a_z;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_PUBLIC_IDENTIFIER]        = myhtml_tokenizer_state_before_doctype_public_identifier;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED] = myhtml_tokenizer_state_doctype_public_identifier_double_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED] = myhtml_tokenizer_state_doctype_public_identifier_single_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_PUBLIC_IDENTIFIER]         = myhtml_tokenizer_state_after_doctype_public_identifier;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED] = myhtml_tokenizer_state_doctype_system_identifier_double_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED] = myhtml_tokenizer_state_doctype_system_identifier_single_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_SYSTEM_IDENTIFIER]         = myhtml_tokenizer_state_after_doctype_system_identifier;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BOGUS_DOCTYPE]                           = myhtml_tokenizer_state_bogus_doctype;
    
    // script
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA]                               = myhtml_tokenizer_state_script_data;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_LESS_THAN_SIGN]                = myhtml_tokenizer_state_script_data_less_than_sign;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_OPEN]                  = myhtml_tokenizer_state_script_data_end_tag_open;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_NAME]                  = myhtml_tokenizer_state_script_data_end_tag_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPE_START]                  = myhtml_tokenizer_state_script_data_escape_start;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPE_START_DASH]             = myhtml_tokenizer_state_script_data_escape_start_dash;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED]                       = myhtml_tokenizer_state_script_data_escaped;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH]                  = myhtml_tokenizer_state_script_data_escaped_dash;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH_DASH]             = myhtml_tokenizer_state_script_data_escaped_dash_dash;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN]        = myhtml_tokenizer_state_script_data_escaped_less_than_sign;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_OPEN]          = myhtml_tokenizer_state_script_data_escaped_end_tag_open;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_NAME]          = myhtml_tokenizer_state_script_data_escaped_end_tag_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_START]           = myhtml_tokenizer_state_script_data_double_escape_start;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED]                = myhtml_tokenizer_state_script_data_double_escaped;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH]           = myhtml_tokenizer_state_script_data_double_escaped_dash;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH_DASH]      = myhtml_tokenizer_state_script_data_double_escaped_dash_dash;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN] = myhtml_tokenizer_state_script_data_double_escaped_less_than_sign;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_END]             = myhtml_tokenizer_state_script_data_double_escape_end;
    
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP]                          = myhtml_tokenizer_state_parse_error_stop;
    
    
    // ***********
    // for ends
    // *********
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_DATA)]                          = myhtml_tokenizer_end_state_data;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_TAG_OPEN)]                      = myhtml_tokenizer_end_state_tag_open;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_TAG_NAME)]                      = myhtml_tokenizer_end_state_tag_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_END_TAG_OPEN)]                  = myhtml_tokenizer_end_state_end_tag_open;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SELF_CLOSING_START_TAG)]        = myhtml_tokenizer_end_state_self_closing_start_tag;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_MARKUP_DECLARATION_OPEN)]       = myhtml_tokenizer_end_state_markup_declaration_open;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME)]         = myhtml_tokenizer_end_state_before_attribute_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME)]                = myhtml_tokenizer_end_state_attribute_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_NAME)]          = myhtml_tokenizer_end_state_after_attribute_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE)]        = myhtml_tokenizer_end_state_before_attribute_value;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED)] = myhtml_tokenizer_end_state_attribute_value_double_quoted;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED)] = myhtml_tokenizer_end_state_attribute_value_single_quoted;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_UNQUOTED)]      = myhtml_tokenizer_end_state_attribute_value_unquoted;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_VALUE_QUOTED)]  = myhtml_tokenizer_end_state_after_attribute_value_quoted;
    
    // for ends comments
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_COMMENT_START)]                 = myhtml_tokenizer_end_state_comment_start;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_COMMENT_START_DASH)]            = myhtml_tokenizer_end_state_comment_start_dash;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_COMMENT)]                       = myhtml_tokenizer_end_state_comment;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_COMMENT_END)]                   = myhtml_tokenizer_end_state_comment_end;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_COMMENT_END_DASH)]              = myhtml_tokenizer_end_state_comment_end_dash;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_COMMENT_END_BANG)]              = myhtml_tokenizer_end_state_comment_end_bang;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_BOGUS_COMMENT)]                 = myhtml_tokenizer_end_state_bogus_comment;
    
    // for ends cdata
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_CDATA_SECTION)]                 = myhtml_tokenizer_end_state_cdata_section;
    
    // rcdata
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RCDATA)]                        = myhtml_tokenizer_end_state_rcdata;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RCDATA_LESS_THAN_SIGN)]         = myhtml_tokenizer_end_state_rcdata_less_than_sign;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_OPEN)]           = myhtml_tokenizer_end_state_rcdata_end_tag_open;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_NAME)]           = myhtml_tokenizer_end_state_rcdata_end_tag_name;
    
    // rawtext
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RAWTEXT)]                        = myhtml_tokenizer_end_state_rawtext;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RAWTEXT_LESS_THAN_SIGN)]         = myhtml_tokenizer_end_state_rawtext_less_than_sign;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_OPEN)]           = myhtml_tokenizer_end_state_rawtext_end_tag_open;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_NAME)]           = myhtml_tokenizer_end_state_rawtext_end_tag_name;
    
    // for ends plaintext
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_PLAINTEXT)]                     = myhtml_tokenizer_end_state_plaintext;
    
    // for ends doctype
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_DOCTYPE)]                                 = myhtml_tokenizer_end_state_doctype;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_NAME)]                     = myhtml_tokenizer_end_state_before_doctype_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_DOCTYPE_NAME)]                            = myhtml_tokenizer_end_state_doctype_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_NAME)]                      = myhtml_tokenizer_end_state_after_doctype_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_CUSTOM_AFTER_DOCTYPE_NAME_A_Z)]           = myhtml_tokenizer_end_state_custom_after_doctype_name_a_z;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_PUBLIC_IDENTIFIER)]        = myhtml_tokenizer_end_state_before_doctype_public_identifier;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED)] = myhtml_tokenizer_end_state_doctype_public_identifier_double_quoted;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED)] = myhtml_tokenizer_end_state_doctype_public_identifier_single_quoted;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_PUBLIC_IDENTIFIER)]         = myhtml_tokenizer_end_state_after_doctype_public_identifier;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED)] = myhtml_tokenizer_end_state_doctype_system_identifier_double_quoted;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED)] = myhtml_tokenizer_end_state_doctype_system_identifier_single_quoted;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_SYSTEM_IDENTIFIER)]         = myhtml_tokenizer_end_state_after_doctype_system_identifier;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_BOGUS_DOCTYPE)]                           = myhtml_tokenizer_end_state_bogus_doctype;
    
    // for ends script
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA)]                               = myhtml_tokenizer_end_state_script_data;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_LESS_THAN_SIGN)]                = myhtml_tokenizer_end_state_script_data_less_than_sign;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_OPEN)]                  = myhtml_tokenizer_end_state_script_data_end_tag_open;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_NAME)]                  = myhtml_tokenizer_end_state_script_data_end_tag_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPE_START)]                  = myhtml_tokenizer_end_state_script_data_escape_start;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPE_START_DASH)]             = myhtml_tokenizer_end_state_script_data_escape_start_dash;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED)]                       = myhtml_tokenizer_end_state_script_data_escaped;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH)]                  = myhtml_tokenizer_end_state_script_data_escaped_dash;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH_DASH)]             = myhtml_tokenizer_end_state_script_data_escaped_dash_dash;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN)]        = myhtml_tokenizer_end_state_script_data_escaped_less_than_sign;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_OPEN)]          = myhtml_tokenizer_end_state_script_data_escaped_end_tag_open;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_NAME)]          = myhtml_tokenizer_end_state_script_data_escaped_end_tag_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_START)]           = myhtml_tokenizer_end_state_script_data_double_escape_start;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED)]                = myhtml_tokenizer_end_state_script_data_double_escaped;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH)]           = myhtml_tokenizer_end_state_script_data_double_escaped_dash;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH_DASH)]      = myhtml_tokenizer_end_state_script_data_double_escaped_dash_dash;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN)] = myhtml_tokenizer_end_state_script_data_double_escaped_less_than_sign;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_END)]             = myhtml_tokenizer_end_state_script_data_double_escape_end;
    
    // parse error
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP)]                          = myhtml_tokenizer_end_state_parse_error_stop;
    
    return MyHTML_STATUS_OK;
}

void myhtml_tokenizer_state_destroy(myhtml_t* myhtml)
{
    if(myhtml->parse_state_func)
        mycore_free(myhtml->parse_state_func);
}




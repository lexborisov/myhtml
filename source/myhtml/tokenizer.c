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

#include "myhtml/tokenizer.h"

static const unsigned char myhtml_tokenizer_chars_map[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x02, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00
};

void myhtml_tokenizer_set_first_settings(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    tree->current_qnode       = mythread_queue_get_current_node(tree->queue);
    
    mythread_queue_node_clean(tree->current_qnode);
    
    tree->current_qnode->text = html;
    tree->current_qnode->tree = tree;
    
    myhtml_token_node_malloc(tree->token, tree->current_qnode->token, tree->mcasync_token_id);
    
    tree->incoming_buf_first = tree->incoming_buf;
}

void myhtml_tokenizer_begin(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    myhtml_tokenizer_chunk(tree, html, html_length);
}

void myhtml_tokenizer_chunk_process(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    myhtml_t* myhtml = tree->myhtml;
    myhtml_tokenizer_state_f* state_f = myhtml->parse_state_func;
    
    // add for a chunk
    myhtml_incomming_buf_add(myhtml, tree, tree->incoming_buf, html, html_length);
    
#ifndef MyHTML_BUILD_WITHOUT_THREADS
    
    if(myhtml->opt & MyHTML_OPTIONS_PARSE_MODE_SINGLE)
        tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    if(tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE)
    {
        if(tree->single_queue) {
            tree->queue = tree->single_queue;
        }
        else {
            tree->queue = mythread_queue_create(4096, NULL);
            tree->single_queue = tree->queue;
        }
    }
    else {
        tree->queue = myhtml->thread->queue;
        myhtml_tokenizer_post(tree);
    }
    
#else
    
    tree->queue = myhtml->thread->queue;
    tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
#endif
    
    if(tree->current_qnode == NULL) {
        myhtml_tokenizer_set_first_settings(tree, html, html_length);
    }
    
    size_t offset = 0;
    
    while (offset < html_length) {
        offset = state_f[tree->state](tree, tree->current_qnode, html, offset, html_length);
    }
    
    tree->global_offset += html_length;
}

void myhtml_tokenizer_chunk(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    if(tree->encoding_usereq != MyHTML_ENCODING_UTF_16LE &&
       tree->encoding_usereq != MyHTML_ENCODING_UTF_16BE) {
        myhtml_tokenizer_chunk_process(tree, html, html_length);
    }
    else
    {
        unsigned const char* u_html = (unsigned const char*)html;
        myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(tree->encoding);
        
        if(tree->temp_stream == NULL || tree->temp_stream->current == NULL)
            myhtml_tree_temp_stream_alloc(tree, (4096 * 2));
        
        struct myhtml_tree_temp_tag_name *current_stream = tree->temp_stream->current;
        
        size_t current_pos = current_stream->length;
        
        for (size_t i = 0; i < html_length; i++)
        {
            if(func(u_html[i], &tree->temp_stream->res) == MyHTML_ENCODING_STATUS_OK)
            {
                if((current_stream->length + 4) >= current_stream->size)
                {
                    tree->encoding = MyHTML_ENCODING_UTF_8;
                    myhtml_tokenizer_chunk_process(tree, &current_stream->data[current_pos], current_stream->length);
                    
                    current_stream = myhtml_tree_temp_stream_alloc(tree, (4096 * 2));
                    
                    if(current_stream == NULL)
                        return;
                    
                    current_pos = current_stream->length;
                }
                
                current_stream->length += myhtml_encoding_codepoint_to_ascii_utf_8(tree->temp_stream->res.result, &current_stream->data[current_stream->length]);
            }
        }
        
        if((current_stream->length - current_pos)) {
            tree->encoding = MyHTML_ENCODING_UTF_8;
            myhtml_tokenizer_chunk_process(tree, current_stream->data, current_stream->length);
        }
    }
}

void myhtml_tokenizer_end(myhtml_tree_t* tree)
{
    if(tree->incoming_buf)
    {
        tree->global_offset -= tree->incoming_buf->size;
        
        tree->myhtml->parse_state_func[(tree->state + MyHTML_TOKENIZER_STATE_LAST_ENTRY)]
        (tree, tree->current_qnode, tree->incoming_buf->data, tree->incoming_buf->size, tree->incoming_buf->size);
    }
    
    tree->current_qnode->token->tag_ctx_idx = MyHTML_TAG__END_OF_FILE;
    mh_queue_add(tree, NULL, 0, NULL);
    
#ifndef MyHTML_BUILD_WITHOUT_THREADS
    
    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0 &&
       (tree->myhtml->opt & MyHTML_OPTIONS_PARSE_MODE_SINGLE) == 0)
    {
        myhtml_tokenizer_wait(tree);
        myhtml_tokenizer_pause(tree);
    }
    
#endif
    
    tree->flags |= MyHTML_TREE_FLAGS_PARSE_END;
    
    myhtml_tree_temp_stream_clean(tree);
    
#ifdef DEBUG_MODE
    if(tree->open_elements->length) {
        MyHTML_DEBUG_ERROR("Tokenizer end; Open Elements is %zu", tree->open_elements->length);
    }
    if(tree->active_formatting->length) {
        MyHTML_DEBUG_ERROR("Tokenizer end; Active Formatting Elements is %zu", tree->active_formatting->length);
    }
#endif
}

myhtml_tree_node_t * myhtml_tokenizer_fragment_init(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, enum myhtml_namespace my_namespace)
{
    // step 3
    tree->fragment = myhtml_tree_node_create(tree);
    tree->fragment->my_namespace = my_namespace;
    tree->fragment->tag_idx = tag_idx;
    
    // step 4, is already done
    if(my_namespace == MyHTML_NAMESPACE_HTML) {
        if(tag_idx == MyHTML_TAG_NOSCRIPT) {
            if(tree->flags & MyHTML_TREE_FLAGS_SCRIPT) {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            }
        }
        else {
            const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, tag_idx);
            mh_state_set(tree) = tag_ctx->data_parser;
        }
    }
    
    mcobject_async_status_t mcstatus;
    tree->fragment->token = (myhtml_token_node_t*)mcobject_async_malloc(tree->token->nodes_obj, tree->mcasync_token_id, &mcstatus);
    
    if(mcstatus)
        return NULL;
    
    myhtml_token_node_clean(tree->fragment->token);
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
    myhtml_t* myhtml = tree->myhtml;
    
    mythread_wait_all(myhtml->thread);
}

void myhtml_tokenizer_post(myhtml_tree_t* tree)
{
    myhtml_t* myhtml = tree->myhtml;
    
    mythread_resume_all(myhtml->thread);
}

void myhtml_tokenizer_pause(myhtml_tree_t* tree)
{
    myhtml_t* myhtml = tree->myhtml;
    
    mythread_stream_pause_all(myhtml->thread);
    mythread_batch_pause_all(myhtml->thread);
}

void myhtml_tokenizer_calc_current_namespace(myhtml_tree_t* tree, mythread_queue_node_t* qnode)
{
    if(tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE)
    {
        mh_state_set(tree) = tree->state_of_builder;
    }
    else {
        if(qnode->token->tag_ctx_idx == MyHTML_TAG_MATH ||
           qnode->token->tag_ctx_idx == MyHTML_TAG_SVG ||
           qnode->token->tag_ctx_idx == MyHTML_TAG_FRAMESET)
        {
            tree->token_namespace = qnode->token;
        }
        else if(tree->token_namespace && (qnode->token->type & MyHTML_TOKEN_TYPE_CLOSE) == 0) {
            const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, qnode->token->tag_ctx_idx);
            
            if(tag_ctx->data_parser != MyHTML_TOKENIZER_STATE_DATA)
            {
                myhtml_tree_wait_for_last_done_token(tree, qnode->token);
                mh_state_set(tree) = tree->state_of_builder;
            }
        }
    }
}

//void myhtml_tokenizer_calc_current_namespace(myhtml_tree_t* tree, mythread_queue_node_t* qnode)
//{
//    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
//    {
//        if(qnode->token->tag_ctx_idx == MyHTML_TAG_MATH ||
//           qnode->token->tag_ctx_idx == MyHTML_TAG_SVG ||
//           qnode->token->tag_ctx_idx == MyHTML_TAG_FRAMESET)
//        {
//            tree->token_namespace = qnode->token;
//            
//            myhtml_tokenizer_wait(tree);
//            myhtml_tokenizer_pause(tree);
//            
//            tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
//        }
//    }
//    else if(tree->token_namespace)
//    {
//        mh_state_set(tree) = tree->state_of_builder;
//    }
//}

void myhtml_check_tag_parser(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset)
{
    myhtml_tag_t* tags = tree->tags;
    const myhtml_tag_context_t *tag_ctx = NULL;
    
    if(html_offset < qnode->length) {
        const char *tagname = myhtml_tree_incomming_buf_make_data(tree, qnode, qnode->length);
        tag_ctx = myhtml_tag_get_by_name(tags, tagname, qnode->length);
    }
    else {
        tag_ctx = myhtml_tag_get_by_name(tags, &html[ (qnode->begin - tree->global_offset) ], qnode->length);
    }
    
    if(tag_ctx) {
        qnode->token->tag_ctx_idx = tag_ctx->id;
    }
    else {
        if(html_offset < qnode->length) {
            const char *tagname = myhtml_tree_incomming_buf_make_data(tree, qnode, qnode->length);
            qnode->token->tag_ctx_idx = myhtml_tag_add(tags, tagname, qnode->length, MyHTML_TOKENIZER_STATE_DATA, true);
        }
        else {
            qnode->token->tag_ctx_idx = myhtml_tag_add(tags, &html[ (qnode->begin - tree->global_offset) ], qnode->length, MyHTML_TOKENIZER_STATE_DATA, true);
        }
        
        myhtml_tag_set_category(tags, qnode->token->tag_ctx_idx, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_ORDINARY);
    }
}

////
mythread_queue_node_t * myhtml_tokenizer_queue_create_text_node_if_need(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t absolute_html_offset, enum myhtml_token_type type)
{
    if(qnode->token->tag_ctx_idx == MyHTML_TAG__UNDEF)
    {
        if(absolute_html_offset > qnode->begin)
        {
            qnode->token->type |= type;
            qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
            qnode->length = absolute_html_offset - qnode->begin;
            mh_queue_add(tree, html, 0, qnode);
            
            return tree->current_qnode;
        }
    }
    
    return qnode;
}

void myhtml_tokenizer_set_state(myhtml_tree_t* tree, mythread_queue_node_t* qnode)
{
    if((qnode->token->type & MyHTML_TOKEN_TYPE_CLOSE) == 0)
    {
        if(qnode->token->tag_ctx_idx == MyHTML_TAG_NOSCRIPT &&
           (tree->flags & MyHTML_TREE_FLAGS_SCRIPT) == 0)
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
        }
        else {
            const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, qnode->token->tag_ctx_idx);
            mh_state_set(tree) = tag_ctx->data_parser;
        }
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
}

/////////////////////////////////////////////////////////
//// RCDATA
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_rcdata(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(tree->tmp_tag_id == 0) {
        qnode->begin = (html_offset + tree->global_offset);
        
        mythread_queue_node_t* prev_qnode = mythread_queue_get_prev_node(tree->queue);
        if(prev_qnode && prev_qnode->token) {
            tree->tmp_tag_id = prev_qnode->token->tag_ctx_idx;
        }
        else if(tree->fragment) {
            tree->tmp_tag_id = tree->fragment->tag_idx;
        }
    }
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            html_offset++;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA_LESS_THAN_SIGN;
            
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rcdata_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '/')
    {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_OPEN;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rcdata_end_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyHTML_TOKENIZER_CHAR_A_Z_a_z)
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_NAME;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
    }
    
    return html_offset;
}

bool _myhtml_tokenizer_state_andata_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t *html_offset, size_t tmp_begin, enum myhtml_token_type type)
{
    qnode->length = (*html_offset + tree->global_offset) - qnode->begin;
    myhtml_check_tag_parser(tree, qnode, html, *html_offset);
    
    if(qnode->token->tag_ctx_idx != tree->tmp_tag_id)
    {
        qnode->begin  = tmp_begin;
        qnode->length = 0;
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
        
        (*html_offset)++;
        return false;
    }
    
    if((qnode->begin - 2) > tmp_begin)
    {
        qnode->length             = (qnode->begin - 2) - tmp_begin;
        qnode->begin              = tmp_begin;
        qnode->token->type       |= type;
        qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
        
        mh_queue_add(tree, html, *html_offset, qnode);
        qnode = tree->current_qnode;
    }
    
    qnode->token->tag_ctx_idx = tree->tmp_tag_id;
    qnode->token->type |= MyHTML_TOKEN_TYPE_CLOSE;
    
    return true;
}

size_t myhtml_tokenizer_state_rcdata_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    size_t tmp_begin = qnode->begin;
    qnode->begin = html_offset + tree->global_offset;
    
    while(html_offset < html_size)
    {
        if(myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyHTML_TOKENIZER_CHAR_WHITESPACE)
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, qnode, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RCDATA)) {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
                
                tree->tmp_tag_id = 0;
                html_offset++;
                
                return html_offset;
            }
            
            break;
        }
        else if(html[html_offset] == '>')
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, qnode, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RCDATA)) {
                html_offset++;
                mh_queue_add(tree, html, html_offset, qnode);
                
                tree->tmp_tag_id = 0;
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
                
                return html_offset;
            }
            
            break;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, qnode, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RCDATA)) {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
                
                tree->tmp_tag_id = 0;
                html_offset++;
                
                return html_offset;
            }
            
            break;
        }
        else if (myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] != MyHTML_TOKENIZER_CHAR_A_Z_a_z) {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
            break;
        }
        
        html_offset++;
    }
    
    qnode->begin = tmp_begin;
    return html_offset;
}

/////////////////////////////////////////////////////////
//// RAWTEXT
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_rawtext(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(tree->tmp_tag_id == 0) {
        qnode->begin = (html_offset + tree->global_offset);
        
        mythread_queue_node_t* prev_qnode = mythread_queue_get_prev_node(tree->queue);
        if(prev_qnode && prev_qnode->token) {
            tree->tmp_tag_id = prev_qnode->token->tag_ctx_idx;
        }
        else if(tree->fragment) {
            tree->tmp_tag_id = tree->fragment->tag_idx;
        }
    }

    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            html_offset++;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT_LESS_THAN_SIGN;
            
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rawtext_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '/')
    {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_OPEN;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rawtext_end_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyHTML_TOKENIZER_CHAR_A_Z_a_z)
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_NAME;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rawtext_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    size_t tmp_begin = qnode->begin;
    qnode->begin = html_offset + tree->global_offset;
    
    while(html_offset < html_size)
    {
        if(myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyHTML_TOKENIZER_CHAR_WHITESPACE)
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, qnode, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RAWTEXT)) {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
                
                tree->tmp_tag_id = 0;
                html_offset++;
            }
            
            return html_offset;
        }
        else if(html[html_offset] == '>')
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, qnode, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RAWTEXT)) {
                html_offset++;
                mh_queue_add(tree, html, html_offset, qnode);
                
                tree->tmp_tag_id = 0;
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            }
            
            return html_offset;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            if(_myhtml_tokenizer_state_andata_end_tag_name(tree, qnode, html, &html_offset, tmp_begin, MyHTML_TOKEN_TYPE_RAWTEXT)) {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
                
                tree->tmp_tag_id = 0;
                html_offset++;
            }
            
            return html_offset;
        }
        else if (myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] != MyHTML_TOKENIZER_CHAR_A_Z_a_z) {
            qnode->begin = tmp_begin;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
            
            return html_offset;
        }
        
        html_offset++;
    }
    
    qnode->begin = tmp_begin;
    return html_offset;
}

/////////////////////////////////////////////////////////
//// PLAINTEXT
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_plaintext(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if((qnode->token->type & MyHTML_TOKEN_TYPE_PLAINTEXT) == 0)
        qnode->token->type |= MyHTML_TOKEN_TYPE_PLAINTEXT;
    
    qnode->begin = (html_offset + tree->global_offset);
    qnode->length = (html_size + tree->global_offset) - qnode->begin;
    qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
    
    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    mh_queue_add(tree, html, html_size, qnode);
    
    return html_size;
}

/////////////////////////////////////////////////////////
//// CDATA
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_cdata_section(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if((qnode->token->type & MyHTML_TOKEN_TYPE_CDATA) == 0)
        qnode->token->type |= MyHTML_TOKEN_TYPE_CDATA;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>')
        {
            const char *tagname;
            if(html_offset < 2)
                tagname = myhtml_tree_incomming_buf_get_last(tree, tree->incoming_buf, html_offset, 2);
            else
                tagname = &html[html_offset - 2];
            
            if(tagname[0] == ']' && tagname[1] == ']')
            {
                qnode->length = (((html_offset + tree->global_offset) - 2) - qnode->begin);
                html_offset++;
                
                if(qnode->length) {
                    mh_queue_add(tree, html, html_offset, qnode);
                }
                else {
                    qnode->begin = html_offset + tree->global_offset;
                }
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
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
size_t myhtml_tokenizer_state_data(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            html_offset++;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_TAG_OPEN;
            
            break;
        }
        else if(html[html_offset] == '\0') {
            size_t ab_offset = (tree->global_offset + html_offset);
            
            if(ab_offset > qnode->begin)
            {
                qnode->token->type |= MyHTML_TOKEN_TYPE_DATA;
                qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
                qnode->length = ab_offset - qnode->begin;
                mh_queue_add(tree, html, ab_offset, qnode);
                
                qnode = tree->current_qnode;
            }
            else
                qnode->begin = ab_offset;
            
            ab_offset++;
            
            qnode->token->type |= MyHTML_TOKEN_TYPE_DATA|MyHTML_TOKEN_TYPE_NULL;
            qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
            qnode->length = 1;
            mh_queue_add(tree, html, ab_offset, qnode);
            
            qnode = tree->current_qnode;
        }
//        else if(qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
//        {
//            if(myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] != MyHTML_TOKENIZER_CHAR_WHITESPACE)
//            {
//                size_t ab_offset = (tree->global_offset + html_offset);
//                
//                if(ab_offset > qnode->begin)
//                {
//                    qnode->token->type |= MyHTML_TOKEN_TYPE_DATA;
//                    qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
//                    qnode->length = ab_offset - qnode->begin;
//                    mh_queue_add(tree, html, ab_offset, qnode);
//                    
//                    qnode = tree->current_qnode;
//                }
//                
//                qnode->token->type ^= (qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE);
//            }
//        }
        else if(qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE &&
                myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] != MyHTML_TOKENIZER_CHAR_WHITESPACE) {
            qnode->token->type ^= (qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE);
            qnode->token->type = MyHTML_TOKEN_TYPE_DATA;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// <%HERE%div></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyHTML_TOKENIZER_CHAR_A_Z_a_z)
    {
        qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((tree->global_offset + html_offset) - 1), MyHTML_TOKEN_TYPE_DATA);
        
        qnode->begin = tree->global_offset + html_offset;
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_TAG_NAME;
    }
    else if(html[html_offset] == '!')
    {
        qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((tree->global_offset + html_offset) - 1), MyHTML_TOKEN_TYPE_DATA);
        
        html_offset++;
        qnode->begin = tree->global_offset + html_offset;
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_MARKUP_DECLARATION_OPEN;
    }
    else if(html[html_offset] == '/')
    {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_END_TAG_OPEN;
    }
    else if(html[html_offset] == '?')
    {
        qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((tree->global_offset + html_offset) - 1), MyHTML_TOKEN_TYPE_DATA);
        
        qnode->begin = tree->global_offset + html_offset;
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
    }
    else {
        qnode->token->type ^= (qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE);
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// </%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_end_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyHTML_TOKENIZER_CHAR_A_Z_a_z)
    {
        qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((tree->global_offset + html_offset) - 2), MyHTML_TOKEN_TYPE_DATA);
        
        qnode->begin = tree->global_offset + html_offset;
        qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_TAG_NAME;
    }
    else if(html[html_offset] == '>')
    {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else {
        qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((tree->global_offset + html_offset) - 2), MyHTML_TOKEN_TYPE_DATA);
        
        qnode->begin = tree->global_offset + html_offset;
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// <!%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_markup_declaration_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if((qnode->begin + 2) > (html_size + tree->global_offset)) {
        tree->incoming_buf->length = html_offset;
        return html_size;
    }
    
    const char *tagname = myhtml_tree_incomming_buf_make_data(tree, qnode, 2);
    
    // for a comment
    if(tagname[0] == '-' && tagname[1] == '-')
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
        
        qnode->begin  = (html_offset + tree->global_offset) + 2;
        qnode->length = 0;
        
        return html_offset;
    }
    
    if((qnode->begin + 7) > (html_size + tree->global_offset)) {
        tree->incoming_buf->length = html_offset;
        return html_size;
    }
    
    tagname = myhtml_tree_incomming_buf_make_data(tree, qnode, 7);
    
    if(myhtml_strncasecmp(tagname, "DOCTYPE", 7) == 0)
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DOCTYPE;
        
        html_offset = (qnode->begin + 7) - tree->incoming_buf->offset;
        
        qnode->length  = 7;
        qnode->token->tag_ctx_idx = MyHTML_TAG__DOCTYPE;
        
        return html_offset;
    }
    
    // CDATA sections can only be used in foreign content (MathML or SVG)
    if(strncmp(tagname, "[CDATA[", 7) == 0)
    {
        if(qnode->prev && qnode->prev->token) {
            myhtml_tree_wait_for_last_done_token(tree, qnode->prev->token);
            myhtml_tree_node_t *adjusted_current_node = myhtml_tree_adjusted_current_node(tree);
            
            if(adjusted_current_node &&
               adjusted_current_node->my_namespace != MyHTML_NAMESPACE_HTML)
            {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_CDATA_SECTION;
                
                html_offset = (qnode->begin + 7) - tree->incoming_buf->offset;
                
                qnode->begin += 7;
                qnode->length = 0;
                qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
                
                return html_offset;
            }
        }
    }
    
    qnode->length = 0;
    
    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// <%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_tokenizer_chars_map[ (unsigned char)html[html_offset] ] == MyHTML_TOKENIZER_CHAR_WHITESPACE)
        {
            qnode->length = (tree->global_offset + html_offset) - qnode->begin;
            myhtml_check_tag_parser(tree, qnode, html, html_offset);
            
            html_offset++;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            break;
        }
        else if(html[html_offset] == '/')
        {
            qnode->length = (tree->global_offset + html_offset) - qnode->begin;
            myhtml_check_tag_parser(tree, qnode, html, html_offset);
            
            html_offset++;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SELF_CLOSING_START_TAG;
            
            break;
        }
        else if(html[html_offset] == '>')
        {
            qnode->length = (tree->global_offset + html_offset) - qnode->begin;
            
            myhtml_check_tag_parser(tree, qnode, html, html_offset);
            myhtml_tokenizer_set_state(tree, qnode);
            
            html_offset++;
            mh_queue_add(tree, html, html_offset, qnode);
            
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
size_t myhtml_tokenizer_state_self_closing_start_tag(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '>') {
        qnode->token->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
        myhtml_tokenizer_set_state(tree, qnode);
        
        html_offset++;
        mh_queue_add(tree, html, html_offset, qnode);
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, after tag name
//// <div%HERE% class="bla"></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_before_attribute_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    // skip WS
    myhtml_parser_skip_whitespace()
    
    if(html_offset >= html_size) {
        return html_offset;
    }
    
    if(html[html_offset] == '>')
    {
        myhtml_tokenizer_set_state(tree, qnode);
        
        html_offset++;
        mh_queue_add(tree, html, html_offset, qnode);
    }
    else if(html[html_offset] == '/') {
        qnode->token->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
        
        html_offset++;
    }
    else {
        myhtml_parser_queue_set_attr(tree, qnode)
        
        tree->attr_current->name_begin   = html_offset + tree->global_offset;
        tree->attr_current->name_length  = 0;
        tree->attr_current->value_begin  = 0;
        tree->attr_current->value_length = 0;
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, inside of attr key
//// <div cla%HERE%ss="bla"></div>
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_attribute_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            tree->attr_current->name_length = (tree->global_offset + html_offset) - tree->attr_current->name_begin;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_NAME;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '=')
        {
            tree->attr_current->name_length = (tree->global_offset + html_offset) - tree->attr_current->name_begin;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            tree->attr_current->name_length = (tree->global_offset + html_offset) - tree->attr_current->name_begin;
            myhtml_tokenizer_set_state(tree, qnode);
            
            html_offset++;
            
            mh_queue_add(tree, html, html_offset, qnode);
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            break;
        }
        else if(html[html_offset] == '/')
        {
            tree->attr_current->name_length = (tree->global_offset + html_offset) - tree->attr_current->name_begin;
            
            qnode->token->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
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
size_t myhtml_tokenizer_state_after_attribute_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '=')
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            myhtml_tokenizer_set_state(tree, qnode);
            
            html_offset++;
            
            mh_queue_add(tree, html, html_offset, qnode);
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            break;
        }
        else if(html[html_offset] == '"' || html[html_offset] == '\'' || html[html_offset] == '<')
        {
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            myhtml_parser_queue_set_attr(tree, qnode)
            
            tree->attr_current->name_begin   = (tree->global_offset + html_offset);
            tree->attr_current->name_length  = 0;
            tree->attr_current->value_begin  = 0;
            tree->attr_current->value_length = 0;
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME;
            break;
        }
        else if(myhtml_whithspace(html[html_offset], !=, &&))
        {
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            myhtml_parser_queue_set_attr(tree, qnode)
            
            tree->attr_current->name_begin   = (html_offset + tree->global_offset);
            tree->attr_current->name_length  = 0;
            tree->attr_current->value_begin  = 0;
            tree->attr_current->value_length = 0;
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME;
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
size_t myhtml_tokenizer_state_before_attribute_value(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>') {
            myhtml_tokenizer_set_state(tree, qnode);
            
            html_offset++;
            
            mh_queue_add(tree, html, html_offset, qnode);
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            break;
        }
        else if(myhtml_whithspace(html[html_offset], !=, &&))
        {
            if(html[html_offset] == '"') {
                html_offset++;
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED;
            }
            else if(html[html_offset] == '\'') {
                html_offset++;
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED;
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_UNQUOTED;
            }
            
            tree->attr_current->value_begin = (tree->global_offset + html_offset);
            
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
size_t myhtml_tokenizer_state_attribute_value_double_quoted(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '"')
        {
            tree->attr_current->value_length = (tree->global_offset + html_offset) - tree->attr_current->value_begin;
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
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
size_t myhtml_tokenizer_state_attribute_value_single_quoted(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '\'')
        {
            tree->attr_current->value_length = (tree->global_offset + html_offset) - tree->attr_current->value_begin;
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
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
size_t myhtml_tokenizer_state_attribute_value_unquoted(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->attr_current->value_length = (tree->global_offset + html_offset) - tree->attr_current->value_begin;
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>') {
            tree->attr_current->value_length = (tree->global_offset + html_offset) - tree->attr_current->value_begin;
            
            myhtml_tokenizer_set_state(tree, qnode);
            
            html_offset++;
            
            mh_queue_add(tree, html, html_offset, qnode);
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// COMMENT
//// <!--%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_comment_start(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    qnode->token->tag_ctx_idx = MyHTML_TAG__COMMENT;
    
    if(html[html_offset] == '-')
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_START_DASH;
    }
    else if(html[html_offset] == '>')
    {
        qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment_start_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    qnode->token->tag_ctx_idx = MyHTML_TAG__COMMENT;
    
    if(html[html_offset] == '-')
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END_DASH;
    }
    else if(html[html_offset] == '>')
    {
        qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    qnode->token->tag_ctx_idx = MyHTML_TAG__COMMENT;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '-')
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END_DASH;
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment_end_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '-')
    {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END;
    }
    else {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment_end(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '>')
    {
        qnode->length = ((tree->global_offset + html_offset) - qnode->begin);
        
        if(qnode->length >= 2)
            qnode->length -= 2;
        else
            qnode->length = 0;
        
        html_offset++;
        mh_queue_add(tree, html, html_offset, qnode);
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else if(html[html_offset] == '!') {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END_BANG;
    }
    else if(html[html_offset] == '-') {
        html_offset++;
    }
    else {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_comment_end_bang(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '>')
    {
        qnode->length = ((tree->global_offset + html_offset) - qnode->begin) - 3;
        
        html_offset++;
        mh_queue_add(tree, html, html_offset, qnode);
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else if(html[html_offset] == '-') {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT_END_DASH;
    }
    else {
        html_offset++;
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// BOGUS COMMENT
//// find >
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_bogus_comment(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    qnode->token->tag_ctx_idx = MyHTML_TAG__COMMENT;
    qnode->token->type |= MyHTML_TOKEN_TYPE_COMMENT;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>')
        {
            qnode->length = ((tree->global_offset + html_offset) - qnode->begin);
            
            html_offset++;
            mh_queue_add(tree, html, html_offset, qnode);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

myhtml_status_t myhtml_tokenizer_state_init(myhtml_t* myhtml)
{
    myhtml->parse_state_func = (myhtml_tokenizer_state_f*)mymalloc(sizeof(myhtml_tokenizer_state_f) *
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
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED] = myhtml_tokenizer_state_attribute_value_double_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED] = myhtml_tokenizer_state_attribute_value_single_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_UNQUOTED]      = myhtml_tokenizer_state_attribute_value_unquoted;
    
    // comments
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
    
    // for ends comments
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
    
    return MyHTML_STATUS_OK;
}

void myhtml_tokenizer_state_destroy(myhtml_t* myhtml)
{
    if(myhtml->parse_state_func)
        free(myhtml->parse_state_func);
}




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

#include "myhtml/tokenizer_script.h"


size_t myhtml_tokenizer_state_script_data(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while (html_offset < html_size)
    {
        if(html[html_offset] == '<') {
            html_offset++;
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_LESS_THAN_SIGN;
            
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '/')
    {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_OPEN;
    }
    else if(html[html_offset] == '!')
    {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPE_START;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escape_start(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '-') {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPE_START_DASH;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escape_start_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '-') {
        html_offset++;
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH_DASH;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_end_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(myhtml_ascii_char_cmp(html[html_offset])) {
        qnode->token->begin = (html_offset + tree->global_offset);
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_NAME;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            if((html_offset - qnode->token->begin) != 6) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
                html_offset++;
                break;
            }
            
            size_t tmp_size = qnode->begin;
            qnode->begin = qnode->token->begin;
            
            const char *tem_name = myhtml_tree_incomming_buffer_make_data(tree, qnode, 6);
            
            qnode->begin = tmp_size;
            
            if(myhtml_strncasecmp(tem_name, "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((html_offset + tree->global_offset) - 8), MyHTML_TOKEN_TYPE_SCRIPT);
                
                qnode->begin  = qnode->token->begin;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
            }
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '/')
        {
            if((html_offset - qnode->token->begin) != 6) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
                html_offset++;
                break;
            }
            
            size_t tmp_size = qnode->begin;
            qnode->begin = qnode->token->begin;
            
            const char *tem_name = myhtml_tree_incomming_buffer_make_data(tree, qnode, 6);
            
            qnode->begin = tmp_size;
            
            if(myhtml_strncasecmp(tem_name, "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((html_offset + tree->global_offset) - 8), MyHTML_TOKEN_TYPE_SCRIPT);
                
                qnode->begin  = qnode->token->begin;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE|MyHTML_TOKEN_TYPE_CLOSE_SELF;
                
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
            }
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            if((html_offset - qnode->token->begin) != 6) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
                html_offset++;
                break;
            }
            
            size_t tmp_size = qnode->begin;
            qnode->begin = qnode->token->begin;
            
            const char *tem_name = myhtml_tree_incomming_buffer_make_data(tree, qnode, 6);
            qnode->begin = tmp_size;
            
            if(myhtml_strncasecmp(tem_name, "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((html_offset + tree->global_offset) - 8), MyHTML_TOKEN_TYPE_SCRIPT);
                
                qnode->begin  = qnode->token->begin;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                
                html_offset++;
                myhtml_queue_add(tree, html, html_offset, qnode);
                
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
                html_offset++;
            }
            
            break;
        }
        else if(myhtml_ascii_char_unless_cmp(html[html_offset]))
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_dash_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '-') {
        html_offset++;
        return html_offset;
    }
    
    if(html[html_offset] == '<') {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN;
    }
    else if(html[html_offset] == '>') {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '/') {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_OPEN;
        html_offset++;
    }
    else if(myhtml_ascii_char_cmp(html[html_offset])) {
        qnode->token->begin = (html_offset + tree->global_offset);
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_START;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_end_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(myhtml_ascii_char_cmp(html[html_offset])) {
        qnode->token->begin = (html_offset + tree->global_offset);
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_NAME;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            if((html_offset - qnode->token->begin) != 6) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
                html_offset++;
                break;
            }
            
            size_t tmp_size = qnode->begin;
            qnode->begin = qnode->token->begin;
            
            const char *tem_name = myhtml_tree_incomming_buffer_make_data(tree, qnode, 6);
            
            qnode->begin = tmp_size;
            
            if(myhtml_strncasecmp(tem_name, "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((html_offset + tree->global_offset) - 8), MyHTML_TOKEN_TYPE_SCRIPT);
                
                qnode->begin  = qnode->token->begin;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            }
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '/')
        {
            if((html_offset - qnode->token->begin) != 6) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
                html_offset++;
                break;
            }
            
            size_t tmp_size = qnode->begin;
            qnode->begin = qnode->token->begin;
            
            const char *tem_name = myhtml_tree_incomming_buffer_make_data(tree, qnode, 6);
            
            qnode->begin = tmp_size;
            
            if(myhtml_strncasecmp(tem_name, "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((html_offset + tree->global_offset) - 8), MyHTML_TOKEN_TYPE_SCRIPT);
                
                qnode->begin  = qnode->token->begin;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE|MyHTML_TOKEN_TYPE_CLOSE_SELF;
                
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            }
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            if((html_offset - qnode->token->begin) != 6) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
                html_offset++;
                break;
            }
            
            size_t tmp_size = qnode->begin;
            qnode->begin = qnode->token->begin;
            
            const char *tem_name = myhtml_tree_incomming_buffer_make_data(tree, qnode, 6);
            
            qnode->begin = tmp_size;
            
            if(myhtml_strncasecmp(tem_name, "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, ((html_offset + tree->global_offset) - 8), MyHTML_TOKEN_TYPE_SCRIPT);
                
                qnode->begin  = qnode->token->begin;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
                
                html_offset++;
                myhtml_queue_add(tree, html, html_offset, qnode);
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
                html_offset++;
            }
            break;
        }
        else if(myhtml_ascii_char_unless_cmp(html[html_offset]))
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '-')
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH;
            html_offset++;
            break;
        }
        else if(html[html_offset] == '<')
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN;
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '-') {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH_DASH;
        html_offset++;
    }
    else if(html[html_offset] == '<') {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN;
    }
    else if(html[html_offset] == '\0') {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escape_start(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||) || html[html_offset] == '/' || html[html_offset] == '>')
        {
            if((html_offset - qnode->token->begin) != 6) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
                html_offset++;
                break;
            }
            
            size_t tmp_size = qnode->begin;
            qnode->begin = qnode->token->begin;
            
            const char *tem_name = myhtml_tree_incomming_buffer_make_data(tree, qnode, 6);
            
            qnode->begin = tmp_size;
            
            if(myhtml_strncasecmp(tem_name, "script", 6) == 0) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            }
            
            html_offset++;
            break;
        }
        else if(myhtml_ascii_char_unless_cmp(html[html_offset]))
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escaped(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '-')
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH;
            html_offset++;
            break;
        }
        else if(html[html_offset] == '<')
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN;
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escaped_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '-')
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH_DASH;
    }
    else if(html[html_offset] == '<')
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escaped_dash_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '-') {
        html_offset++;
        return html_offset;
    }
    
    if(html[html_offset] == '<')
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN;
    }
    else if(html[html_offset] == '>')
    {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escaped_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(html[html_offset] == '/') {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_END;
        html_offset++;
        
        qnode->token->begin = (html_offset + tree->global_offset);
    }
    else {
        myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escape_end(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||) || html[html_offset] == '/' || html[html_offset] == '>')
        {
            if((html_offset - qnode->token->begin) != 6) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
                html_offset++;
                break;
            }
            
            size_t tmp_size = qnode->begin;
            qnode->begin = qnode->token->begin;
            
            const char *tem_name = myhtml_tree_incomming_buffer_make_data(tree, qnode, 6);
            
            qnode->begin = tmp_size;
            
            if(myhtml_strncasecmp(tem_name, "script", 6) == 0) {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            }
            else {
                myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
            }
            
            html_offset++;
            break;
        }
        else if(myhtml_ascii_char_unless_cmp(html[html_offset]))
        {
            myhtml_tokenizer_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}





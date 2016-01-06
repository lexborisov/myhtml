/*
 Copyright 2015 Alexander Borisov
 
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

#include "tokenizer_script.h"


size_t myhtml_tokenizer_state_script_data(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while (html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            myhtml_tokenizer_inc_html_offset(html_offset, html_size);
            
            // skip state: script_data_less_than_sign, script_data_escate_start and script_data_escate_start_dash
            if(html[html_offset] == '/')
            {
                myhtml_tokenizer_inc_html_offset(html_offset, html_size);
                
                if(myhtml_ascii_char_cmp(html[html_offset]))
                {
                    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_NAME;
                    break;
                }
            }
            else if(html[html_offset] == '!')
            {
                myhtml_tokenizer_inc_html_offset(html_offset, html_size);
                
                size_t next_offset = html_offset + 1;
                
                if(next_offset < html_size)
                {
                    if(html[html_offset] == '-' && html[next_offset] == '-')
                    {
                        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH_DASH;
                        
                        html_offset = next_offset + 1;
                        break;
                    }
                }
            }
        }
        
        html_offset++;
    }
    
    return html_offset;
}

// see myhtml_tokenizer_state_script_data
size_t myhtml_tokenizer_state_script_data_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    return html_offset;
}

// see myhtml_tokenizer_state_script_data
size_t myhtml_tokenizer_state_script_data_escape_start(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    return html_offset;
}

// see myhtml_tokenizer_state_script_data
size_t myhtml_tokenizer_state_script_data_escape_start_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    return html_offset;
}

// see myhtml_tokenizer_state_script_data
size_t myhtml_tokenizer_state_script_data_end_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    if(myhtml_ascii_char_cmp(html[html_offset])) {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_NAME;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    size_t offset_cache = html_offset;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            if(strncasecmp(&html[offset_cache], "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, offset_cache - 2);
                
                qnode->begin = offset_cache;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
            }
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '/')
        {
            if(strncasecmp(&html[offset_cache], "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, offset_cache - 2);
                
                qnode->begin = offset_cache;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE|MyHTML_TOKEN_TYPE_CLOSE_SELF;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
            }
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            if(strncasecmp(&html[offset_cache], "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, offset_cache - 2);
                
                qnode->begin = offset_cache;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                
                html_offset++;
                mh_queue_add(tree, html, html_offset);
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
                html_offset++;
            }
            
            break;
        }
        else if(myhtml_ascii_char_unless_cmp(html[html_offset]))
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_dash_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    if(html[html_offset] == '-') {
        html_offset++;
        return html_offset;
    }
    
    if(html[html_offset] == '<') {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN;
    }
    else if(html[html_offset] == '>') {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    if(html[html_offset] == '/') {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_OPEN;
        html_offset++;
    }
    else if(myhtml_ascii_char_cmp(html[html_offset])) {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_START;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_end_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    if(myhtml_ascii_char_cmp(html[html_offset])) {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_NAME;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    size_t offset_cache = html_offset;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            if(strncasecmp(&html[offset_cache], "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, offset_cache - 2);
                
                qnode->begin = offset_cache;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            }
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '/')
        {
            if(strncasecmp(&html[offset_cache], "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, offset_cache - 2);
                
                qnode->begin = offset_cache;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE|MyHTML_TOKEN_TYPE_CLOSE_SELF;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            }
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            if(strncasecmp(&html[offset_cache], "script", 6) == 0)
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, offset_cache - 2);
                
                qnode->begin = offset_cache;
                qnode->length = 6;
                qnode->token->tag_ctx_idx = MyHTML_TAG_SCRIPT;
                qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
                
                html_offset++;
                mh_queue_add(tree, html, html_offset);
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
                html_offset++;
            }
            
            break;
        }
        else if(myhtml_ascii_char_unless_cmp(html[html_offset]))
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '-')
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH;
            html_offset++;
            break;
        }
        else if(html[html_offset] == '<')
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN;
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_escaped_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    if(html[html_offset] == '-') {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH_DASH;
        html_offset++;
    }
    else if(html[html_offset] == '<') {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escape_start(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    size_t offset_cache = html_offset;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||) || html[html_offset] == '/' || html[html_offset] == '>')
        {
            if((html_offset - offset_cache) > 5 && strncasecmp(&html[offset_cache], "script", 6) == 0) {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            }
            
            html_offset++;
            break;
        }
        else if(myhtml_ascii_char_unless_cmp(html[html_offset]))
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escaped(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '-')
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH;
            html_offset++;
            break;
        }
        else if(html[html_offset] == '<')
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN;
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escaped_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    if(html[html_offset] == '-')
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH_DASH;
    }
    else if(html[html_offset] == '<')
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escaped_dash_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    if(html[html_offset] == '-') {
        html_offset++;
        return html_offset;
    }
    
    if(html[html_offset] == '<')
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN;
    }
    else if(html[html_offset] == '>')
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
    }
    
    html_offset++;
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escaped_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    if(html[html_offset] == '/') {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_END;
        html_offset++;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_script_data_double_escape_end(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    size_t offset_cache = html_offset;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||) || html[html_offset] == '/' || html[html_offset] == '>')
        {
            if((html_offset - offset_cache) > 5 && strncasecmp(&html[offset_cache], "script", 6) == 0) {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED;
            }
            else {
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
            }
            
            html_offset++;
            break;
        }
        else if(myhtml_ascii_char_unless_cmp(html[html_offset]))
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}



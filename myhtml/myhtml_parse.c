//
//  myhtml_parse.c
//  myhtml
//
//  Created by Alexander Borisov on 30.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_parse.h"

void myhtml_check_tag_parser(myhtml_tree_t* tree, const char* html, size_t* html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    mytags_t* tags = myhtml->tags;
    mctree_t* tags_tree = tags->tree;
    
    size_t tagname_begin = mh_tree_token_current(tagname_begin);
    
    // get tag id by name
    mctree_index_t idx = mctree_search_lowercase(tags_tree, &html[tagname_begin], mh_tree_token_current(tagname_length));
    
    if(idx) {
        mh_tree_token_current(tag_ctx_idx) = (mytags_ctx_index_t)(tags_tree->nodes[idx].value);
    }
    else {
        mh_tree_token_current(tag_ctx_idx) = mytags_add(tags, &html[tagname_begin], mh_tree_token_current(tagname_length), MyHTML_PARSE_STATE_DATA);
    }
}

////
void myhtml_parse_queue_create_text_node_if_need(myhtml_tree_t* tree, const char* html, size_t html_offset)
{
    myhtml_t* myhtml = tree->myhtml;
    
    if(mh_tree_token_current(tag_ctx_idx) == MyTAGS_TAG__TEXT)
    {
        size_t tag_begin = mh_tree_token_current(tag_begin);
        if(html_offset > tag_begin)
        {
            mh_tree_token_current(tag_length) = html_offset - mh_tree_token_current(tag_begin);
            mh_queue_add(tree, html, mh_queue_current(), 0);
        }
    }
}

/////////////////////////////////////////////////////////
//// RCDATA
////
/////////////////////////////////////////////////////////
size_t myhtml_parse_state_rcdata(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    if(tree->tmp_tag_id == 0) {
        tree->tmp_tag_id = mh_queue_token_get((mh_queue_current() - 1), tag_ctx_idx);
    }
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            myhtml_parse_inc_html_offset(html_offset, html_size);
            
            if(html[html_offset] == '/')
            {
                myhtml_parse_inc_html_offset(html_offset, html_size);
                
                if(myhtml_ascii_char_cmp(html[html_offset]))
                {
                    myhtml_parse_queue_create_text_node_if_need(tree, html, html_offset - 2);
                    
                    mh_tree_token_current(tag_begin) = html_offset - 2;
                    mh_tree_token_current(tagname_begin) = html_offset;
                    mh_tree_token_current(type) = MyHTML_TOKEN_TYPE_CLOSE;
                    
                    mh_state_set(tree) = MyHTML_PARSE_STATE_RCDATA_END_TAG_NAME;
                    break;
                }
            }
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_parse_state_rcdata_end_tag_name(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            mh_tree_token_current(tagname_length) = html_offset - mh_tree_token_current(tagname_begin);
            myhtml_check_tag_parser(tree, html, &html_offset, html_size);
            
            if(mh_tree_token_current(tag_ctx_idx) != tree->tmp_tag_id)
            {
                mh_tree_token_current(tag_begin)      = 0;
                mh_tree_token_current(tagname_begin)  = 0;
                mh_tree_token_current(tagname_length) = 0;
                
                mh_state_set(tree) = MyHTML_PARSE_STATE_RCDATA;
                
                html_offset++;
                break;
            }
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->tmp_tag_id = 0;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            mh_tree_token_current(tagname_length) = html_offset - mh_tree_token_current(tagname_begin);
            myhtml_check_tag_parser(tree, html, &html_offset, html_size);
            
            if(mh_tree_token_current(tag_ctx_idx) != tree->tmp_tag_id)
            {
                mh_tree_token_current(tag_begin)      = 0;
                mh_tree_token_current(tagname_begin)  = 0;
                mh_tree_token_current(tagname_length) = 0;
                
                mh_state_set(tree) = MyHTML_PARSE_STATE_RCDATA;
                
                html_offset++;
                break;
            }
            
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            
            html_offset++;
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            
            tree->tmp_tag_id = 0;
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_DATA;
            break;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            mh_tree_token_current(tagname_length) = html_offset - mh_tree_token_current(tagname_begin);
            myhtml_check_tag_parser(tree, html, &html_offset, html_size);
            
            if(mh_tree_token_current(tag_ctx_idx) != tree->tmp_tag_id)
            {
                mh_tree_token_current(tag_begin)      = 0;
                mh_tree_token_current(tagname_begin)  = 0;
                mh_tree_token_current(tagname_length) = 0;
                
                mh_state_set(tree) = MyHTML_PARSE_STATE_RCDATA;
                
                html_offset++;
                break;
            }
            
            mh_tree_token_current(type) |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->tmp_tag_id = 0;
            
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// RAWTEXT
////
/////////////////////////////////////////////////////////
size_t myhtml_parse_state_rawtext(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    if(tree->tmp_tag_id == 0) {
        tree->tmp_tag_id = mh_queue_token_get((mh_queue_current() - 1), tag_ctx_idx);
    }
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            myhtml_parse_inc_html_offset(html_offset, html_size);
            
            if(html[html_offset] == '/')
            {
                myhtml_parse_inc_html_offset(html_offset, html_size);
                
                if(myhtml_ascii_char_cmp(html[html_offset]))
                {
                    myhtml_parse_queue_create_text_node_if_need(tree, html, html_offset - 2);
                    
                    mh_tree_token_current(tag_begin) = html_offset - 2;
                    mh_tree_token_current(tagname_begin) = html_offset;
                    mh_tree_token_current(type) = MyHTML_TOKEN_TYPE_CLOSE;
                    
                    mh_state_set(tree) = MyHTML_PARSE_STATE_RAWTEXT_END_TAG_NAME;
                    break;
                }
            }
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_parse_state_rawtext_end_tag_name(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            mh_tree_token_current(tagname_length) = html_offset - mh_tree_token_current(tagname_begin);
            myhtml_check_tag_parser(tree, html, &html_offset, html_size);
            
            if(mh_tree_token_current(tag_ctx_idx) != tree->tmp_tag_id)
            {
                mh_tree_token_current(tag_begin)      = 0;
                mh_tree_token_current(tagname_begin)  = 0;
                mh_tree_token_current(tagname_length) = 0;
                
                mh_state_set(tree) = MyHTML_PARSE_STATE_RAWTEXT;
                
                html_offset++;
                break;
            }
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->tmp_tag_id = 0;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            mh_tree_token_current(tagname_length) = html_offset - mh_tree_token_current(tagname_begin);
            myhtml_check_tag_parser(tree, html, &html_offset, html_size);
            
            if(mh_tree_token_current(tag_ctx_idx) != tree->tmp_tag_id)
            {
                mh_tree_token_current(tag_begin)      = 0;
                mh_tree_token_current(tagname_begin)  = 0;
                mh_tree_token_current(tagname_length) = 0;
                
                mh_state_set(tree) = MyHTML_PARSE_STATE_RAWTEXT;
                
                html_offset++;
                break;
            }
            
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            
            html_offset++;
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            
            tree->tmp_tag_id = 0;
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_DATA;
            break;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            mh_tree_token_current(tagname_length) = html_offset - mh_tree_token_current(tagname_begin);
            myhtml_check_tag_parser(tree, html, &html_offset, html_size);
            
            if(mh_tree_token_current(tag_ctx_idx) != tree->tmp_tag_id)
            {
                mh_tree_token_current(tag_begin)      = 0;
                mh_tree_token_current(tagname_begin)  = 0;
                mh_tree_token_current(tagname_length) = 0;
                
                mh_state_set(tree) = MyHTML_PARSE_STATE_RAWTEXT;
                
                html_offset++;
                break;
            }
            
            mh_tree_token_current(type) |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->tmp_tag_id = 0;
            
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// PLAINTEXT
////
/////////////////////////////////////////////////////////
size_t myhtml_parse_state_plaintext(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    mh_tree_token_current(tag_begin) = html_offset;
    mh_tree_token_current(tag_length) = html_size - html_offset;
    mh_state_set(tree) = MyHTML_PARSE_STATE_DATA;
    
    mh_queue_add(tree, html, mh_queue_current(), 0);
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// CDATA
////
/////////////////////////////////////////////////////////
size_t myhtml_parse_state_cdata_section(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>' && html[html_offset - 1] == ']' && html[html_offset - 2] == ']')
        {
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            
            html_offset++;
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// outside of tag
//// %HERE%<div>%HERE%</div>%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_parse_state_data(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            myhtml_parse_inc_html_offset(html_offset, html_size);
            
            // close tag
            if(html[html_offset] == '/')
            {
                myhtml_parse_queue_create_text_node_if_need(tree, html, html_offset - 1);
                
                myhtml_parse_inc_html_offset(html_offset, html_size);
                
                if(myhtml_ascii_char_cmp(html[html_offset])) {
                    mh_state_set(tree) = MyHTML_PARSE_STATE_TAG_OPEN;
                    
                    mh_tree_token_current(type) = MyHTML_TOKEN_TYPE_CLOSE;
                    mh_tree_token_current(tag_begin) = html_offset - 2;
                    mh_tree_token_current(tagname_begin) = html_offset;
                    
                    break;
                }
                else {
                    mh_tree_token_current(tag_begin)      = html_offset - 2;
                    mh_tree_token_current(tagname_begin)  = 0;
                    mh_tree_token_current(tagname_length) = 0;
                    mh_tree_token_current(tag_ctx_idx)    = MyTAGS_TAG__COMMENT;
                    
                    mh_state_set(tree) = MyHTML_PARSE_STATE_BOGUS_COMMENT;
                }
            }
            else if(html[html_offset] == '!')
            {
                myhtml_parse_queue_create_text_node_if_need(tree, html, html_offset - 1);
                
                myhtml_parse_inc_html_offset(html_offset, html_size);
                
                mh_tree_token_current(tag_begin) = html_offset - 2;
                
                size_t html_offset_n;
                
                // for a comment
                {
                    html_offset_n = html_offset + 1;
                    
                    if(html_offset_n >= html_size)
                        return html_offset_n; // parse exit
                    
                    if(html[html_offset] == '-' && html[html_offset_n] == '-')
                    {
                        mh_state_set(tree) = MyHTML_PARSE_STATE_COMMENT;
                        
                        mh_tree_token_current(tagname_begin)  = html_offset;
                        mh_tree_token_current(tagname_length) = 2;
                        mh_tree_token_current(tag_ctx_idx)    = MyTAGS_TAG__COMMENT;
                        
                        html_offset = html_offset_n + 1;
                        break;
                    }
                }
                
                // for a CDATA; CDATA sections can only be used in foreign content (MathML or SVG)
//                {
//                    html_offset_n = html_offset + 7;
//                    
//                    if(html_offset_n >= html_size)
//                        return html_offset_n; // parse exit
//                    
//                    if(strncmp(&html[html_offset], "[CDATA[", 7) == 0)
//                    {
//                        mh_state_set(tree) = MyHTML_PARSE_STATE_CDATA_SECTION;
//                        
//                        mh_tree_token_current(tagname_begin)  = html_offset;
//                        mh_tree_token_current(tagname_length) = 7;
//                        mh_tree_token_current(tag_ctx_idx)    = MyTAGS_TAG__COMMENT;
//                        
//                        html_offset = html_offset_n + 1;
//                        break;
//                    }
//                }
                
                // for a DOCTYPE
                {
                    html_offset_n = html_offset + 7;
                    
                    if(html_offset_n >= html_size)
                        return html_offset_n; // parse exit
                    
                    if(strncasecmp(&html[html_offset], "DOCTYPE", 7) == 0)
                    {
                        mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_DOCTYPE_NAME;
                        
                        mh_tree_token_current(tagname_begin)  = html_offset;
                        mh_tree_token_current(tagname_length) = 7;
                        mh_tree_token_current(tag_ctx_idx)    = MyTAGS_TAG__DOCTYPE;
                        
                        html_offset = html_offset_n + 1;
                        break;
                    }
                }
                
                mh_tree_token_current(tagname_begin)  = 0;
                mh_tree_token_current(tagname_length) = 0;
                mh_tree_token_current(tag_ctx_idx)    = MyTAGS_TAG__COMMENT;
                
                mh_state_set(tree) = MyHTML_PARSE_STATE_BOGUS_COMMENT;
                break;
            }
            else if(html[html_offset] == '?')
            {
                myhtml_parse_queue_create_text_node_if_need(tree, html, html_offset - 1);
                
                myhtml_parse_inc_html_offset(html_offset, html_size);
                
                mh_tree_token_current(tagname_begin)  = 0;
                mh_tree_token_current(tagname_length) = 0;
                mh_tree_token_current(tag_begin)      = html_offset - 2;
                
                mh_state_set(tree) = MyHTML_PARSE_STATE_BOGUS_COMMENT;
                
                break;
            }
            else if(myhtml_ascii_char_cmp(html[html_offset]))
            {
                myhtml_parse_queue_create_text_node_if_need(tree, html, html_offset - 1);
                
                mh_state_set(tree) = MyHTML_PARSE_STATE_TAG_OPEN;
                mh_tree_token_current(tag_begin)     = html_offset - 1;
                mh_tree_token_current(tagname_begin) = html_offset;
                
                html_offset++;
                break;
            }
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag
//// <%HERE%div></div>
/////////////////////////////////////////////////////////
size_t myhtml_parse_state_tag_open(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size) {
        // check end of tag name
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
            mh_tree_token_current(tagname_length) = html_offset - mh_tree_token_current(tagname_begin);
            myhtml_check_tag_parser(tree, html, &html_offset, html_size);
            
            html_offset++;
            break;
        }
        // check end of tag
        else if(html[html_offset] == '>')
        {
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            mh_tree_token_current(tagname_length) = html_offset - mh_tree_token_current(tagname_begin);
            
            myhtml_check_tag_parser(tree, html, &html_offset, html_size);
            
            mh_state_set(tree) = mh_tags_get(mh_tree_token_current(tag_ctx_idx), data_parser);
            
            html_offset++;
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            
            break;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            mh_tree_token_current(type)          |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
            mh_tree_token_current(tagname_length) = html_offset - mh_tree_token_current(tagname_begin);
            
            myhtml_check_tag_parser(tree, html, &html_offset, html_size);
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, after tag name
//// <div%HERE% class="bla"></div>
/////////////////////////////////////////////////////////
size_t myhtml_parse_state_before_attribute_name(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    // skip WS
    myhtml_parser_skip_whitespace()
    
    if(html_offset >= html_size) {
        mh_tree_token_current(tag_length) = html_offset - mh_tree_token_current(tag_begin);
        // move to thread for parsing tag
        return html_offset;
    }
    
    if(html[html_offset] == '>') {
        mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
        
        mh_state_set(tree) = mh_tags_get(mh_tree_token_current(tag_ctx_idx), data_parser);
        
        html_offset++;
        mh_queue_add(tree, html, mh_queue_current(), html_offset);
    }
    else if(html[html_offset] == '/') {
        mh_tree_token_current(type) |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
        
        html_offset++;
    }
    else {
        myhtml_parser_queue_set_attr();
        
        size_t attr_id = mh_tree_token_current(attr_last);
        
        mh_token_attr_set(attr_id, name_begin) = html_offset;
        mh_state_set(tree) = MyHTML_PARSE_STATE_ATTRIBUTE_NAME;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// inside of tag, inside of attr key
//// <div cla%HERE%ss="bla"></div>
/////////////////////////////////////////////////////////
size_t myhtml_parse_state_attribute_name(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            size_t attr_id = mh_tree_token_current(attr_last);
            
            mh_token_attr_set(attr_id, name_length) = html_offset - mh_token_attr_get(attr_id, name_begin);
            mh_state_set(tree) = MyHTML_PARSE_STATE_AFTER_ATTRIBUTE_NAME;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '=')
        {
            size_t attr_id = mh_tree_token_current(attr_last);
            
            mh_token_attr_set(attr_id, name_length) = html_offset - mh_token_attr_get(attr_id, name_begin);
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_VALUE;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            size_t attr_id = mh_tree_token_current(attr_last);
            
            mh_token_attr_set(attr_id, name_length) = html_offset - mh_token_attr_get(attr_id, name_begin);
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            
            mh_state_set(tree) = mh_tags_get(mh_tree_token_current(tag_ctx_idx), data_parser);
            
            html_offset++;
            
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
            break;
        }
        else if(html[html_offset] == '/')
        {
            size_t attr_id = mh_tree_token_current(attr_last);
            
            mh_token_attr_set(attr_id, name_length) = html_offset - mh_token_attr_get(attr_id, name_begin);
            
            mh_tree_token_current(type) |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
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
size_t myhtml_parse_state_after_attribute_name(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '=')
        {
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_VALUE;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            
            mh_state_set(tree) = mh_tags_get(mh_tree_token_current(tag_ctx_idx), data_parser);
            
            html_offset++;
            
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
            break;
        }
        else if(html[html_offset] == '"' || html[html_offset] == '\'' || html[html_offset] == '<')
        {
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
            size_t attr_id = mh_tree_token_current(attr_last);
            mh_token_attr_set(attr_id, name_begin) = html_offset;
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_ATTRIBUTE_NAME;
            break;
        }
        else if(myhtml_whithspace(html[html_offset], !=, &&))
        {
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
            size_t attr_id = mh_tree_token_current(attr_last);
            mh_token_attr_set(attr_id, name_begin) = html_offset;
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_ATTRIBUTE_NAME;
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
size_t myhtml_parse_state_before_attribute_value(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>') {
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            
            mh_state_set(tree) = mh_tags_get(mh_tree_token_current(tag_ctx_idx), data_parser);
            
            html_offset++;
            
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
            break;
        }
        else if(myhtml_whithspace(html[html_offset], !=, &&))
        {
            if(html[html_offset] == '"') {
                html_offset++;
                mh_state_set(tree) = MyHTML_PARSE_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED;
            }
            else if(html[html_offset] == '\'') {
                html_offset++;
                mh_state_set(tree) = MyHTML_PARSE_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED;
            }
            else {
                mh_state_set(tree) = MyHTML_PARSE_STATE_ATTRIBUTE_VALUE_UNQUOTED;
            }
            
            size_t attr_id = mh_tree_token_current(attr_last);
            mh_token_attr_set(attr_id, value_begin) = html_offset;
            
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
size_t myhtml_parse_state_attribute_value_double_quoted(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '"')
        {
            size_t attr_id = mh_tree_token_current(attr_last);
            mh_token_attr_set(attr_id, value_length) = html_offset - mh_token_attr_get(attr_id, value_begin);
            
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
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
size_t myhtml_parse_state_attribute_value_single_quoted(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '\'')
        {
            size_t attr_id = mh_tree_token_current(attr_last);
            mh_token_attr_set(attr_id, value_length) = html_offset - mh_token_attr_get(attr_id, value_begin);
            
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
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
size_t myhtml_parse_state_attribute_value_unquoted(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            mh_state_set(tree) = MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME;
            
            size_t attr_id = mh_tree_token_current(attr_last);
            mh_token_attr_set(attr_id, value_length) = html_offset - mh_token_attr_get(attr_id, value_begin);
            
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>') {
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            
            size_t attr_id = mh_tree_token_current(attr_last);
            mh_token_attr_set(attr_id, value_length) = html_offset - mh_token_attr_get(attr_id, value_begin);
            
            mh_state_set(tree) = mh_tags_get(mh_tree_token_current(tag_ctx_idx), data_parser);
            
            html_offset++;
            
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            myhtml_token_attr_malloc(tree->token, mh_tree_token_attr_current_index());
            
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
size_t myhtml_parse_state_comment(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>' && html[html_offset - 1] == '-' && html[html_offset - 2] == '-')
        {
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            
            html_offset++;
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// BOGUS COMMENT
//// find >
/////////////////////////////////////////////////////////
size_t myhtml_parse_state_bogus_comment(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>')
        {
            mh_tree_token_current(tag_length) = (html_offset - mh_tree_token_current(tag_begin)) + 1;
            
            html_offset++;
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            
            mh_state_set(tree) = MyHTML_PARSE_STATE_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

void myhtml_parse_state_init(myhtml_t* myhtml)
{
    myhtml->parse_state_func = (myhtml_parse_state_f*)malloc(sizeof(myhtml_parse_state_f) * MyHTML_PARSE_STATE_LAST_ENTRY);
    
    myhtml->parse_state_func[MyHTML_PARSE_STATE_DATA]                          = myhtml_parse_state_data;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_TAG_OPEN]                      = myhtml_parse_state_tag_open;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME]         = myhtml_parse_state_before_attribute_name;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_ATTRIBUTE_NAME]                = myhtml_parse_state_attribute_name;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_AFTER_ATTRIBUTE_NAME]          = myhtml_parse_state_after_attribute_name;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_VALUE]        = myhtml_parse_state_before_attribute_value;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED] = myhtml_parse_state_attribute_value_double_quoted;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED] = myhtml_parse_state_attribute_value_single_quoted;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_ATTRIBUTE_VALUE_UNQUOTED]      = myhtml_parse_state_attribute_value_unquoted;
    
    // comments
    myhtml->parse_state_func[MyHTML_PARSE_STATE_COMMENT]                       = myhtml_parse_state_comment;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_BOGUS_COMMENT]                 = myhtml_parse_state_bogus_comment;
    
    // cdata
    myhtml->parse_state_func[MyHTML_PARSE_STATE_CDATA_SECTION]                 = myhtml_parse_state_cdata_section;
    
    // rcdata
    myhtml->parse_state_func[MyHTML_PARSE_STATE_RCDATA]                        = myhtml_parse_state_rcdata;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_RCDATA_END_TAG_NAME]           = myhtml_parse_state_rcdata_end_tag_name;
    
    // rawtext
    myhtml->parse_state_func[MyHTML_PARSE_STATE_RAWTEXT]                       = myhtml_parse_state_rawtext;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_RAWTEXT_END_TAG_NAME]          = myhtml_parse_state_rawtext_end_tag_name;
    
    // plaintext
    myhtml->parse_state_func[MyHTML_PARSE_STATE_PLAINTEXT]                     = myhtml_parse_state_plaintext;
    
    // doctype
    myhtml->parse_state_func[MyHTML_PARSE_STATE_BEFORE_DOCTYPE_NAME]                     = myhtml_parse_state_before_doctype_name;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_DOCTYPE_NAME]                            = myhtml_parse_state_doctype_name;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_AFTER_DOCTYPE_NAME]                      = myhtml_parse_state_after_doctype_name;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_BEFORE_DOCTYPE_PUBLIC_IDENTIFIER]        = myhtml_parse_state_before_doctype_public_identifier;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED] = myhtml_parse_state_doctype_public_identifier_double_quoted;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED] = myhtml_parse_state_doctype_public_identifier_single_quoted;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_AFTER_DOCTYPE_PUBLIC_IDENTIFIER]         = myhtml_parse_state_after_doctype_public_identifier;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED] = myhtml_parse_state_doctype_system_identifier_double_quoted;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED] = myhtml_parse_state_doctype_system_identifier_single_quoted;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_AFTER_DOCTYPE_SYSTEM_IDENTIFIER]         = myhtml_parse_state_after_doctype_system_identifier;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_BOGUS_DOCTYPE]                           = myhtml_parse_state_bogus_doctype;
    
    // script
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA]                               = myhtml_parse_state_script_data;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_LESS_THAN_SIGN]                = myhtml_parse_state_script_data_less_than_sign;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_END_TAG_OPEN]                  = myhtml_parse_state_script_data_end_tag_open;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_END_TAG_NAME]                  = myhtml_parse_state_script_data_end_tag_name;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPE_START]                  = myhtml_parse_state_script_data_escape_start;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPE_START_DASH]             = myhtml_parse_state_script_data_escape_start_dash;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED]                       = myhtml_parse_state_script_data_escaped;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_DASH]                  = myhtml_parse_state_script_data_escaped_dash;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_DASH_DASH]             = myhtml_parse_state_script_data_escaped_dash_dash;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN]        = myhtml_parse_state_script_data_escaped_less_than_sign;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_END_TAG_OPEN]          = myhtml_parse_state_script_data_escaped_end_tag_open;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_END_TAG_NAME]          = myhtml_parse_state_script_data_escaped_end_tag_name;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_START]           = myhtml_parse_state_script_data_double_escape_start;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPED]                = myhtml_parse_state_script_data_double_escaped;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH]           = myhtml_parse_state_script_data_double_escaped_dash;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH_DASH]      = myhtml_parse_state_script_data_double_escaped_dash_dash;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN] = myhtml_parse_state_script_data_double_escaped_less_than_sign;
    myhtml->parse_state_func[MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_END]             = myhtml_parse_state_script_data_double_escape_end;

}

void myhtml_parse_state_destroy(myhtml_t* myhtml)
{
    if(myhtml->parse_state_func)
        free(myhtml->parse_state_func);
}






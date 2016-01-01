//
//  myhtml_parse.c
//  myhtml
//
//  Created by Alexander Borisov on 30.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#include "tokenizer.h"

void myhtml_tokenizer_begin(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    myhtml_t* myhtml = tree->myhtml;
    tree->current_qnode = mythread_queue_get_current_node(myhtml->queue);
    
    tree->current_qnode->text = html;
    tree->current_qnode->tree = tree;
    
    myhtml_token_node_malloc(tree->token, tree->current_qnode->token, tree->mcasync_token_id);
    
    myhtml_tokenizer_continue(tree, html, html_length);
}

void myhtml_tokenizer_end(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    if(tree->current_qnode && tree->current_qnode->token) {
        tree->current_qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, tree->current_qnode, html, html_length);
    }
    
    if(tree->is_single == myfalse) {
        myhtml_tokenizer_wait(tree);
    }
    
#ifdef DEBUG_MODE
    if(tree->open_elements->length) {
        MyHTML_DEBUG_ERROR("Tokenizer end; Open Elements is %lu", tree->open_elements->length);
    }
    if(tree->active_formatting->length) {
        MyHTML_DEBUG_ERROR("Tokenizer end; Active Formatting Elements is %lu", tree->active_formatting->length);
    }
#endif
}

void myhtml_tokenizer_continue(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    myhtml_t* myhtml = tree->myhtml;
    myhtml_tokenizer_state_f* state_f = myhtml->parse_state_func;
    
    if(tree->is_single == myfalse)
        myhtml_tokenizer_post(tree);
    
    size_t offset = 0;
    
    while (offset < html_length) {
        offset = state_f[tree->state](tree, tree->current_qnode, html, offset, html_length);
    }
}

myhtml_tree_node_t * myhtml_tokenizer_fragment_init(myhtml_tree_t* tree, mytags_ctx_index_t tag_idx, enum myhtml_namespace my_namespace)
{
    // step 3
    tree->fragment = myhtml_tree_node_create(tree);
    tree->fragment->namespace = my_namespace;
    tree->fragment->tag_idx = tag_idx;
    
    // skip step 4, is already done
    
    // step 5-7
    myhtml_tree_node_t* root = myhtml_tree_node_insert_root(tree, NULL, my_namespace);
    
    if(tag_idx == MyTAGS_TAG_TEMPLATE)
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
    
    mythread_resume_all(myhtml->thread);
}

void myhtml_check_tag_parser(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t* html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    mytags_t* tags = myhtml->tags;
    mctree_t* tags_tree = tags->tree;
    
    size_t tagname_begin = qnode->begin;
    
    // get tag id by name
    mctree_index_t idx = mctree_search_lowercase(tags_tree, &html[tagname_begin], qnode->length);
    
    if(idx) {
        qnode->token->tag_ctx_idx = (mytags_ctx_index_t)(tags_tree->nodes[idx].value);
        
        // parser is multi-threaded, and the specification requires the definition namespace for parsing cdata sections
        // but namespace we see after build tree, that is unacceptable to us
        // so this code here
        //
        // if there is an adjusted current node and it is not an element in the HTML namespace and
        // the next seven characters are a case-sensitive match for the string "[CDATA["
        // (the five uppercase letters "CDATA" with a U+005B LEFT SQUARE BRACKET character before and after),
        // then consume those characters and switch to the CDATA section state.
        if(tree->namespace != MyHTML_NAMESPACE_HTML)
        {
            if(tags->context[ qnode->token->tag_ctx_idx ].cats[ tree->namespace ] == MyTAGS_CATEGORIES_UNDEF)
            {
                if(qnode->token->tag_ctx_idx == MyTAGS_TAG_MATH)
                    tree->namespace = MyHTML_NAMESPACE_MATHML;
                else if(qnode->token->tag_ctx_idx == MyTAGS_TAG_SVG)
                    tree->namespace = MyHTML_NAMESPACE_SVG;
                else
                    tree->namespace = MyHTML_NAMESPACE_HTML;
            }
        }
        else if(qnode->token->tag_ctx_idx == MyTAGS_TAG_MATH)
                tree->namespace = MyHTML_NAMESPACE_MATHML;
        else if(qnode->token->tag_ctx_idx == MyTAGS_TAG_SVG)
            tree->namespace = MyHTML_NAMESPACE_SVG;
    }
    else {
        qnode->token->tag_ctx_idx = mytags_add(tags, &html[tagname_begin], qnode->length, MyHTML_TOKENIZER_STATE_DATA);
        mytags_set_category(tags, qnode->token->tag_ctx_idx, tree->namespace, MyTAGS_CATEGORIES_ORDINARY);
    }
}

////
mythread_queue_node_t * myhtml_tokenizer_queue_create_text_node_if_need(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset)
{
    if(qnode->token->tag_ctx_idx == MyTAGS_TAG__UNDEF)
    {
        if(html_offset > qnode->begin)
        {
            qnode->token->tag_ctx_idx = MyTAGS_TAG__TEXT;
            qnode->length = html_offset - qnode->begin;
            mh_queue_add(tree, html, 0);
            
            return tree->current_qnode;
        }
    }
    
    return qnode;
}

/////////////////////////////////////////////////////////
//// RCDATA
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_rcdata(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(tree->tmp_qnode == NULL) {
        tree->tmp_qnode = mythread_queue_get_prev_node(tree->myhtml->queue);
    }
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            myhtml_tokenizer_inc_html_offset(html_offset, html_size);
            
            if(html[html_offset] == '/')
            {
                myhtml_tokenizer_inc_html_offset(html_offset, html_size);
                
                if(myhtml_ascii_char_cmp(html[html_offset]))
                {
                    qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, html_offset - 2);
                    
                    qnode->begin = html_offset;
                    qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                    
                    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_NAME;
                    break;
                }
            }
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rcdata_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            qnode->length = html_offset - qnode->begin;
            myhtml_check_tag_parser(tree, qnode, html, &html_offset, html_size);
            
            if(qnode->token->tag_ctx_idx != tree->tmp_qnode->token->tag_ctx_idx)
            {
                qnode->begin  = 0;
                qnode->length = 0;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
                
                html_offset++;
                break;
            }
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->tmp_qnode = NULL;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            qnode->length = html_offset - qnode->begin;
            myhtml_check_tag_parser(tree, qnode, html, &html_offset, html_size);
            
            if(qnode->token->tag_ctx_idx != tree->tmp_qnode->token->tag_ctx_idx)
            {
                qnode->begin  = 0;
                qnode->length = 0;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
                
                html_offset++;
                break;
            }
            
            html_offset++;
            mh_queue_add(tree, html, html_offset);
            
            tree->tmp_qnode = NULL;
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            break;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            qnode->length = html_offset - qnode->begin;
            myhtml_check_tag_parser(tree, qnode, html, &html_offset, html_size);
            
            if(qnode->token->tag_ctx_idx != tree->tmp_qnode->token->tag_ctx_idx)
            {
                qnode->begin  = 0;
                qnode->length = 0;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RCDATA;
                
                html_offset++;
                break;
            }
            
            qnode->token->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->tmp_qnode = NULL;
            
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
size_t myhtml_tokenizer_state_rawtext(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    if(tree->tmp_qnode == NULL) {
        tree->tmp_qnode = mythread_queue_get_prev_node(tree->myhtml->queue);
    }
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            myhtml_tokenizer_inc_html_offset(html_offset, html_size);
            
            if(html[html_offset] == '/')
            {
                myhtml_tokenizer_inc_html_offset(html_offset, html_size);
                
                if(myhtml_ascii_char_cmp(html[html_offset]))
                {
                    qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, html_offset - 2);
                    
                    qnode->begin = html_offset;
                    qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                    
                    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_NAME;
                    break;
                }
            }
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_rawtext_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            qnode->length = html_offset - qnode->begin;
            myhtml_check_tag_parser(tree, qnode, html, &html_offset, html_size);
            
            if(qnode->token->tag_ctx_idx != tree->tmp_qnode->token->tag_ctx_idx)
            {
                qnode->begin  = 0;
                qnode->length = 0;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
                
                html_offset++;
                break;
            }
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->tmp_qnode = NULL;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            qnode->length = html_offset - qnode->begin;
            myhtml_check_tag_parser(tree, qnode, html, &html_offset, html_size);
            
            if(qnode->token->tag_ctx_idx != tree->tmp_qnode->token->tag_ctx_idx)
            {
                qnode->begin  = 0;
                qnode->length = 0;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
                
                html_offset++;
                break;
            }
            
            html_offset++;
            mh_queue_add(tree, html, html_offset);
            
            tree->tmp_qnode = NULL;
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            break;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            qnode->length = html_offset - qnode->begin;
            myhtml_check_tag_parser(tree, qnode, html, &html_offset, html_size);
            
            if(qnode->token->tag_ctx_idx != tree->tmp_qnode->token->tag_ctx_idx)
            {
                qnode->begin  = 0;
                qnode->length = 0;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_RAWTEXT;
                
                html_offset++;
                break;
            }
            
            qnode->token->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->tmp_qnode = NULL;
            
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
size_t myhtml_tokenizer_state_plaintext(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    qnode->begin = html_offset;
    qnode->length = html_size - html_offset;
    
    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    
    mh_queue_add(tree, html, html_size);
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// CDATA
////
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_cdata_section(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>' && html[html_offset - 1] == ']' && html[html_offset - 2] == ']')
        {
            qnode->length = ((html_offset - 2) - qnode->begin);
            
            html_offset++;
            mh_queue_add(tree, html, html_offset);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
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
size_t myhtml_tokenizer_state_data(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '<')
        {
            myhtml_tokenizer_inc_html_offset(html_offset, html_size);
            
            // close tag
            if(html[html_offset] == '/')
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, html_offset - 1);
                
                myhtml_tokenizer_inc_html_offset(html_offset, html_size);
                
                if(myhtml_ascii_char_cmp(html[html_offset])) {
                    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_TAG_OPEN;
                    qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                    
                    qnode->begin  = html_offset;
                }
                else {
                    qnode->token->tag_ctx_idx = MyTAGS_TAG__COMMENT;
                    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
                    
                    qnode->begin = html_offset - 2;
                }
                
                qnode->length = 0;
                
                break;
            }
            else if(html[html_offset] == '!')
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, html_offset - 1);
                
                myhtml_tokenizer_inc_html_offset(html_offset, html_size);
                
                size_t html_offset_n;
                
                // for a comment
                {
                    html_offset_n = html_offset + 1;
                    
                    if(html_offset_n >= html_size)
                        return html_offset_n; // parse exit
                    
                    if(html[html_offset] == '-' && html[html_offset_n] == '-')
                    {
                        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
                        qnode->token->tag_ctx_idx = MyTAGS_TAG__COMMENT;
                        
                        qnode->begin  = html_offset - 2;
                        qnode->length = 0;
                        
                        html_offset = html_offset_n + 1;
                        break;
                    }
                }
                
                // TODO: for a CDATA; CDATA sections can only be used in foreign content (MathML or SVG)
                if(tree->namespace != MyHTML_NAMESPACE_HTML)
                {
                    html_offset_n = html_offset + 7;
                    
                    if(html_offset_n >= html_size)
                        return html_offset_n; // parse exit
                    
                    if(strncmp(&html[html_offset], "[CDATA[", 7) == 0)
                    {
                        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_CDATA_SECTION;
                        
                        html_offset = html_offset_n;
                        
                        qnode->begin  = html_offset;
                        qnode->length = 0;
                        qnode->token->tag_ctx_idx = MyTAGS_TAG__TEXT;
                        
                        break;
                    }
                }
                
                // for a DOCTYPE
                {
                    html_offset_n = html_offset + 7;
                    
                    if(html_offset_n >= html_size)
                        return html_offset_n; // parse exit
                    
                    if(strncasecmp(&html[html_offset], "DOCTYPE", 7) == 0)
                    {
                        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_NAME;
                        
                        qnode->begin  = html_offset;
                        qnode->length = 7;
                        qnode->token->tag_ctx_idx = MyTAGS_TAG__DOCTYPE;
                        
                        html_offset = html_offset_n + 1;
                        break;
                    }
                }
                
                qnode->begin  = html_offset - 2;
                qnode->length = 0;
                
                qnode->token->tag_ctx_idx = MyTAGS_TAG__COMMENT;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
                break;
            }
            else if(html[html_offset] == '?')
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, html_offset - 1);
                
                myhtml_tokenizer_inc_html_offset(html_offset, html_size);
                
                qnode->begin  = html_offset - 2;
                qnode->length = 0;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
                
                break;
            }
            else if(myhtml_ascii_char_cmp(html[html_offset]))
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, html_offset - 1);
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_TAG_OPEN;
                qnode->begin = html_offset;
                
                html_offset++;
                break;
            }
        }
        else if(qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE && myhtml_ascii_char_cmp(html[html_offset])) {
            qnode->token->type ^= (tree->flags & MyHTML_TOKEN_TYPE_WHITESPACE);
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
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size) {
        // check end of tag name
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            qnode->length = html_offset - qnode->begin;
            myhtml_check_tag_parser(tree, qnode, html, &html_offset, html_size);
            
            html_offset++;
            break;
        }
        // check end of tag
        else if(html[html_offset] == '>')
        {
            qnode->length = html_offset - qnode->begin;
            
            myhtml_check_tag_parser(tree, qnode, html, &html_offset, html_size);
            
            mh_state_set(tree) = mh_tags_get(qnode->token->tag_ctx_idx, data_parser);
            
            html_offset++;
            mh_queue_add(tree, html, html_offset);
            
            break;
        }
        // check end of tag
        else if(html[html_offset] == '/')
        {
            qnode->token->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
            qnode->length = html_offset - qnode->begin;
            
            myhtml_check_tag_parser(tree, qnode, html, &html_offset, html_size);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
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
size_t myhtml_tokenizer_state_before_attribute_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    // skip WS
    myhtml_parser_skip_whitespace()
    
    if(html_offset >= html_size) {
        // move to thread for parsing tag
        return html_offset;
    }
    
    if(html[html_offset] == '>') {
        mh_state_set(tree) = mh_tags_get(qnode->token->tag_ctx_idx, data_parser);
        
        html_offset++;
        mh_queue_add(tree, html, html_offset);
    }
    else if(html[html_offset] == '/') {
        qnode->token->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
        
        html_offset++;
    }
    else {
        myhtml_parser_queue_set_attr(tree, qnode)
        
        tree->queue_attr->name_begin   = html_offset;
        tree->queue_attr->name_length  = 0;
        tree->queue_attr->value_begin  = 0;
        tree->queue_attr->value_length = 0;
        
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
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            tree->queue_attr->name_length = html_offset - tree->queue_attr->name_begin;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_NAME;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '=')
        {
            tree->queue_attr->name_length = html_offset - tree->queue_attr->name_begin;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            tree->queue_attr->name_length = html_offset - tree->queue_attr->name_begin;
            mh_state_set(tree) = mh_tags_get(qnode->token->tag_ctx_idx, data_parser);
            
            html_offset++;
            
            mh_queue_add(tree, html, html_offset);
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            break;
        }
        else if(html[html_offset] == '/')
        {
            tree->queue_attr->name_length = html_offset - tree->queue_attr->name_begin;
            
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
    myhtml_t* myhtml = tree->myhtml;
    
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
            mh_state_set(tree) = mh_tags_get(qnode->token->tag_ctx_idx, data_parser);
            
            html_offset++;
            
            mh_queue_add(tree, html, html_offset);
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            break;
        }
        else if(html[html_offset] == '"' || html[html_offset] == '\'' || html[html_offset] == '<')
        {
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            myhtml_parser_queue_set_attr(tree, qnode)
            
            tree->queue_attr->name_begin   = html_offset;
            tree->queue_attr->name_length  = 0;
            tree->queue_attr->value_begin  = 0;
            tree->queue_attr->value_length = 0;
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME;
            break;
        }
        else if(myhtml_whithspace(html[html_offset], !=, &&))
        {
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            myhtml_parser_queue_set_attr(tree, qnode)
            
            tree->queue_attr->name_begin   = html_offset;
            tree->queue_attr->name_length  = 0;
            tree->queue_attr->value_begin  = 0;
            tree->queue_attr->value_length = 0;
            
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
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>') {
            mh_state_set(tree) = mh_tags_get(qnode->token->tag_ctx_idx, data_parser);
            
            html_offset++;
            
            mh_queue_add(tree, html, html_offset);
            
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
            
            tree->queue_attr->value_begin = html_offset;
            
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
            tree->queue_attr->value_length = html_offset - tree->queue_attr->value_begin;
            
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
            tree->queue_attr->value_length = html_offset - tree->queue_attr->value_begin;
            
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
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME;
            
            tree->queue_attr->value_length = html_offset - tree->queue_attr->value_begin;
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>') {
            tree->queue_attr->value_length = html_offset - tree->queue_attr->value_begin;
            
            mh_state_set(tree) = mh_tags_get(qnode->token->tag_ctx_idx, data_parser);
            
            html_offset++;
            
            mh_queue_add(tree, html, html_offset);
            
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
size_t myhtml_tokenizer_state_comment(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>' && html[html_offset - 1] == '-' && html[html_offset - 2] == '-')
        {
            qnode->length = (html_offset - qnode->begin) + 1;
            
            html_offset++;
            mh_queue_add(tree, html, html_offset);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
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
size_t myhtml_tokenizer_state_bogus_comment(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>')
        {
            qnode->length = (html_offset - qnode->begin) + 1;
            
            html_offset++;
            mh_queue_add(tree, html, html_offset);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

void myhtml_tokenizer_state_init(myhtml_t* myhtml)
{
    myhtml->parse_state_func = (myhtml_tokenizer_state_f*)mymalloc(sizeof(myhtml_tokenizer_state_f) * MyHTML_TOKENIZER_STATE_LAST_ENTRY);
    
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_DATA]                          = myhtml_tokenizer_state_data;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_TAG_OPEN]                      = myhtml_tokenizer_state_tag_open;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME]         = myhtml_tokenizer_state_before_attribute_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME]                = myhtml_tokenizer_state_attribute_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_NAME]          = myhtml_tokenizer_state_after_attribute_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE]        = myhtml_tokenizer_state_before_attribute_value;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED] = myhtml_tokenizer_state_attribute_value_double_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED] = myhtml_tokenizer_state_attribute_value_single_quoted;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_UNQUOTED]      = myhtml_tokenizer_state_attribute_value_unquoted;
    
    // comments
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_COMMENT]                       = myhtml_tokenizer_state_comment;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BOGUS_COMMENT]                 = myhtml_tokenizer_state_bogus_comment;
    
    // cdata
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_CDATA_SECTION]                 = myhtml_tokenizer_state_cdata_section;
    
    // rcdata
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RCDATA]                        = myhtml_tokenizer_state_rcdata;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_NAME]           = myhtml_tokenizer_state_rcdata_end_tag_name;
    
    // rawtext
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RAWTEXT]                       = myhtml_tokenizer_state_rawtext;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_NAME]          = myhtml_tokenizer_state_rawtext_end_tag_name;
    
    // plaintext
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_PLAINTEXT]                     = myhtml_tokenizer_state_plaintext;
    
    // doctype
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_NAME]                     = myhtml_tokenizer_state_before_doctype_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_DOCTYPE_NAME]                            = myhtml_tokenizer_state_doctype_name;
    myhtml->parse_state_func[MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_NAME]                      = myhtml_tokenizer_state_after_doctype_name;
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

}

void myhtml_tokenizer_state_destroy(myhtml_t* myhtml)
{
    if(myhtml->parse_state_func)
        free(myhtml->parse_state_func);
}






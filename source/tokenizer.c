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

#include "tokenizer.h"

static const unsigned char myhtml_tokenizer_chars_map_ws[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x00, 0x00, 0x0b, 0x00, 0x00, 0x0e, 0x0f, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
    0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x00, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
    0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
    0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
    0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d,
    0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
    0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1,
    0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
    0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc,
    0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce,
    0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0,
    0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2,
    0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 
    0xfc, 0xfd, 0xfe, 0xff
};

void myhtml_tokenizer_begin(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    myhtml_t* myhtml = tree->myhtml;
    tree->current_qnode = mythread_queue_get_current_node(myhtml->queue);
    
    tree->current_qnode->text = html;
    tree->current_qnode->tree = tree;
    
    myhtml_token_node_malloc(tree->token, tree->current_qnode->token, tree->mcasync_token_id);
    
    if(myhtml->opt & MyHTML_OPTIONS_PARSE_MODE_SINGLE)
        tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    myhtml_tokenizer_continue(tree, html, html_length);
}

void myhtml_tokenizer_end(myhtml_tree_t* tree, const char* html, size_t html_length)
{
//    if(tree->current_qnode && tree->current_qnode->token) {
//        tree->current_qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, tree->current_qnode, html, html_length);
//    }
    
    tree->myhtml->parse_state_func[(tree->state + MyHTML_TOKENIZER_STATE_LAST_ENTRY)](tree, tree->current_qnode, html, html_length, html_length);
    
    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0 &&
       (tree->myhtml->opt & MyHTML_OPTIONS_PARSE_MODE_SINGLE) == 0)
        myhtml_tokenizer_wait(tree);
    
#ifdef DEBUG_MODE
    if(tree->open_elements->length) {
        MyHTML_DEBUG_ERROR("Tokenizer end; Open Elements is %zu", tree->open_elements->length);
    }
    if(tree->active_formatting->length) {
        MyHTML_DEBUG_ERROR("Tokenizer end; Active Formatting Elements is %zu", tree->active_formatting->length);
    }
#endif
}

void myhtml_tokenizer_continue(myhtml_tree_t* tree, const char* html, size_t html_length)
{
    myhtml_t* myhtml = tree->myhtml;
    myhtml_tokenizer_state_f* state_f = myhtml->parse_state_func;
    
    if(myhtml->opt & MyHTML_OPTIONS_PARSE_MODE_SINGLE)
        tree->flags |= MyHTML_TREE_FLAGS_SINGLE_MODE;
    
    if((tree->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) == 0)
        myhtml_tokenizer_post(tree);
    
    size_t offset = 0;
    
    while (offset < html_length) {
        offset = state_f[tree->state](tree, tree->current_qnode, html, offset, html_length);
    }
}

myhtml_tree_node_t * myhtml_tokenizer_fragment_init(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, enum myhtml_namespace my_namespace)
{
    // step 3
    tree->fragment = myhtml_tree_node_create(tree);
    tree->fragment->my_namespace = my_namespace;
    tree->fragment->tag_idx = tag_idx;
    
    // skip step 4, is already done
    
    // step 5-7
    myhtml_tree_node_t* root = myhtml_tree_node_insert_root(tree, NULL, my_namespace);
    
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
    
    mythread_resume_all(myhtml->thread);
}

void myhtml_check_tag_parser(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t* html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    myhtml_tag_t* tags = myhtml->tags;
    mctree_t* tags_tree = tags->tree;
    
    size_t tagname_begin = qnode->begin;
    
    // get tag id by name
    mctree_index_t idx = mctree_search_lowercase(tags_tree, &html[tagname_begin], qnode->length);
    
    if(idx) {
        qnode->token->tag_ctx_idx = (myhtml_tag_id_t)(tags_tree->nodes[idx].value);
        
        // parser is multi-threaded, and the specification requires the definition namespace for parsing cdata sections
        // but namespace we see after build tree, that is unacceptable to us
        // so this code here
        //
        // if there is an adjusted current node and it is not an element in the HTML namespace and
        // the next seven characters are a case-sensitive match for the string "[CDATA["
        // (the five uppercase letters "CDATA" with a U+005B LEFT SQUARE BRACKET character before and after),
        // then consume those characters and switch to the CDATA section state.
        if(tree->my_namespace != MyHTML_NAMESPACE_HTML)
        {
            if(tags->context[ qnode->token->tag_ctx_idx ].cats[ tree->my_namespace ] == MyHTML_TAG_CATEGORIES_UNDEF)
            {
                if(qnode->token->tag_ctx_idx == MyHTML_TAG_MATH)
                    tree->my_namespace = MyHTML_NAMESPACE_MATHML;
                else if(qnode->token->tag_ctx_idx == MyHTML_TAG_SVG)
                    tree->my_namespace = MyHTML_NAMESPACE_SVG;
                else
                    tree->my_namespace = MyHTML_NAMESPACE_HTML;
            }
        }
        else if(qnode->token->tag_ctx_idx == MyHTML_TAG_MATH)
                tree->my_namespace = MyHTML_NAMESPACE_MATHML;
        else if(qnode->token->tag_ctx_idx == MyHTML_TAG_SVG)
            tree->my_namespace = MyHTML_NAMESPACE_SVG;
    }
    else {
        qnode->token->tag_ctx_idx = myhtml_tag_add(tags, &html[tagname_begin], qnode->length, MyHTML_TOKENIZER_STATE_DATA);
        myhtml_tag_set_category(tags, qnode->token->tag_ctx_idx, tree->my_namespace, MyHTML_TAG_CATEGORIES_ORDINARY);
    }
}

////
mythread_queue_node_t * myhtml_tokenizer_queue_create_text_node_if_need(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset)
{
    if(qnode->token->tag_ctx_idx == MyHTML_TAG__UNDEF)
    {
        if(html_offset > qnode->begin)
        {
            qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
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
        // TODO: for ends is not fine
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
            html_offset++;
            if(html_offset >= html_size) {
                qnode->token->type ^= (qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE);
                return html_offset;
            }
            
            // close tag
            if(html[html_offset] == '/')
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, html_offset - 1);
                
                html_offset++;
                if(html_offset >= html_size) {
                    qnode->token->type ^= (qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE);
                    return html_offset;
                }
                
                if(myhtml_ascii_char_cmp(html[html_offset])) {
                    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_TAG_OPEN;
                    qnode->token->type = MyHTML_TOKEN_TYPE_CLOSE;
                    
                    qnode->begin  = html_offset;
                }
                else {
                    qnode->token->tag_ctx_idx = MyHTML_TAG__COMMENT;
                    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
                    
                    qnode->begin = html_offset - 2;
                }
                
                qnode->length = 0;
                
                break;
            }
            else if(html[html_offset] == '!')
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, html_offset - 1);
                
                html_offset++;
                if(html_offset >= html_size) {
                    qnode->token->type ^= (qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE);
                    return html_offset;
                }
                
                size_t html_offset_n;
                
                // for a comment
                {
                    html_offset_n = html_offset + 1;
                    
                    if(html_offset_n < html_size &&
                       html[html_offset] == '-' && html[html_offset_n] == '-')
                    {
                        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_COMMENT;
                        qnode->token->tag_ctx_idx = MyHTML_TAG__COMMENT;
                        
                        html_offset = html_offset_n + 1;
                        
                        qnode->begin  = html_offset;
                        qnode->length = 0;
                        
                        break;
                    }
                }
                
                // CDATA sections can only be used in foreign content (MathML or SVG)
                if(tree->my_namespace != MyHTML_NAMESPACE_HTML)
                {
                    html_offset_n = html_offset + 7;
                    
                    if(html_offset_n < html_size &&
                       strncmp(&html[html_offset], "[CDATA[", 7) == 0)
                    {
                        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_CDATA_SECTION;
                        
                        html_offset = html_offset_n;
                        
                        qnode->begin  = html_offset;
                        qnode->length = 0;
                        qnode->token->tag_ctx_idx = MyHTML_TAG__TEXT;
                        
                        break;
                    }
                }
                
                // for a DOCTYPE
                {
                    html_offset_n = html_offset + 7;
                    
                    if(html_offset_n < html_size &&
                       myhtml_strncasecmp(&html[html_offset], "DOCTYPE", 7) == 0)
                    {
                        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_NAME;
                        
                        qnode->begin  = html_offset;
                        qnode->length = 7;
                        qnode->token->tag_ctx_idx = MyHTML_TAG__DOCTYPE;
                        
                        html_offset = html_offset_n + 1;
                        break;
                    }
                }
                
                qnode->begin  = html_offset - 2;
                qnode->length = 0;
                
                qnode->token->tag_ctx_idx = MyHTML_TAG__COMMENT;
                
                mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_COMMENT;
                break;
            }
            else if(html[html_offset] == '?')
            {
                qnode = myhtml_tokenizer_queue_create_text_node_if_need(tree, qnode, html, html_offset - 1);
                
                html_offset++;
                if(html_offset >= html_size) {
                    qnode->token->type ^= (tree->flags & MyHTML_TOKEN_TYPE_WHITESPACE);
                    return html_offset;
                }
                
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
        else if(qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE && myhtml_tokenizer_chars_map_ws[ ((unsigned char)html[html_offset]) ]) {
            qnode->token->type ^= (qnode->token->type & MyHTML_TOKEN_TYPE_WHITESPACE);
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
        
        tree->attr_current->name_begin   = html_offset;
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
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            tree->attr_current->name_length = html_offset - tree->attr_current->name_begin;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_NAME;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '=')
        {
            tree->attr_current->name_length = html_offset - tree->attr_current->name_begin;
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            tree->attr_current->name_length = html_offset - tree->attr_current->name_begin;
            mh_state_set(tree) = mh_tags_get(qnode->token->tag_ctx_idx, data_parser);
            
            html_offset++;
            
            mh_queue_add(tree, html, html_offset);
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            break;
        }
        else if(html[html_offset] == '/')
        {
            tree->attr_current->name_length = html_offset - tree->attr_current->name_begin;
            
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
            
            tree->attr_current->name_begin   = html_offset;
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
            
            tree->attr_current->name_begin   = html_offset;
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
            
            tree->attr_current->value_begin = html_offset;
            
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
            tree->attr_current->value_length = html_offset - tree->attr_current->value_begin;
            
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
            tree->attr_current->value_length = html_offset - tree->attr_current->value_begin;
            
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
            
            tree->attr_current->value_length = html_offset - tree->attr_current->value_begin;
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current, tree->token->mcasync_attr_id);
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>') {
            tree->attr_current->value_length = html_offset - tree->attr_current->value_begin;
            
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

myhtml_status_t myhtml_tokenizer_state_init(myhtml_t* myhtml)
{
    myhtml->parse_state_func = (myhtml_tokenizer_state_f*)mymalloc(sizeof(myhtml_tokenizer_state_f) *
                                                                   ((MyHTML_TOKENIZER_STATE_LAST_ENTRY *
                                                                     MyHTML_TOKENIZER_STATE_LAST_ENTRY) + 1));
    
    if(myhtml->parse_state_func == NULL)
        return MyHTML_STATUS_TOKENIZER_ERROR_MEMORY_ALLOCATION;
    
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
    
    // ***********
    // for ends
    // *********
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_DATA)]                          = myhtml_tokenizer_end_state_data;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_TAG_OPEN)]                      = myhtml_tokenizer_end_state_tag_open;
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
                              + MyHTML_TOKENIZER_STATE_BOGUS_COMMENT)]                 = myhtml_tokenizer_end_state_bogus_comment;
    
    // for ends cdata
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_CDATA_SECTION)]                 = myhtml_tokenizer_end_state_cdata_section;
    
    // for ends rcdata
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RCDATA)]                        = myhtml_tokenizer_end_state_rcdata;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_NAME)]           = myhtml_tokenizer_end_state_rcdata_end_tag_name;
    
    // for ends rawtext
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RAWTEXT)]                       = myhtml_tokenizer_end_state_rawtext;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_NAME)]          = myhtml_tokenizer_end_state_rawtext_end_tag_name;
    
    // for ends plaintext
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_PLAINTEXT)]                     = myhtml_tokenizer_end_state_plaintext;
    
    // for ends doctype
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_NAME)]                     = myhtml_tokenizer_end_state_before_doctype_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_DOCTYPE_NAME)]                            = myhtml_tokenizer_end_state_doctype_name;
    myhtml->parse_state_func[(MyHTML_TOKENIZER_STATE_LAST_ENTRY
                              + MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_NAME)]                      = myhtml_tokenizer_end_state_after_doctype_name;
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






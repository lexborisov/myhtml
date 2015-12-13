//
//  myhtml_tokenizer_doctype.c
//  myhtml
//
//  Created by Alexander Borisov on 12.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_tokenizer_doctype.h"

/////////////////////////////////////////////////////////
//// BEFORE DOCTYPE NAME
//// <!DOCTYPE html%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_doctype(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_NAME;
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// BEFORE DOCTYPE NAME
//// <!DOCTYPE html%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_before_doctype_name(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    myhtml_parser_skip_whitespace()
    
    // TODO: ONLY UP?
    if(html[html_offset] == '>')
    {
        tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
        
        html_offset++;
        mh_queue_add(tree, html, mh_queue_current(), html_offset);
        myhtml_token_attr_malloc(tree->token, tree->attr_current);
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DOCTYPE_NAME;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// DOCTYPE NAME
//// <!DOCTYPE html%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_doctype_name(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    size_t name_begin = html_offset;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>')
        {
            myhtml_parser_queue_set_attr(tree, qnode);
            
            tree->queue_attr->name_begin = name_begin;
            tree->queue_attr->name_length = html_offset - name_begin;
            
            html_offset++;
            
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            myhtml_token_attr_malloc(tree->token, tree->attr_current);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            
            break;
        }
        else if(myhtml_whithspace(html[html_offset], ==, ||))
        {
            myhtml_parser_queue_set_attr(tree, qnode);
            
            tree->queue_attr->name_begin = name_begin;
            tree->queue_attr->name_length = html_offset - name_begin;
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_NAME;
            
            html_offset++;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// AFTER DOCTYPE NAME
//// <!DOCTYPE html%HERE%
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_after_doctype_name(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    myhtml_parser_skip_whitespace()
    
    if(html[html_offset] == '>')
    {
        html_offset++;
        mh_queue_add(tree, html, mh_queue_current(), html_offset);
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
        return html_offset;
    }
    
    size_t html_offset_n = html_offset + 6;
    
    if(html_offset_n >= html_size) {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_DOCTYPE;
        return html_offset;
    }
    
    if(strncasecmp(&html[html_offset], "PUBLIC", 6) == 0) {
        myhtml_parser_queue_set_attr(tree, qnode);
        
        tree->queue_attr->name_begin  = html_offset;
        tree->queue_attr->name_length = 6;
        
        myhtml_token_attr_malloc(tree->token, tree->attr_current);
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_PUBLIC_IDENTIFIER;
        
        html_offset = html_offset_n + 1;
    }
    else if(strncasecmp(&html[html_offset], "SYSTEM", 6) == 0) {
        myhtml_parser_queue_set_attr(tree, qnode);
        
        tree->queue_attr->name_begin  = html_offset;
        tree->queue_attr->name_length = 6;
        
        myhtml_token_attr_malloc(tree->token, tree->attr_current);
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_PUBLIC_IDENTIFIER;
        
        html_offset = html_offset_n + 1;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_DOCTYPE;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// BEFORE DOCTYPE PUBLIC IDENTIFIER
//// <!DOCTYPE html PUBLIC %HERE%"
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_before_doctype_public_identifier(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    myhtml_parser_skip_whitespace()
    
    if(html[html_offset] == '"') {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED;
    }
    else if(html[html_offset] == '\'') {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED;
    }
    else if(html[html_offset] == '>')
    {
        tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
        
        html_offset++;
        mh_queue_add(tree, html, mh_queue_current(), html_offset);
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
        return html_offset;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_DOCTYPE;
    }
    
    return (html_offset + 1);
}

/////////////////////////////////////////////////////////
//// DOCTYPE PUBLIC IDENTIFIER DOUBLE or SINGLE QUOTED
//// <!DOCTYPE html PUBLIC %HERE%"
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_doctype_public_identifier_dsq(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size, char quote)
{
    myhtml_t* myhtml = tree->myhtml;
    size_t id_begin = html_offset;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == quote)
        {
            myhtml_parser_queue_set_attr(tree, qnode);
            
            tree->queue_attr->name_begin  = id_begin;
            tree->queue_attr->name_length = html_offset - id_begin;
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_PUBLIC_IDENTIFIER;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
            
            if(id_begin < html_size) {
                myhtml_parser_queue_set_attr(tree, qnode);
                
                tree->queue_attr->name_begin  = id_begin;
                tree->queue_attr->name_length = html_offset - id_begin;
                
                myhtml_token_attr_malloc(tree->token, tree->attr_current);
            }
            
            html_offset++;
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_doctype_public_identifier_double_quoted(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    return myhtml_tokenizer_doctype_public_identifier_dsq(tree, qnode, html, html_offset, html_size, '"');
}

size_t myhtml_tokenizer_state_doctype_public_identifier_single_quoted(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    return myhtml_tokenizer_doctype_public_identifier_dsq(tree, qnode, html, html_offset, html_size, '\'');
}

/////////////////////////////////////////////////////////
//// AFTER DOCTYPE PUBLIC IDENTIFIER
//// <!DOCTYPE html PUBLIC "blah-blah-blah"%HERE%"
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_after_doctype_public_identifier(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    myhtml_parser_skip_whitespace()
    
    if(html[html_offset] == '"')
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED;
    }
    else if(html[html_offset] == '\'')
    {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED;
    }
    else if(html[html_offset] == '>')
    {
        html_offset++;
        mh_queue_add(tree, html, mh_queue_current(), html_offset);
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
        return html_offset;
    }
    else {
        tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_DOCTYPE;
        return html_offset;
    }
    
    html_offset++;
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// DOCTYPE SYSTEM IDENTIFIER DOUBLE or SINGLE QUOTED
//// <!DOCTYPE html PUBLIC %HERE%"
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_doctype_system_identifier_dsq(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size, char quote)
{
    myhtml_t* myhtml = tree->myhtml;
    size_t id_begin = html_offset;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == quote)
        {
            myhtml_parser_queue_set_attr(tree, qnode);
            
            tree->queue_attr->name_begin  = id_begin;
            tree->queue_attr->name_length = html_offset - id_begin;
            
            myhtml_token_attr_malloc(tree->token, tree->attr_current);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_SYSTEM_IDENTIFIER;
            
            html_offset++;
            break;
        }
        else if(html[html_offset] == '>')
        {
            tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
            
            if(id_begin < html_size) {
                myhtml_parser_queue_set_attr(tree, qnode);
                
                tree->queue_attr->name_begin = id_begin;
                tree->queue_attr->name_length = html_offset - id_begin;
                
                myhtml_token_attr_malloc(tree->token, tree->attr_current);
            }
            
            html_offset++;
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}

size_t myhtml_tokenizer_state_doctype_system_identifier_double_quoted(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    return myhtml_tokenizer_doctype_system_identifier_dsq(tree, qnode, html, html_offset, html_size, '"');
}

size_t myhtml_tokenizer_state_doctype_system_identifier_single_quoted(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    return myhtml_tokenizer_doctype_system_identifier_dsq(tree, qnode, html, html_offset, html_size, '\'');
}

/////////////////////////////////////////////////////////
//// AFTER DOCTYPE SYSTEM IDENTIFIER
//// <!DOCTYPE html PUBLIC "blah-blah-blah"%HERE%"
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_after_doctype_system_identifier(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    myhtml_parser_skip_whitespace();
    
    if(html[html_offset] == '>')
    {
        html_offset++;
        mh_queue_add(tree, html, mh_queue_current(), html_offset);
        
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
    }
    else {
        mh_state_set(tree) = MyHTML_TOKENIZER_STATE_BOGUS_DOCTYPE;
    }
    
    return html_offset;
}

/////////////////////////////////////////////////////////
//// BOGUS DOCTYPE
//// find >
/////////////////////////////////////////////////////////
size_t myhtml_tokenizer_state_bogus_doctype(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size)
{
    myhtml_t* myhtml = tree->myhtml;
    
    while(html_offset < html_size)
    {
        if(html[html_offset] == '>')
        {
            html_offset++;
            mh_queue_add(tree, html, mh_queue_current(), html_offset);
            
            mh_state_set(tree) = MyHTML_TOKENIZER_STATE_DATA;
            break;
        }
        
        html_offset++;
    }
    
    return html_offset;
}


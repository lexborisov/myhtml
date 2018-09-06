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

#include "myhtml/rules.h"

void myhtml_insertion_fix_emit_for_text_begin_ws(myhtml_token_t* token, myhtml_token_node_t* node)
{
    myhtml_token_node_wait_for_done(token, node);
    mycore_string_crop_whitespace_from_begin(&node->str);
}

myhtml_token_node_t * myhtml_insertion_fix_split_for_text_begin_ws(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    myhtml_token_node_wait_for_done(tree->token, token);
    size_t len = mycore_string_whitespace_from_begin(&token->str);
    
    if(len == 0)
        return NULL;
    
    // create new ws token and insert
    myhtml_token_node_t* new_token = myhtml_token_node_create(tree->token, tree->mcasync_rules_token_id);
    
    if(new_token == NULL)
        return NULL;
    
    mycore_string_init(tree->mchar, tree->mchar_node_id, &new_token->str, (len + 2));
    
    mycore_string_append(&new_token->str, token->str.data, len);
    
    new_token->type |= MyHTML_TOKEN_TYPE_DONE;
    
    // and cut ws for original
    token->str.data    = mchar_async_crop_first_chars_without_cache(token->str.data, len);
    token->str.length -= len;
    
    return new_token;
}

void myhtml_insertion_fix_for_null_char_drop_all(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    myhtml_token_node_wait_for_done(tree->token, token);
    
    mycore_string_t *str = &token->str;
    size_t len = str->length;
    size_t offset = 0;
    
    for (size_t i = 0; i < len; ++i)
    {
        if (str->data[i] == '\0')
        {
            size_t next_non_null = i;
            while ((next_non_null < len) && str->data[next_non_null] == '\0') {++next_non_null;}
            
            str->length = str->length - (next_non_null - i);
            
            size_t next_null = next_non_null;
            while ((next_null < len) && str->data[next_null] != '\0') {++next_null;}
            
            memmove(&str->data[(i - offset)], &str->data[next_non_null], (next_null - next_non_null));
            
            i = next_null - 1;
            
            offset++;
        }
    }
}

bool myhtml_insertion_mode_initial(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    switch (token->tag_id)
    {
        case MyHTML_TAG__TEXT:
        {
            if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:INFO */
                return false;
            }
            
            myhtml_insertion_fix_emit_for_text_begin_ws(tree->token, token);
            
            // default, other token
            tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
            tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HTML;
            break;
        }
            
        case MyHTML_TAG__COMMENT:
        {
            myhtml_tree_node_insert_comment(tree, token, tree->document);
            return false;
        }
            
        case MyHTML_TAG__DOCTYPE:
        {
            myhtml_token_node_wait_for_done(tree->token, token);
            
            myhtml_token_release_and_check_doctype_attributes(tree->token, token, &tree->doctype);
            
            if((tree->parse_flags & MyHTML_TREE_PARSE_FLAGS_WITHOUT_DOCTYPE_IN_TREE) == 0)
                myhtml_tree_node_insert_doctype(tree, token);
            
            // fix for tokenizer
            if(tree->doctype.is_html == false &&
               (tree->doctype.attr_public == NULL ||
               tree->doctype.attr_system == NULL))
            {
                tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
            }
            
            tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HTML;
            return false;
        }
            
        default:
            tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
            tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HTML;
            break;
    }
    
    return true;
}

bool myhtml_insertion_mode_before_html(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_BR:
            case MyHTML_TAG_HTML:
            case MyHTML_TAG_HEAD:
            case MyHTML_TAG_BODY:
            {
                myhtml_tree_node_insert_root(tree, NULL, MyHTML_NAMESPACE_HTML);
                
                /* %EXTERNAL% VALIDATOR:RULES TAG STATUS:ELEMENT_MISSING_NEED LEVEL:INFO TAG_ID:MyHTML_TAG_HTML NS:MyHTML_NAMESPACE_HTML */
                
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                return true;
            }
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:WARNING */
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__DOCTYPE: {
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:WARNING */
                break;
            }
                
            case MyHTML_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, tree->document);
                break;
            }
                
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:INFO */
                    break;
                }
                
                myhtml_insertion_fix_emit_for_text_begin_ws(tree->token, token);
                
                // default, other token
                myhtml_tree_node_insert_root(tree, NULL, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                return true;
            }
                
            case MyHTML_TAG_HTML:
            {
                myhtml_tree_node_insert_root(tree, token, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                break;
            }
            
            default:
            {
                myhtml_tree_node_insert_root(tree, NULL, MyHTML_NAMESPACE_HTML);
                /* %EXTERNAL% VALIDATOR:RULES TAG STATUS:ELEMENT_MISSING_NEED LEVEL:INFO TAG_ID:MyHTML_TAG_HTML NS:MyHTML_NAMESPACE_HTML */
                
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                return true;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_before_head(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_BR:
            case MyHTML_TAG_HTML:
            case MyHTML_TAG_HEAD:
            case MyHTML_TAG_BODY:
            {
                tree->node_head = myhtml_tree_node_insert(tree, MyHTML_TAG_HEAD, MyHTML_NAMESPACE_HTML);
                /* %EXTERNAL% VALIDATOR:RULES TAG STATUS:ELEMENT_MISSING_NEED LEVEL:INFO TAG_ID:MyHTML_TAG_HEAD NS:MyHTML_NAMESPACE_HTML */
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return true;
            }
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:INFO */
                    break;
                }
                
                myhtml_insertion_fix_emit_for_text_begin_ws(tree->token, token);
                
                // default, other token
                tree->node_head = myhtml_tree_node_insert(tree, MyHTML_TAG_HEAD, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return true;
            }
                
            case MyHTML_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
            }
                
            case MyHTML_TAG__DOCTYPE: {
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            case MyHTML_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token);
            }
                
            case MyHTML_TAG_HEAD:
            {
                tree->node_head = myhtml_tree_node_insert_html_element(tree, token);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                break;
            }
                
            default:
            {
                tree->node_head = myhtml_tree_node_insert(tree, MyHTML_TAG_HEAD, MyHTML_NAMESPACE_HTML);
                /* %EXTERNAL% VALIDATOR:RULES TAG STATUS:ELEMENT_MISSING_NEED LEVEL:INFO TAG_ID:MyHTML_TAG_HEAD NS:MyHTML_NAMESPACE_HTML */
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return true;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_head(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_HEAD:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                break;
            }
                
            case MyHTML_TAG_BR:
            case MyHTML_TAG_HTML:
            case MyHTML_TAG_BODY:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                return true;
            }
                
            case MyHTML_TAG_TEMPLATE:
            {
                if(myhtml_tree_open_elements_find_by_tag_idx_reverse(tree, MyHTML_TAG_TEMPLATE, MyHTML_NAMESPACE_HTML, NULL) == NULL)
                {
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:WARNING */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_ANY NEED_TAG_ID:MyHTML_TAG_TEMPLATE NEED_NS:MyHTML_NAMESPACE_HTML */
                    
                    break;
                }
                
                // oh God...
                myhtml_tree_generate_all_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(current_node && current_node->tag_id != MyHTML_TAG_TEMPLATE) {
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED_CLOSE_BEFORE LEVEL:WARNING */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_TEMPLATE NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_TEMPLATE, MyHTML_NAMESPACE_HTML, false);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                break;
            }
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:WARNING */
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
                
                myhtml_token_node_t* new_token = myhtml_insertion_fix_split_for_text_begin_ws(tree, token);
                if(new_token)
                    myhtml_tree_node_insert_text(tree, new_token);
                
                // default, other token
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                return true;
            }
                
            case MyHTML_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
            }
                
            case MyHTML_TAG__DOCTYPE: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            case MyHTML_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token);
            }
                
            case MyHTML_TAG_BASE:
            case MyHTML_TAG_BASEFONT:
            case MyHTML_TAG_BGSOUND:
            case MyHTML_TAG_LINK:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                break;
            }
                
            case MyHTML_TAG_META:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                // if the element has an http-equiv attribute
                break;
            }
                
            case MyHTML_TAG_TITLE:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                tree->state_of_builder = MyHTML_TOKENIZER_STATE_RCDATA;
                
                break;
            }
                
            case MyHTML_TAG_NOSCRIPT:
            {
                if(tree->flags & MyHTML_TREE_FLAGS_SCRIPT) {
                    myhtml_tree_node_insert_html_element(tree, token);
                    
                    tree->orig_insert_mode = tree->insert_mode;
                    tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                    tree->state_of_builder = MyHTML_TOKENIZER_STATE_RAWTEXT;
                }
                else {
                    myhtml_tree_node_insert_html_element(tree, token);
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD_NOSCRIPT;
                }
                
                break;
            }
                
            case MyHTML_TAG_STYLE:
            case MyHTML_TAG_NOFRAMES:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                tree->state_of_builder = MyHTML_TOKENIZER_STATE_RAWTEXT;
                
                break;
            }
                
            case MyHTML_TAG_SCRIPT:
            {
                // state 1
                enum myhtml_tree_insertion_mode insert_mode;
                myhtml_tree_node_t* adjusted_location = myhtml_tree_appropriate_place_inserting(tree, NULL, &insert_mode);
                
                // state 2
                myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
                
                node->tag_id      = MyHTML_TAG_SCRIPT;
                node->token        = token;
                node->ns = MyHTML_NAMESPACE_HTML;
                node->flags        = MyHTML_TREE_NODE_PARSER_INSERTED|MyHTML_TREE_NODE_BLOCKING;
                
                myhtml_tree_node_insert_by_mode(adjusted_location, node, insert_mode);
                myhtml_tree_open_elements_append(tree, node);
                
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                tree->state_of_builder = MyHTML_TOKENIZER_STATE_SCRIPT_DATA;
                
                break;
            }
                
            case MyHTML_TAG_TEMPLATE:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append(tree, tree->myhtml->marker); // set marker
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TEMPLATE;
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_TEMPLATE);
                
                break;
            }
                
            case MyHTML_TAG_HEAD: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY LEVEL:WARNING */
                break;
            }
                
            default:
            {
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                return true;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_head_noscript(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_NOSCRIPT:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                break;
            }
                
            case MyHTML_TAG_BR:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return true;
            }
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__DOCTYPE: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            case MyHTML_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token);
            }
                
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                    return myhtml_insertion_mode_in_head(tree, token);
                
                // default, other token
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return true;
            }
                
            case MyHTML_TAG_BASEFONT:
            case MyHTML_TAG_BGSOUND:
            case MyHTML_TAG_LINK:
            case MyHTML_TAG_META:
            case MyHTML_TAG_NOFRAMES:
            case MyHTML_TAG_STYLE:
            case MyHTML_TAG__COMMENT:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyHTML_TAG_HEAD:
            case MyHTML_TAG_NOSCRIPT: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY LEVEL:WARNING */
                break;
            }
                
            default:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return true;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_after_head(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_BR:
            case MyHTML_TAG_HTML:
            case MyHTML_TAG_BODY:
            {
                tree->node_body = myhtml_tree_node_insert(tree, MyHTML_TAG_BODY, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                
                /* %EXTERNAL% VALIDATOR:RULES TAG STATUS:ELEMENT_MISSING_NEED LEVEL:INFO TAG_ID:MyHTML_TAG_BODY NS:MyHTML_NAMESPACE_HTML */
                return true;
            }
                
            case MyHTML_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
                
                myhtml_token_node_t* new_token = myhtml_insertion_fix_split_for_text_begin_ws(tree, token);
                if(new_token)
                    myhtml_tree_node_insert_text(tree, new_token);
                
                // default, other token
                tree->node_body = myhtml_tree_node_insert(tree, MyHTML_TAG_BODY, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return true;
            }
                
            case MyHTML_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
                
            case MyHTML_TAG__DOCTYPE: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            case MyHTML_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyHTML_TAG_BODY:
            {
                tree->node_body = myhtml_tree_node_insert_html_element(tree, token);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                break;
            }
                
            case MyHTML_TAG_FRAMESET:
                myhtml_tree_node_insert_html_element(tree, token);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_FRAMESET;
                break;
                
            case MyHTML_TAG_BASE:
            case MyHTML_TAG_BASEFONT:
            case MyHTML_TAG_BGSOUND:
            case MyHTML_TAG_LINK:
            case MyHTML_TAG_META:
            case MyHTML_TAG_NOFRAMES:
            case MyHTML_TAG_SCRIPT:
            case MyHTML_TAG_STYLE:
            case MyHTML_TAG_TEMPLATE:
            case MyHTML_TAG_TITLE:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_open_elements_append(tree, tree->node_head);
                myhtml_insertion_mode_in_head(tree, token);
                myhtml_tree_open_elements_remove(tree, tree->node_head);
            }
                
            case MyHTML_TAG_HEAD: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY LEVEL:WARNING */
                break;
            }
                
            default:
            {
                tree->node_body = myhtml_tree_node_insert(tree, MyHTML_TAG_BODY, MyHTML_NAMESPACE_HTML);
                /* %EXTERNAL% VALIDATOR:RULES TAG STATUS:ELEMENT_MISSING_NEED LEVEL:INFO TAG_ID:MyHTML_TAG_BODY NS:MyHTML_NAMESPACE_HTML */
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return true;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_body_other_end_tag(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    // step 1
    size_t i = tree->open_elements->length;
    while(i) {
        i--;
        
        myhtml_tree_node_t* node = tree->open_elements->list[i];
        
        // step 2
        if(node->tag_id == token->tag_id && node->ns == MyHTML_NAMESPACE_HTML) {
            myhtml_tree_generate_implied_end_tags(tree, token->tag_id, MyHTML_NAMESPACE_HTML);
            
            myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
            if(current_node->tag_id != node->tag_id) {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:node->tag_id NEED_NS:node->ns */
            }
            
            myhtml_tree_open_elements_pop_until_by_node(tree, node, false);
            
            return false;
        }
        
        const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, node->tag_id);
        if(tag_ctx->cats[ node->ns ] & MyHTML_TAG_CATEGORIES_SPECIAL) {
            // parse error
            /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
            break;
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_body(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            case MyHTML_TAG_BODY:
            {
                myhtml_tree_node_t* body_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_BODY, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE);
                
                if(body_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                for (size_t i = 0; i < tree->open_elements->length; i++) {
                    switch (tree->open_elements->list[i]->tag_id) {
                        case MyHTML_TAG_DD:
                        case MyHTML_TAG_DT:
                        case MyHTML_TAG_LI:
                        case MyHTML_TAG_MENUITEM:
                        case MyHTML_TAG_OPTGROUP:
                        case MyHTML_TAG_OPTION:
                        case MyHTML_TAG_P:
                        case MyHTML_TAG_RB:
                        case MyHTML_TAG_RP:
                        case MyHTML_TAG_RT:
                        case MyHTML_TAG_RTC:
                        case MyHTML_TAG_TBODY:
                        case MyHTML_TAG_TD:
                        case MyHTML_TAG_TFOOT:
                        case MyHTML_TAG_TH:
                        case MyHTML_TAG_THEAD:
                        case MyHTML_TAG_TR:
                        case MyHTML_TAG_BODY:
                        case MyHTML_TAG_HTML:
                            // set parse error
                            break;
                            
                        default:
                            break;
                    }
                }
                
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_BODY;
                break;
            }
                
            case MyHTML_TAG_HTML:
            {
                myhtml_tree_node_t* body_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_BODY, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE);
                
                if(body_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                for (size_t i = 0; i < tree->open_elements->length; i++) {
                    switch (tree->open_elements->list[i]->tag_id) {
                        case MyHTML_TAG_DD:
                        case MyHTML_TAG_DT:
                        case MyHTML_TAG_LI:
                        case MyHTML_TAG_MENUITEM:
                        case MyHTML_TAG_OPTGROUP:
                        case MyHTML_TAG_OPTION:
                        case MyHTML_TAG_P:
                        case MyHTML_TAG_RB:
                        case MyHTML_TAG_RP:
                        case MyHTML_TAG_RT:
                        case MyHTML_TAG_RTC:
                        case MyHTML_TAG_TBODY:
                        case MyHTML_TAG_TD:
                        case MyHTML_TAG_TFOOT:
                        case MyHTML_TAG_TH:
                        case MyHTML_TAG_THEAD:
                        case MyHTML_TAG_TR:
                        case MyHTML_TAG_BODY:
                        case MyHTML_TAG_HTML:
                            // set parse error
                            break;
                            
                        default:
                            break;
                    }
                }
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_BODY;
                
                return true;
            }
                
            case MyHTML_TAG_ADDRESS:
            case MyHTML_TAG_ARTICLE:
            case MyHTML_TAG_ASIDE:
            case MyHTML_TAG_BLOCKQUOTE:
            case MyHTML_TAG_BUTTON:
            case MyHTML_TAG_CENTER:
            case MyHTML_TAG_DETAILS:
            case MyHTML_TAG_DIALOG:
            case MyHTML_TAG_DIR:
            case MyHTML_TAG_DIV:
            case MyHTML_TAG_DL:
            case MyHTML_TAG_FIELDSET:
            case MyHTML_TAG_FIGCAPTION:
            case MyHTML_TAG_FIGURE:
            case MyHTML_TAG_FOOTER:
            case MyHTML_TAG_HEADER:
            case MyHTML_TAG_HGROUP:
            case MyHTML_TAG_LISTING:
            case MyHTML_TAG_MAIN:
            case MyHTML_TAG_MENU:
            case MyHTML_TAG_NAV:
            case MyHTML_TAG_OL:
            case MyHTML_TAG_PRE:
            case MyHTML_TAG_SECTION:
            case MyHTML_TAG_SUMMARY:
            case MyHTML_TAG_UL:
            {
                if(myhtml_tree_element_in_scope(tree, token->tag_id, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE) == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                
                // step 2
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(myhtml_is_html_node(current_node, token->tag_id) == false) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:token->tag_id NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                // step 3
                myhtml_tree_open_elements_pop_until(tree, token->tag_id, MyHTML_NAMESPACE_HTML, false);
                break;
            }
                
            case MyHTML_TAG_FORM:
            {
                myhtml_tree_node_t* template_node = myhtml_tree_open_elements_find_by_tag_idx(tree, MyHTML_TAG_TEMPLATE, MyHTML_NAMESPACE_HTML, NULL);
                
                if(template_node == NULL)
                {
                    // step 1
                    myhtml_tree_node_t* node = tree->node_form;
                    
                    // step 2
                    tree->node_form = NULL;
                    
                    // step 3
                    if(node == NULL || myhtml_tree_element_in_scope_by_node(node, MyHTML_TAG_CATEGORIES_SCOPE) == false) {
                        // parse error
                        /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                        
                        break;
                    }
                    
                    // step 4
                    myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                    
                    // step 5
                    myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                    if(current_node != node) {
                        // parse error
                        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:node->tag_id NEED_NS:node->ns */
                    }
                    
                    // step 6
                    myhtml_tree_open_elements_remove(tree, node);
                }
                else {
                    // step 1
                    myhtml_tree_node_t* form_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_FORM, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE);
                    
                    if(form_node == NULL) {
                        // parse error
                        /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                        
                        break;
                    }
                    
                    // step 2
                    myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                    
                    // step 3
                    myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                    if(myhtml_is_html_node(current_node, MyHTML_TAG_FORM) == false) {
                        // parse error
                        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_FORM NEED_NS:MyHTML_NAMESPACE_HTML */
                    }
                    
                    // step 4
                    myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_FORM, MyHTML_NAMESPACE_HTML, false);
                }
                
                break;
            }
                
            case MyHTML_TAG_P:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON) == NULL) {
                    // parse error
                    myhtml_tree_node_insert(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML);
                }
                
                myhtml_tree_tags_close_p(tree, token);
                break;
            }
                
            case MyHTML_TAG_LI:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_LI, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_LIST_ITEM) == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, MyHTML_TAG_LI, MyHTML_NAMESPACE_HTML);
                
                // step 2
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(myhtml_is_html_node(current_node, MyHTML_TAG_LI) == false) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_LI NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                // step 3
                myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_LI, MyHTML_NAMESPACE_HTML, false);
                
                break;
            }
               
            case MyHTML_TAG_DT:
            case MyHTML_TAG_DD:
            {
                if(myhtml_tree_element_in_scope(tree, token->tag_id, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE) == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, token->tag_id, MyHTML_NAMESPACE_HTML);
                
                // step 2
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(myhtml_is_html_node(current_node, token->tag_id) == false) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:token->tag_id NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                // step 3
                myhtml_tree_open_elements_pop_until(tree, token->tag_id, MyHTML_NAMESPACE_HTML, false);
                
                break;
            }
                
            case MyHTML_TAG_H1:
            case MyHTML_TAG_H2:
            case MyHTML_TAG_H3:
            case MyHTML_TAG_H4:
            case MyHTML_TAG_H5:
            case MyHTML_TAG_H6:
            {
                myhtml_tree_node_t** list = tree->open_elements->list;
                
                myhtml_tree_node_t* node = NULL;
                size_t i = tree->open_elements->length;
                while(i) {
                    i--;
                    
                    const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, list[i]->tag_id);
                    
                    if((list[i]->tag_id == MyHTML_TAG_H1 ||
                       list[i]->tag_id == MyHTML_TAG_H2  ||
                       list[i]->tag_id == MyHTML_TAG_H3  ||
                       list[i]->tag_id == MyHTML_TAG_H4  ||
                       list[i]->tag_id == MyHTML_TAG_H5  ||
                       list[i]->tag_id == MyHTML_TAG_H6) &&
                       list[i]->ns == MyHTML_NAMESPACE_HTML) {
                        node = list[i];
                        break;
                    }
                    else if(tag_ctx->cats[list[i]->ns] & MyHTML_TAG_CATEGORIES_SCOPE)
                        break;
                }
                
                if(node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                
                // step 2
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(myhtml_is_html_node(current_node, token->tag_id) == false) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:token->tag_id NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                // step 3
                while(tree->open_elements->length) {
                    tree->open_elements->length--;
                    
                    if((list[tree->open_elements->length]->tag_id == MyHTML_TAG_H1 ||
                       list[tree->open_elements->length]->tag_id == MyHTML_TAG_H2 ||
                       list[tree->open_elements->length]->tag_id == MyHTML_TAG_H3 ||
                       list[tree->open_elements->length]->tag_id == MyHTML_TAG_H4 ||
                       list[tree->open_elements->length]->tag_id == MyHTML_TAG_H5 ||
                       list[tree->open_elements->length]->tag_id == MyHTML_TAG_H6) &&
                       list[tree->open_elements->length]->ns == MyHTML_NAMESPACE_HTML)
                    {
                        break;
                    }
                }
                
                break;
            }
                
            case MyHTML_TAG_A:
            case MyHTML_TAG_B:
            case MyHTML_TAG_BIG:
            case MyHTML_TAG_CODE:
            case MyHTML_TAG_EM:
            case MyHTML_TAG_FONT:
            case MyHTML_TAG_I:
            case MyHTML_TAG_NOBR:
            case MyHTML_TAG_S:
            case MyHTML_TAG_SMALL:
            case MyHTML_TAG_STRIKE:
            case MyHTML_TAG_STRONG:
            case MyHTML_TAG_TT:
            case MyHTML_TAG_U:
            {
                myhtml_tree_adoption_agency_algorithm(tree, token, token->tag_id);
                    //myhtml_insertion_mode_in_body_other_end_tag(tree, token);
                
                break;
            }
                
            case MyHTML_TAG_APPLET:
            case MyHTML_TAG_MARQUEE:
            case MyHTML_TAG_OBJECT:
            {
                if(myhtml_tree_element_in_scope(tree, token->tag_id, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE) == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                
                // step 2
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(myhtml_is_html_node(current_node, token->tag_id) == false) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:token->tag_id NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                // step 3
                myhtml_tree_open_elements_pop_until(tree, token->tag_id, MyHTML_NAMESPACE_HTML, false);
                
                // step 4
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                break;
            }
                
            case MyHTML_TAG_BR:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES CONVERT STATUS:ELEMENT_BAD LEVEL:ERROR FROM_TAG_ID:MyHTML_TAG_BR FROM_NS:MyHTML_NAMESPACE_HTML FROM_TYPE:MyHTML_TOKEN_TYPE_CLOSE TO_TAG_ID:MyHTML_TAG_BR TO_NS:MyHTML_NAMESPACE_HTML TO_TYPE:MyHTML_TOKEN_TYPE_OPEN */
                
                if(token->attr_first) {
                    token->attr_first = NULL;
                }
                
                if(token->attr_last) {
                    token->attr_last = NULL;
                }
                
                myhtml_tree_active_formatting_reconstruction(tree);
                
                token->type = MyHTML_TOKEN_TYPE_OPEN;
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                break;
            }
                
            default:
            {
                return myhtml_insertion_mode_in_body_other_end_tag(tree, token);
            }
        }
    }
    // open elements
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:NULL_CHAR ACTION:IGNORE LEVEL:ERROR */
                    
                    myhtml_insertion_fix_for_null_char_drop_all(tree, token);
                    
                    if(token->str.length) {
                        myhtml_tree_active_formatting_reconstruction(tree);
                        myhtml_tree_node_insert_text(tree, token);
                        
                        if((token->type & MyHTML_TOKEN_TYPE_WHITESPACE) == 0)
                            tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                    }
                }
                else {
                    myhtml_tree_active_formatting_reconstruction(tree);
                    myhtml_tree_node_insert_text(tree, token);
                    
                    if((token->type & MyHTML_TOKEN_TYPE_WHITESPACE) == 0)
                        tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                }
                
                break;
            }
                
            case MyHTML_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
                
            case MyHTML_TAG__DOCTYPE: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:WARNING */
                break;
            }
                
            case MyHTML_TAG_HTML:
            {
                if(myhtml_tree_open_elements_find_by_tag_idx(tree, MyHTML_TAG_TEMPLATE, MyHTML_NAMESPACE_HTML, NULL)) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:WARNING */
                    break;
                }
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:WARNING */
                
                if(tree->open_elements->length > 0) {
                    myhtml_tree_node_t* top_node = tree->open_elements->list[0];
                    
                    if(top_node->token) {
                        myhtml_token_node_wait_for_done(tree->token, token);
                        myhtml_token_node_wait_for_done(tree->token, top_node->token);
                        myhtml_token_node_attr_copy_with_check(tree->token, token, top_node->token, tree->mcasync_rules_attr_id);
                    }
                    else {
                        top_node->token = token;
                    }
                }
                
                break;
            }
                
            case MyHTML_TAG_BASE:
            case MyHTML_TAG_BASEFONT:
            case MyHTML_TAG_BGSOUND:
            case MyHTML_TAG_LINK:
            case MyHTML_TAG_META:
            case MyHTML_TAG_NOFRAMES:
            case MyHTML_TAG_SCRIPT:
            case MyHTML_TAG_STYLE:
            case MyHTML_TAG_TEMPLATE:
            case MyHTML_TAG_TITLE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            case MyHTML_TAG_BODY:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:WARNING */
                
                if(tree->open_elements->length > 1)
                {
                    if(!(tree->open_elements->list[1]->tag_id == MyHTML_TAG_BODY &&
                         tree->open_elements->list[1]->ns == MyHTML_NAMESPACE_HTML) ||
                       myhtml_tree_open_elements_find_by_tag_idx(tree, MyHTML_TAG_TEMPLATE, MyHTML_NAMESPACE_HTML, NULL))
                    {
                        // parse error
                        /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:WARNING */
                        
                        break;
                    }
                }
                else
                    break;
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                if(tree->open_elements->length > 1) {
                    myhtml_tree_node_t* top_node = tree->open_elements->list[1];
                    
                    if(top_node->token) {
                        myhtml_token_node_wait_for_done(tree->token, token);
                        myhtml_token_node_wait_for_done(tree->token, top_node->token);
                        myhtml_token_node_attr_copy_with_check(tree->token, token, top_node->token, tree->mcasync_rules_attr_id);
                    }
                    else {
                        top_node->token = token;
                    }
                }
                
                break;
            }
                
            case MyHTML_TAG_FRAMESET:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                if(tree->open_elements->length > 1)
                {
                    if(!(tree->open_elements->list[1]->tag_id == MyHTML_TAG_BODY &&
                         tree->open_elements->list[1]->ns == MyHTML_NAMESPACE_HTML))
                    {
                        // parse error
                        /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                        
                        break;
                    }
                }
                else
                    break;
                
                if((tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK) == 0) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                myhtml_tree_node_t* node = tree->open_elements->list[1];
                
                myhtml_tree_node_remove(node);
                myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_HTML, MyHTML_NAMESPACE_HTML, true);
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_FRAMESET;
                break;
            }
                
            case MyHTML_TAG__END_OF_FILE:
            {
                if(tree->template_insertion->length)
                    return myhtml_insertion_mode_in_template(tree, token);
                
                myhtml_tree_node_t** list = tree->open_elements->list;
                for(size_t i = 0; i < tree->open_elements->length; i++) {
                    if(list[i]->tag_id != MyHTML_TAG_DD       && list[i]->tag_id != MyHTML_TAG_DT       &&
                       list[i]->tag_id != MyHTML_TAG_LI       && list[i]->tag_id != MyHTML_TAG_MENUITEM &&
                       list[i]->tag_id != MyHTML_TAG_OPTGROUP && list[i]->tag_id != MyHTML_TAG_OPTION   &&
                       list[i]->tag_id != MyHTML_TAG_P        && list[i]->tag_id != MyHTML_TAG_RB       &&
                       list[i]->tag_id != MyHTML_TAG_RP       && list[i]->tag_id != MyHTML_TAG_RT       &&
                       list[i]->tag_id != MyHTML_TAG_RTC      && list[i]->tag_id != MyHTML_TAG_TBODY    &&
                       list[i]->tag_id != MyHTML_TAG_TD       && list[i]->tag_id != MyHTML_TAG_TFOOT    &&
                       list[i]->tag_id != MyHTML_TAG_TH       && list[i]->tag_id != MyHTML_TAG_THEAD    &&
                       list[i]->tag_id != MyHTML_TAG_TR       && list[i]->tag_id != MyHTML_TAG_BODY     &&
                       list[i]->tag_id != MyHTML_TAG_HTML     && list[i]->ns != MyHTML_NAMESPACE_HTML)
                    {
                        // parse error
                    }
                }
                
                myhtml_rules_stop_parsing(tree);
                break;
            }
                
            case MyHTML_TAG_ADDRESS:
            case MyHTML_TAG_ARTICLE:
            case MyHTML_TAG_ASIDE:
            case MyHTML_TAG_BLOCKQUOTE:
            case MyHTML_TAG_CENTER:
            case MyHTML_TAG_DETAILS:
            case MyHTML_TAG_DIALOG:
            case MyHTML_TAG_DIR:
            case MyHTML_TAG_DIV:
            case MyHTML_TAG_DL:
            case MyHTML_TAG_FIELDSET:
            case MyHTML_TAG_FIGCAPTION:
            case MyHTML_TAG_FIGURE:
            case MyHTML_TAG_FOOTER:
            case MyHTML_TAG_HEADER:
            case MyHTML_TAG_HGROUP:
            case MyHTML_TAG_MAIN:
            case MyHTML_TAG_NAV:
            case MyHTML_TAG_OL:
            case MyHTML_TAG_P:
            case MyHTML_TAG_SECTION:
            case MyHTML_TAG_SUMMARY:
            case MyHTML_TAG_UL:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_MENU:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(myhtml_is_html_node(current_node, MyHTML_TAG_MENUITEM))
                    myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_H1:
            case MyHTML_TAG_H2:
            case MyHTML_TAG_H3:
            case MyHTML_TAG_H4:
            case MyHTML_TAG_H5:
            case MyHTML_TAG_H6:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                switch (current_node->tag_id) {
                    case MyHTML_TAG_H1:
                        case MyHTML_TAG_H2:
                        case MyHTML_TAG_H3:
                        case MyHTML_TAG_H4:
                        case MyHTML_TAG_H5:
                        case MyHTML_TAG_H6:
                        
                        if(current_node->ns == MyHTML_NAMESPACE_HTML) {
                            // parse error
                            /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:WARNING */
                            myhtml_tree_open_elements_pop(tree);
                        }
                        
                        break;
                        
                    default:
                        break;
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_PRE:
            case MyHTML_TAG_LISTING:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                // If the next token is a U+000A LINE FEED (LF) character token, then ignore that token and move on to the next one.
                // (Newlines at the start of pre blocks are ignored as an authoring convenience.)
                // !!! see dispatcher (myhtml_rules_tree_dispatcher) for this
                tree->flags |= MyHTML_TREE_FLAGS_PARSE_FLAG|MyHTML_TREE_FLAGS_PARSE_FLAG_EMIT_NEWLINE;
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
                
            case MyHTML_TAG_FORM:
            {
                myhtml_tree_node_t* is_in_node = myhtml_tree_open_elements_find_by_tag_idx(tree, MyHTML_TAG_TEMPLATE, MyHTML_NAMESPACE_HTML, NULL);
                if(tree->node_form && is_in_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_t* current = myhtml_tree_node_insert_html_element(tree, token);
                
                if(is_in_node == NULL)
                    tree->node_form = current;
                
                break;
            }
                
            case MyHTML_TAG_LI:
            {
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                size_t oel_index = tree->open_elements->length;
                
                while (oel_index) {
                    oel_index--;
                    
                    myhtml_tree_node_t* node = tree->open_elements->list[oel_index];
                    const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, node->tag_id);
                    
                    /* 3 */
                    if(myhtml_is_html_node(node, MyHTML_TAG_LI)) {
                        /* 3.1 */
                        myhtml_tree_generate_implied_end_tags(tree, MyHTML_TAG_LI, MyHTML_NAMESPACE_HTML);
                        
                        /* 3.2 */
                        myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                        if(myhtml_is_html_node(current_node, MyHTML_TAG_LI) == false) {
                            // parse error
                            /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                            /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_LI NEED_NS:MyHTML_NAMESPACE_HTML */
                        }
                        
                        /* 3.3 */
                        myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_LI, MyHTML_NAMESPACE_HTML, false);
                        break;
                    }
                    else if(tag_ctx->cats[node->ns] & MyHTML_TAG_CATEGORIES_SPECIAL)
                    {
                        if(!((node->tag_id == MyHTML_TAG_ADDRESS || node->tag_id == MyHTML_TAG_DIV ||
                             node->tag_id == MyHTML_TAG_P) && node->ns == MyHTML_NAMESPACE_HTML))
                                break;
                    }
                }
                
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
            
            case MyHTML_TAG_DT:
            case MyHTML_TAG_DD:
            {
                // this is copy/past
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                size_t oel_index = tree->open_elements->length;
                
                while (oel_index) {
                    oel_index--;
                    
                    myhtml_tree_node_t* node = tree->open_elements->list[oel_index];
                    const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, node->tag_id);
                    
                    if(myhtml_is_html_node(node, MyHTML_TAG_DD)) {
                        myhtml_tree_generate_implied_end_tags(tree, MyHTML_TAG_DD, MyHTML_NAMESPACE_HTML);
                        
                        /* 3.2 */
                        myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                        if(myhtml_is_html_node(current_node, MyHTML_TAG_DD)) {
                            // parse error
                            /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                            /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_DD NEED_NS:MyHTML_NAMESPACE_HTML */
                        }
                        
                        myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_DD, MyHTML_NAMESPACE_HTML, false);
                        break;
                    }
                    else if(myhtml_is_html_node(node, MyHTML_TAG_DT)) {
                        myhtml_tree_generate_implied_end_tags(tree, MyHTML_TAG_DT, MyHTML_NAMESPACE_HTML);
                        
                        /* 3.2 */
                        myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                        if(myhtml_is_html_node(current_node, MyHTML_TAG_DT)) {
                            // parse error
                            /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                            /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_DT NEED_NS:MyHTML_NAMESPACE_HTML */
                        }
                        
                        myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_DT, MyHTML_NAMESPACE_HTML, false);
                        break;
                    }
                    else if(tag_ctx->cats[node->ns] & MyHTML_TAG_CATEGORIES_SPECIAL)
                    {
                        if(!((node->tag_id == MyHTML_TAG_ADDRESS || node->tag_id == MyHTML_TAG_DIV ||
                             node->tag_id == MyHTML_TAG_P) && node->ns == MyHTML_NAMESPACE_HTML))
                                break;
                    }
                }
                
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_PLAINTEXT:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->state_of_builder = MyHTML_TOKENIZER_STATE_PLAINTEXT;
                break;
            }
                
            case MyHTML_TAG_BUTTON:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_BUTTON, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE)) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                    
                    myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                    myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_BUTTON, MyHTML_NAMESPACE_HTML, false);
                }
                
                myhtml_tree_active_formatting_reconstruction(tree);
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
               
            case MyHTML_TAG_A:
            {
                myhtml_tree_node_t* node = myhtml_tree_active_formatting_between_last_marker(tree, MyHTML_TAG_A, NULL);
                
                if(node) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                    
                    myhtml_tree_adoption_agency_algorithm(tree, token, MyHTML_TAG_A);
                    node = myhtml_tree_active_formatting_between_last_marker(tree, MyHTML_TAG_A, NULL);
                    
                    if(node) {
                        myhtml_tree_open_elements_remove(tree, node);
                        myhtml_tree_active_formatting_remove(tree, node);
                    }
                }
                
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_t* current = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append_with_check(tree, current);
                break;
            }
                
            case MyHTML_TAG_B:
            case MyHTML_TAG_BIG:
            case MyHTML_TAG_CODE:
            case MyHTML_TAG_EM:
            case MyHTML_TAG_FONT:
            case MyHTML_TAG_I:
            case MyHTML_TAG_S:
            case MyHTML_TAG_SMALL:
            case MyHTML_TAG_STRIKE:
            case MyHTML_TAG_STRONG:
            case MyHTML_TAG_TT:
            case MyHTML_TAG_U:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_t* current = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append_with_check(tree, current);
                break;
            }

            case MyHTML_TAG_NOBR:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_NOBR, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE)) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                    
                    myhtml_tree_adoption_agency_algorithm(tree, token, MyHTML_TAG_NOBR);
                    myhtml_tree_active_formatting_reconstruction(tree);
                }
                
                myhtml_tree_node_t* current = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append_with_check(tree, current);
                break;
            }

            case MyHTML_TAG_APPLET:
            case MyHTML_TAG_MARQUEE:
            case MyHTML_TAG_OBJECT:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append(tree, tree->myhtml->marker); // marker
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
                
            case MyHTML_TAG_TABLE:
            {
                if((tree->compat_mode & MyHTML_TREE_COMPAT_MODE_QUIRKS) == 0 &&
                   myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON))
                {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                break;
            }
                
            case MyHTML_TAG_AREA:
            case MyHTML_TAG_BR:
            case MyHTML_TAG_EMBED:
            case MyHTML_TAG_IMG:
            case MyHTML_TAG_KEYGEN:
            case MyHTML_TAG_WBR:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
                
            case MyHTML_TAG_INPUT:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                myhtml_token_node_wait_for_done(tree->token, token);
                if(myhtml_token_attr_match_case(tree->token, token, "type", 4, "hidden", 6) == NULL) {
                    tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                }
                
                break;
            }
                
            case MyHTML_TAG_PARAM:
            case MyHTML_TAG_SOURCE:
            case MyHTML_TAG_TRACK:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                break;
            }
                
            case MyHTML_TAG_HR:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(myhtml_is_html_node(current_node, MyHTML_TAG_MENUITEM))
                    myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
                
            case MyHTML_TAG_IMAGE:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES CONVERT STATUS:ELEMENT_CONVERT LEVEL:ERROR FROM_TAG_ID:MyHTML_TAG_IMAGE FROM_NS:MyHTML_NAMESPACE_ANY FROM_TYPE:MyHTML_TOKEN_TYPE_OPEN TO_TAG_ID:MyHTML_TAG_IMG TO_NS:MyHTML_NAMESPACE_ANY TO_TYPE:MyHTML_TOKEN_TYPE_OPEN */
                
                token->tag_id = MyHTML_TAG_IMG;
                return true;
            }
                
            case MyHTML_TAG_TEXTAREA:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                
                // If the next token is a U+000A LINE FEED (LF) character token,
                // then ignore that token and move on to the next one.
                // (Newlines at the start of textarea elements are ignored as an authoring convenience.)
                // !!! see dispatcher (myhtml_rules_tree_dispatcher) for this
                tree->flags |= MyHTML_TREE_FLAGS_PARSE_FLAG|MyHTML_TREE_FLAGS_PARSE_FLAG_EMIT_NEWLINE;
                
                tree->orig_insert_mode = tree->insert_mode;
                tree->flags           ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                tree->insert_mode      = MyHTML_INSERTION_MODE_TEXT;
                tree->state_of_builder = MyHTML_TOKENIZER_STATE_RCDATA;
                
                break;
            }

            case MyHTML_TAG_XMP:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree, token);
                }
                
                myhtml_tree_active_formatting_reconstruction(tree);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                tree->state_of_builder = MyHTML_TOKENIZER_STATE_RAWTEXT;
                
                myhtml_tree_generic_raw_text_element_parsing_algorithm(tree, token);
                break;
            }

            case MyHTML_TAG_IFRAME:
            {
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                tree->state_of_builder = MyHTML_TOKENIZER_STATE_RAWTEXT;
                
                myhtml_tree_generic_raw_text_element_parsing_algorithm(tree, token);
                break;
            }
                
            case MyHTML_TAG_NOEMBED:
            {
                tree->state_of_builder = MyHTML_TOKENIZER_STATE_RAWTEXT;
                myhtml_tree_generic_raw_text_element_parsing_algorithm(tree, token);
                break;
            }
                
            case MyHTML_TAG_NOSCRIPT:
            {
                if(tree->flags & MyHTML_TREE_FLAGS_SCRIPT) {
                    tree->state_of_builder = MyHTML_TOKENIZER_STATE_RAWTEXT;
                    myhtml_tree_generic_raw_text_element_parsing_algorithm(tree, token);
                }
                else {
                    myhtml_tree_active_formatting_reconstruction(tree);
                    myhtml_tree_node_insert_html_element(tree, token);
                }
//                else {
//                    myhtml_tree_node_insert_html_element(tree, token);
//                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD_NOSCRIPT;
//                }
                
                break;
            }
                
            case MyHTML_TAG_SELECT:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                if(tree->insert_mode == MyHTML_INSERTION_MODE_IN_TABLE ||
                   tree->insert_mode == MyHTML_INSERTION_MODE_IN_CAPTION ||
                   tree->insert_mode == MyHTML_INSERTION_MODE_IN_TABLE_BODY ||
                   tree->insert_mode == MyHTML_INSERTION_MODE_IN_ROW ||
                   tree->insert_mode == MyHTML_INSERTION_MODE_IN_CELL)
                {
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_SELECT_IN_TABLE;
                }
                else
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_SELECT;
                
                break;
            }
                
            case MyHTML_TAG_OPTGROUP:
            case MyHTML_TAG_OPTION:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(myhtml_is_html_node(current_node, MyHTML_TAG_OPTION))
                    myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_MENUITEM:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(myhtml_is_html_node(current_node, MyHTML_TAG_MENUITEM))
                    myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_RB:
            case MyHTML_TAG_RTC:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_RUBY, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE)) {
                    myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                }
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(current_node->tag_id != MyHTML_TAG_RUBY) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_RUBY NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_RP:
            case MyHTML_TAG_RT:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_RUBY, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE)) {
                    myhtml_tree_generate_implied_end_tags(tree, MyHTML_TAG_RTC, MyHTML_NAMESPACE_HTML);
                }
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(current_node->tag_id != MyHTML_TAG_RTC && current_node->tag_id != MyHTML_TAG_RUBY) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_RTC NEED_NS:MyHTML_NAMESPACE_HTML */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_RUBY NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_MATH:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_token_node_wait_for_done(tree->token, token);
                
                myhtml_token_adjust_mathml_attributes(token);
                myhtml_token_adjust_foreign_attributes(token);
                
                myhtml_tree_node_t* current_node = myhtml_tree_node_insert_foreign_element(tree, token);
                current_node->ns = MyHTML_NAMESPACE_MATHML;
                
                if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF)
                    myhtml_tree_open_elements_pop(tree);
                
                break;
            }
                
            case MyHTML_TAG_SVG:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_token_node_wait_for_done(tree->token, token);
                
                myhtml_token_adjust_svg_attributes(token);
                myhtml_token_adjust_foreign_attributes(token);
                
                myhtml_tree_node_t* current_node = myhtml_tree_node_insert_foreign_element(tree, token);
                current_node->ns = MyHTML_NAMESPACE_SVG;
                
                if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF)
                    myhtml_tree_open_elements_pop(tree);
                
                break;
            }
                
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_FRAME:
            case MyHTML_TAG_HEAD:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_TH:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            default:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                myhtml_tree_node_insert_html_element(tree, token);
                
                break;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_text(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_SCRIPT:
            {
                // new document.write is not works; set back
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = tree->orig_insert_mode;
                break;
            }
                
            default:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = tree->orig_insert_mode;
                break;
            }
        }
    }
    else {
        if(token->tag_id == MyHTML_TAG__END_OF_FILE)
        {
            // parse error
            /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:PREMATURE_TERMINATION LEVEL:ERROR */
            
            myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
            
            if(current_node->tag_id == MyHTML_TAG_SCRIPT)
                current_node->flags |= MyHTML_TREE_FLAGS_ALREADY_STARTED;
            
            myhtml_tree_open_elements_pop(tree);
            
            tree->insert_mode = tree->orig_insert_mode;
            return true;
        }
        
        myhtml_tree_node_insert_text(tree, token);
    }
    
    return false;
}

bool myhtml_insertion_mode_in_table(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_TABLE:
            {
                myhtml_tree_node_t* table_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TABLE, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(table_node == NULL) {
                     // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                myhtml_tree_open_elements_pop_until_by_node(tree, table_node, false);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                break;
            }
                
            case MyHTML_TAG_BODY:
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_HTML:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_TH:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            case MyHTML_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            default: {
                // parse error
                tree->foster_parenting = true;
                myhtml_insertion_mode_in_body(tree, token);
                tree->foster_parenting = false;
                
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if((current_node->tag_id == MyHTML_TAG_TABLE  ||
                   current_node->tag_id  == MyHTML_TAG_TBODY  ||
                   current_node->tag_id  == MyHTML_TAG_TFOOT  ||
                   current_node->tag_id  == MyHTML_TAG_THEAD  ||
                   current_node->tag_id  == MyHTML_TAG_TR)    &&
                   current_node->ns == MyHTML_NAMESPACE_HTML)
                {
                    myhtml_tree_token_list_clean(tree->token_list);
                    
                    tree->orig_insert_mode = tree->insert_mode;
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_TEXT;
                    
                    return true;
                }
                else {
                    tree->foster_parenting = true;
                    myhtml_insertion_mode_in_body(tree, token);
                    tree->foster_parenting = false;
                    
                    break;
                }
            }
            
            case MyHTML_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
                
            case MyHTML_TAG__DOCTYPE: {
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:WARNING */
                break;
            }
                
            case MyHTML_TAG_CAPTION:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append(tree, tree->myhtml->marker);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_CAPTION;
                break;
            }
                
            case MyHTML_TAG_COLGROUP:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_COLUMN_GROUP;
                break;
            }
                
            case MyHTML_TAG_COL:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                myhtml_tree_node_insert(tree, MyHTML_TAG_COLGROUP, MyHTML_NAMESPACE_HTML);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_COLUMN_GROUP;
                return true;
            }
                
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                break;
            }
               
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TH:
            case MyHTML_TAG_TR:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                myhtml_tree_node_insert(tree, MyHTML_TAG_TBODY, MyHTML_NAMESPACE_HTML);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return true;
            }
                
            case MyHTML_TAG_TABLE:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_node_t* table_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TABLE, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(table_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_TABLE, MyHTML_NAMESPACE_HTML, false);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return true;
            }
                
            case MyHTML_TAG_STYLE:
            case MyHTML_TAG_SCRIPT:
            case MyHTML_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            case MyHTML_TAG_INPUT:
            {
                myhtml_token_node_wait_for_done(tree->token, token);
                
                if(myhtml_token_attr_match_case(tree->token, token, "type", 4, "hidden", 6) == NULL) {
                    tree->foster_parenting = true;
                    myhtml_insertion_mode_in_body(tree, token);
                    tree->foster_parenting = false;
                    
                    break;
                }
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                token->type |= MyHTML_TOKEN_TYPE_CLOSE_SELF;
                break;
            }
                
            case MyHTML_TAG_FORM:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_node_t* template = myhtml_tree_open_elements_find_by_tag_idx(tree, MyHTML_TAG_TEMPLATE, MyHTML_NAMESPACE_HTML, NULL);
                if(tree->node_form || template)
                    break;
                
                tree->node_form = myhtml_tree_node_insert_html_element(tree, token);
                
                myhtml_tree_open_elements_pop(tree);
            }
                
            case MyHTML_TAG__END_OF_FILE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            default:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                tree->foster_parenting = true;
                myhtml_insertion_mode_in_body(tree, token);
                tree->foster_parenting = false;
                
                break;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_table_text(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    // skip NULL, we replaced earlier
    if(token->tag_id == MyHTML_TAG__TEXT)
    {
        if(token->type & MyHTML_TOKEN_TYPE_NULL) {
            // parse error
            /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:NULL_CHAR ACTION:IGNORE LEVEL:ERROR */
            
            myhtml_insertion_fix_for_null_char_drop_all(tree, token);
            
            if(token->str.length)
                myhtml_tree_token_list_append(tree->token_list, token);
        }
        else
            myhtml_tree_token_list_append(tree->token_list, token);
    }
    else {
        myhtml_tree_token_list_t* token_list = tree->token_list;
        bool is_not_ws = false;
        
        for(size_t i = 0; i < token_list->length; i++) {
            if((token_list->list[i]->type & MyHTML_TOKEN_TYPE_WHITESPACE) == 0) {
                is_not_ws = true;
                break;
            }
        }
        
        if(is_not_ws)
        {
            for(size_t i = 0; i < token_list->length; i++) {
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR TOKEN:token_list->list[i] */
                
                tree->foster_parenting = true;
                myhtml_insertion_mode_in_body(tree, token_list->list[i]);
                tree->foster_parenting = false;
            }
        }
        else {
            for(size_t i = 0; i < token_list->length; i++) {
                myhtml_tree_node_insert_text(tree, token_list->list[i]);
            }
        }
        
        tree->insert_mode = tree->orig_insert_mode;
        return true;
    }
    
    return false;
}

bool myhtml_insertion_mode_in_caption(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_CAPTION:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_CAPTION, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE) == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(myhtml_is_html_node(current_node, MyHTML_TAG_CAPTION) == false) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_CAPTION NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_CAPTION, MyHTML_NAMESPACE_HTML, false);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                break;
            }
              
            case MyHTML_TAG_TABLE:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_CAPTION, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE) == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(myhtml_is_html_node(current_node, MyHTML_TAG_CAPTION) == false) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_CAPTION NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_CAPTION, MyHTML_NAMESPACE_HTML, false);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return true;
            }
                
            case MyHTML_TAG_BODY:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_HTML:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_TH:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            {
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            default:
                return myhtml_insertion_mode_in_body(tree, token);
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_TH:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            {
                if(myhtml_tree_element_in_scope(tree, MyHTML_TAG_CAPTION, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE) == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                if(myhtml_is_html_node(current_node, MyHTML_TAG_CAPTION) == false) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_CAPTION NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_CAPTION, MyHTML_NAMESPACE_HTML, false);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return true;
            }
                
            default:
                return myhtml_insertion_mode_in_body(tree, token);
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_column_group(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_COLGROUP:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node && myhtml_is_html_node(current_node, MyHTML_TAG_COLGROUP)) {
                    myhtml_tree_open_elements_pop(tree);
                    
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                    return false;
                }
                
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            case MyHTML_TAG_COL:
            {
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
                
            case MyHTML_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            default: {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node && myhtml_is_html_node(current_node, MyHTML_TAG_COLGROUP)) {
                    myhtml_tree_open_elements_pop(tree);
                    
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                    return true;
                }
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
                
                myhtml_token_node_t* new_token = myhtml_insertion_fix_split_for_text_begin_ws(tree, token);
                if(new_token)
                    myhtml_tree_node_insert_text(tree, new_token);
                
                /* default: */
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node && myhtml_is_html_node(current_node, MyHTML_TAG_COLGROUP)) {
                    myhtml_tree_open_elements_pop(tree);
                    
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                    return true;
                }
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            case MyHTML_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
            }
                
            case MyHTML_TAG__DOCTYPE: {
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                break;
            }
            case MyHTML_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token);
            }
                
            case MyHTML_TAG_COL:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                break;
            }
                
            case MyHTML_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            case MyHTML_TAG__END_OF_FILE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            default:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node && myhtml_is_html_node(current_node, MyHTML_TAG_COLGROUP)) {
                    myhtml_tree_open_elements_pop(tree);
                    
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                    return true;
                }
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                break;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_table_body(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            {
                myhtml_tree_node_t* node = myhtml_tree_element_in_scope(tree, token->tag_id, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                myhtml_tree_clear_stack_back_table_body_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                break;
            }
                
            case MyHTML_TAG_TABLE:
            {
                myhtml_tree_node_t* tbody_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TBODY, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* tfoot_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TFOOT, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* thead_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_THEAD, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(tbody_node == NULL && tfoot_node == NULL && thead_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_THEAD NEED_NS:MyHTML_NAMESPACE_HTML */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TBODY NEED_NS:MyHTML_NAMESPACE_HTML */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TFOOT NEED_NS:MyHTML_NAMESPACE_HTML */
                    break;
                }
                
                myhtml_tree_clear_stack_back_table_body_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return true;
            }
               
            case MyHTML_TAG_BODY:
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_HTML:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TH:
            case MyHTML_TAG_TR:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG_TR:
            {
                myhtml_tree_clear_stack_back_table_body_context(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
                break;
            }
                
            case MyHTML_TAG_TH:
            case MyHTML_TAG_TD:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_clear_stack_back_table_body_context(tree);
                
                myhtml_tree_node_insert(tree, MyHTML_TAG_TR, MyHTML_NAMESPACE_HTML);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
                return true;
            }
                
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            {
                myhtml_tree_node_t* tbody_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TBODY, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* tfoot_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TFOOT, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* thead_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_THEAD, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(tbody_node == NULL && tfoot_node == NULL && thead_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_THEAD NEED_NS:MyHTML_NAMESPACE_HTML */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TBODY NEED_NS:MyHTML_NAMESPACE_HTML */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TFOOT NEED_NS:MyHTML_NAMESPACE_HTML */
                    break;
                }
                
                myhtml_tree_clear_stack_back_table_body_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return true;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_row(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_TR:
            {
                myhtml_tree_node_t* tr_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TR, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(tr_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                myhtml_tree_clear_stack_back_table_row_context(tree);
                
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                break;
            }
                
            case MyHTML_TAG_TABLE:
            {
                myhtml_tree_node_t* tr_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TR, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(tr_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TR NEED_NS:MyHTML_NAMESPACE_HTML */
                    break;
                }
                
                myhtml_tree_clear_stack_back_table_row_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return true;
            }
                
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            {
                myhtml_tree_node_t* node = myhtml_tree_element_in_scope(tree, token->tag_id, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                if(node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                myhtml_tree_node_t* tr_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TR, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                if(tr_node == NULL)
                    break;
                
                myhtml_tree_clear_stack_back_table_row_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return true;
            }
                
            case MyHTML_TAG_BODY:
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_HTML:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TH:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG_TH:
            case MyHTML_TAG_TD:
            {
                myhtml_tree_clear_stack_back_table_row_context(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append(tree, tree->myhtml->marker);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_CELL;
                break;
            }
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            {
                myhtml_tree_node_t* tr_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TR, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(tr_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TR NEED_NS:MyHTML_NAMESPACE_HTML */
                    break;
                }
                
                myhtml_tree_clear_stack_back_table_row_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return true;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_cell(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TH:
            {
                myhtml_tree_node_t* node = myhtml_tree_element_in_scope(tree, token->tag_id, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(myhtml_is_html_node(current_node, token->tag_id) == false)
                {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:token->tag_id NEED_NS:MyHTML_NAMESPACE_HTML */
                }
                
                myhtml_tree_open_elements_pop_until(tree, token->tag_id, MyHTML_NAMESPACE_HTML, false);
                
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
                break;
            }
                
            case MyHTML_TAG_BODY:
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_HTML:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
                
            case MyHTML_TAG_TABLE:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            {
                myhtml_tree_node_t* node = myhtml_tree_element_in_scope(tree, token->tag_id, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TD, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                if(node) {
                    myhtml_tree_close_cell(tree, node, token);
                }
                else {
                    node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TH, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                    if(node)
                        myhtml_tree_close_cell(tree, node, token);
                }
                
                return true;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COL:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_TH:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            {
                myhtml_tree_node_t* td_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TD, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* th_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_TH, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(td_node == NULL && th_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TD NEED_NS:MyHTML_NAMESPACE_HTML */
                    /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TH NEED_NS:MyHTML_NAMESPACE_HTML */
                    
                    break;
                }
                
                myhtml_tree_close_cell(tree, (td_node == NULL ? th_node : td_node), token);
                
                return true;
            }
                
            default:
                return myhtml_insertion_mode_in_body(tree, token);
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_select(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_OPTGROUP:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(myhtml_is_html_node(current_node, MyHTML_TAG_OPTION))
                {
                    if(tree->open_elements->length > 1) {
                        myhtml_tree_node_t *optgrp_node = tree->open_elements->list[ tree->open_elements->length - 2 ];
                        
                        if(myhtml_is_html_node(optgrp_node, MyHTML_TAG_OPTGROUP))
                        {
                            myhtml_tree_open_elements_pop(tree);
                        }
                    }
                }
                
                current_node = myhtml_tree_current_node(tree);
                
                if(myhtml_is_html_node(current_node, MyHTML_TAG_OPTGROUP))
                    myhtml_tree_open_elements_pop(tree);
                else {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_NO_EXPECTED ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                break;
            }
                
            case MyHTML_TAG_OPTION:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(myhtml_is_html_node(current_node, MyHTML_TAG_OPTION))
                    myhtml_tree_open_elements_pop(tree);
                else {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                break;
            }
                
            case MyHTML_TAG_SELECT:
            {
                myhtml_tree_node_t* select_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_SELECT, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_SELECT);
                
                if(select_node == NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    break;
                }
                
                myhtml_tree_open_elements_pop_until_by_node(tree, select_node, false);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                break;
            }
                
            case MyHTML_TAG_TEMPLATE:
                return myhtml_insertion_mode_in_head(tree, token);
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT: {
                if(token->type & MyHTML_TOKEN_TYPE_NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:NULL_CHAR ACTION:IGNORE LEVEL:ERROR */
                    
                    myhtml_insertion_fix_for_null_char_drop_all(tree, token);
                    
                    if(token->str.length)
                        myhtml_tree_node_insert_text(tree, token);
                }
                else
                    myhtml_tree_node_insert_text(tree, token);
                
                break;
            }
                
            case MyHTML_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, NULL);
                break;
                
            case MyHTML_TAG__DOCTYPE: {
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
                
            case MyHTML_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyHTML_TAG_OPTION:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(myhtml_is_html_node(current_node, token->tag_id))
                    myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_OPTGROUP:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_id == MyHTML_TAG_OPTION &&
                   current_node->ns == MyHTML_NAMESPACE_HTML)
                    myhtml_tree_open_elements_pop(tree);
                
                current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_id == token->tag_id &&
                   current_node->ns == MyHTML_NAMESPACE_HTML)
                    myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyHTML_TAG_SELECT:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_node_t* select_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_SELECT, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_SELECT);
                
                if(select_node == NULL) {
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                myhtml_tree_open_elements_pop_until_by_node(tree, select_node, false);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                break;
            }
                
            case MyHTML_TAG_INPUT:
            case MyHTML_TAG_KEYGEN:
            case MyHTML_TAG_TEXTAREA:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_node_t* select_node = myhtml_tree_element_in_scope(tree, MyHTML_TAG_SELECT, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_SELECT);
                
                if(select_node == NULL) {
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                myhtml_tree_open_elements_pop_until_by_node(tree, select_node, false);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return true;
            }
                
            case MyHTML_TAG_SCRIPT:
            case MyHTML_TAG_TEMPLATE:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyHTML_TAG__END_OF_FILE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                break;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_select_in_table(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_TABLE:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TH:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_node_t* some_node = myhtml_tree_element_in_scope(tree, token->tag_id, MyHTML_NAMESPACE_HTML, MyHTML_TAG_CATEGORIES_SCOPE_TABLE);
                
                if(some_node == NULL) {
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_OPEN_NOT_FOUND ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_SELECT, MyHTML_NAMESPACE_HTML, false);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return true;
            }
                
            default:
                return myhtml_insertion_mode_in_select(tree, token);
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_TABLE:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TH:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION LEVEL:ERROR */
                
                myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_SELECT, MyHTML_NAMESPACE_HTML, false);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return true;
            }
            
            default:
                return myhtml_insertion_mode_in_select(tree, token);
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_template(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_TEMPLATE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            case MyHTML_TAG__COMMENT:
            case MyHTML_TAG__DOCTYPE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyHTML_TAG_BASE:
            case MyHTML_TAG_BASEFONT:
            case MyHTML_TAG_BGSOUND:
            case MyHTML_TAG_LINK:
            case MyHTML_TAG_META:
            case MyHTML_TAG_NOFRAMES:
            case MyHTML_TAG_SCRIPT:
            case MyHTML_TAG_STYLE:
            case MyHTML_TAG_TEMPLATE:
            case MyHTML_TAG_TITLE:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_TABLE);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return true;
                
            case MyHTML_TAG_COL:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_COLUMN_GROUP);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_COLUMN_GROUP;
                return true;
                
            case MyHTML_TAG_TR:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_TABLE_BODY);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return true;
                
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TH:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_ROW);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
                return true;
                
            case MyHTML_TAG__END_OF_FILE:
            {
                myhtml_tree_node_t* node = myhtml_tree_open_elements_find_by_tag_idx(tree, MyHTML_TAG_TEMPLATE, MyHTML_NAMESPACE_HTML, NULL);
                
                if(node == NULL) {
                    myhtml_rules_stop_parsing(tree);
                    break;
                }
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TAG STATUS:ELEMENT_NOT_CLOSED LEVEL:ERROR TAG_ID:MyHTML_TAG_TEMPLATE NS:MyHTML_NAMESPACE_HTML */
                
                myhtml_tree_open_elements_pop_until_by_node(tree, node, false);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return true;
            }
                
             default:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_BODY);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return true;
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_after_body(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_HTML:
            {
                if(tree->fragment) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_NO_EXPECTED ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_AFTER_BODY;
                break;
            }
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY LEVEL:ERROR */
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return true;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                    return myhtml_insertion_mode_in_body(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return true;
            }
                
            case MyHTML_TAG__COMMENT:
            {
                if(tree->open_elements->length == 0) {
                    MyCORE_DEBUG_ERROR("after body state; open_elements length < 1");
                    break;
                }
                
                myhtml_tree_node_t* adjusted_location = tree->open_elements->list[0];
                
                // state 2
                myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
                
                node->tag_id      = MyHTML_TAG__COMMENT;
                node->token        = token;
                node->ns = adjusted_location->ns;
                
                myhtml_tree_node_add_child(adjusted_location, node);
                
                break;
            }
                
            case MyHTML_TAG__DOCTYPE: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
            case MyHTML_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyHTML_TAG__END_OF_FILE:
                myhtml_rules_stop_parsing(tree);
                break;
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY LEVEL:ERROR */
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return true;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_frameset(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_FRAMESET:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node == tree->document->child) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_NO_EXPECTED ACTION:IGNORE LEVEL:ERROR */
                    
                    break;
                }
                
                myhtml_tree_open_elements_pop(tree);
                
                current_node = myhtml_tree_current_node(tree);
                
                if(tree->fragment == NULL &&
                   !(current_node->tag_id == MyHTML_TAG_FRAMESET &&
                     current_node->ns == MyHTML_NAMESPACE_HTML))
                {
                    tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_FRAMESET;
                }
                
                break;
            }
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                
                myhtml_token_node_wait_for_done(tree->token, token);
                mycore_string_stay_only_whitespace(&token->str);
                
                if(token->str.length)
                    myhtml_tree_node_insert_text(tree, token);
                
                break;
            }
                
            case MyHTML_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, NULL);
                break;
            }
                
            case MyHTML_TAG__DOCTYPE: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
                
            case MyHTML_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyHTML_TAG_FRAMESET:
                myhtml_tree_node_insert_html_element(tree, token);
                break;
                
            case MyHTML_TAG_FRAME:
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                break;
                
            case MyHTML_TAG_NOFRAMES:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyHTML_TAG__END_OF_FILE:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node == tree->document->child) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                }
                
                myhtml_rules_stop_parsing(tree);
                break;
            }
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_after_frameset(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_id) {
            case MyHTML_TAG_HTML:
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_AFTER_FRAMESET;
                break;
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
        }
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                
                myhtml_token_node_wait_for_done(tree->token, token);
                mycore_string_stay_only_whitespace(&token->str);
                
                if(token->str.length)
                    myhtml_tree_node_insert_text(tree, token);
                
                break;
            }
                
            case MyHTML_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, NULL);
                break;
            }
                
            case MyHTML_TAG__DOCTYPE: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                break;
            }
                
            case MyHTML_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyHTML_TAG_NOFRAMES:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyHTML_TAG__END_OF_FILE:
                myhtml_rules_stop_parsing(tree);
                break;
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_after_after_body(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
        return true;
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__COMMENT:
            {
                myhtml_tree_node_t* adjusted_location = tree->document;
                myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
                
                node->tag_id      = MyHTML_TAG__COMMENT;
                node->token        = token;
                node->ns = adjusted_location->ns;
                
                myhtml_tree_node_add_child(adjusted_location, node);
                break;
            }
                
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                    return myhtml_insertion_mode_in_body(tree, token);
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return true;
            }
                
            case MyHTML_TAG_HTML:
            case MyHTML_TAG__DOCTYPE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyHTML_TAG__END_OF_FILE:
                myhtml_rules_stop_parsing(tree);
                break;
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return true;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_after_after_frameset(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE) {
        // parse error
        /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY LEVEL:ERROR */
        
        return false;
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__COMMENT:
            {
                myhtml_tree_node_t* adjusted_location = tree->document;
                myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
                
                node->tag_id      = MyHTML_TAG__COMMENT;
                node->token        = token;
                node->ns = adjusted_location->ns;
                
                myhtml_tree_node_add_child(adjusted_location, node);
                break;
            }
                
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                    return myhtml_insertion_mode_in_body(tree, token);
                
                myhtml_token_node_t* new_token = myhtml_insertion_fix_split_for_text_begin_ws(tree, token);
                if(new_token)
                    return myhtml_insertion_mode_in_body(tree, new_token);
                
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY LEVEL:ERROR */
                
                break;
            }
                
            case MyHTML_TAG_HTML:
            case MyHTML_TAG__DOCTYPE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyHTML_TAG__END_OF_FILE:
                myhtml_rules_stop_parsing(tree);
                break;
                
            case MyHTML_TAG_NOFRAMES:
                return myhtml_insertion_mode_in_head(tree, token);
                
            default: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_UNNECESSARY LEVEL:ERROR */
                break;
            }
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_foreign_content_end_other(myhtml_tree_t* tree, myhtml_tree_node_t* current_node, myhtml_token_node_t* token)
{
    if(current_node->tag_id != token->tag_id) {
        // parse error
        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:token HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:token->tag_id NEED_NS:MyHTML_NAMESPACE_HTML */
    }
    
    if(tree->open_elements->length)
    {
        myhtml_tree_node_t** list = tree->open_elements->list;
        size_t i = tree->open_elements->length - 1;
        
        while (i)
        {
            current_node = list[i];
            
            if(current_node->tag_id == token->tag_id) {
                myhtml_tree_open_elements_pop_until_by_node(tree, current_node, false);
                return false;
            }
            
            i--;
            
            if(list[i]->ns == MyHTML_NAMESPACE_HTML)
                break;
        }
    }
    
    return tree->myhtml->insertion_func[tree->insert_mode](tree, token);
}

bool myhtml_insertion_mode_in_foreign_content_start_other(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    myhtml_tree_node_t* adjusted_node = myhtml_tree_adjusted_current_node(tree);
    
    myhtml_token_node_wait_for_done(tree->token, token);
    
    if(adjusted_node->ns == MyHTML_NAMESPACE_MATHML) {
        myhtml_token_adjust_mathml_attributes(token);
    }
    else if(adjusted_node->ns == MyHTML_NAMESPACE_SVG) {
        myhtml_token_adjust_svg_attributes(token);
    }
    
    myhtml_token_adjust_foreign_attributes(token);
    
    myhtml_tree_node_t* node = myhtml_tree_node_insert_foreign_element(tree, token);
    node->ns = adjusted_node->ns;
    
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF)
    {
        if(token->tag_id == MyHTML_TAG_SCRIPT &&
           node->ns == MyHTML_NAMESPACE_SVG)
        {
            return myhtml_insertion_mode_in_foreign_content_end_other(tree, myhtml_tree_current_node(tree), token);
        }
        else {
            myhtml_tree_open_elements_pop(tree);
        }
    }
    
    return false;
}

bool myhtml_insertion_mode_in_foreign_content(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE) {
        myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
        
        if(token->tag_id == MyHTML_TAG_SCRIPT &&
           current_node->tag_id == MyHTML_TAG_SCRIPT &&
           current_node->ns == MyHTML_NAMESPACE_SVG)
        {
            myhtml_tree_open_elements_pop(tree);
            // TODO: now script is disable, skip this
            return false;
        }
        
        return myhtml_insertion_mode_in_foreign_content_end_other(tree, current_node, token);
    }
    else {
        switch (token->tag_id)
        {
            case MyHTML_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_NULL) {
                    // parse error
                    /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:NULL_CHAR LEVEL:ERROR */
                    
                    myhtml_token_node_wait_for_done(tree->token, token);
                    myhtml_token_set_replacement_character_for_null_token(tree, token);
                }
                
                myhtml_tree_node_insert_text(tree, token);
                
                if((token->type & MyHTML_TOKEN_TYPE_WHITESPACE) == 0)
                    tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                break;
            }
                
            case MyHTML_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, NULL);
                break;
                
            case MyHTML_TAG__DOCTYPE: {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_WRONG_LOCATION ACTION:IGNORE LEVEL:ERROR */
                
                break;
            }
                
            case MyHTML_TAG_B:
            case MyHTML_TAG_BIG:
            case MyHTML_TAG_BLOCKQUOTE:
            case MyHTML_TAG_BODY:
            case MyHTML_TAG_BR:
            case MyHTML_TAG_CENTER:
            case MyHTML_TAG_CODE:
            case MyHTML_TAG_DD:
            case MyHTML_TAG_DIV:
            case MyHTML_TAG_DL:
            case MyHTML_TAG_DT:
            case MyHTML_TAG_EM:
            case MyHTML_TAG_EMBED:
            case MyHTML_TAG_H1:
            case MyHTML_TAG_H2:
            case MyHTML_TAG_H3:
            case MyHTML_TAG_H4:
            case MyHTML_TAG_H5:
            case MyHTML_TAG_H6:
            case MyHTML_TAG_HEAD:
            case MyHTML_TAG_HR:
            case MyHTML_TAG_I:
            case MyHTML_TAG_IMG:
            case MyHTML_TAG_LI:
            case MyHTML_TAG_LISTING:
            case MyHTML_TAG_MENU:
            case MyHTML_TAG_META:
            case MyHTML_TAG_NOBR:
            case MyHTML_TAG_OL:
            case MyHTML_TAG_P:
            case MyHTML_TAG_PRE:
            case MyHTML_TAG_RUBY:
            case MyHTML_TAG_S:
            case MyHTML_TAG_SMALL:
            case MyHTML_TAG_SPAN:
            case MyHTML_TAG_STRONG:
            case MyHTML_TAG_STRIKE:
            case MyHTML_TAG_SUB:
            case MyHTML_TAG_SUP:
            case MyHTML_TAG_TABLE:
            case MyHTML_TAG_TT:
            case MyHTML_TAG_U:
            case MyHTML_TAG_UL:
            case MyHTML_TAG_VAR:
            case MyHTML_TAG_FONT:
            {
                // parse error
                /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
                
                if(token->tag_id == MyHTML_TAG_FONT)
                {
                    myhtml_token_node_wait_for_done(tree->token, token);
                    
                    if(myhtml_token_attr_by_name(token, "color", 5) == NULL &&
                       myhtml_token_attr_by_name(token, "face" , 4) == NULL &&
                       myhtml_token_attr_by_name(token, "size" , 4) == NULL)
                    {
                        return myhtml_insertion_mode_in_foreign_content_start_other(tree, token);
                    }
                }
                
                if(tree->fragment == NULL) {
                    myhtml_tree_node_t* current_node;
                    
                    do {
                        myhtml_tree_open_elements_pop(tree);
                        current_node = myhtml_tree_current_node(tree);
                    }
                    while(current_node && !(myhtml_tree_is_mathml_integration_point(tree, current_node) ||
                                            myhtml_tree_is_html_integration_point(tree, current_node) ||
                                            current_node->ns == MyHTML_NAMESPACE_HTML));
                    
                    return true;
                }
            }
                
            default:
                return myhtml_insertion_mode_in_foreign_content_start_other(tree, token);
        }
    }
    
    return false;
}

void myhtml_rules_stop_parsing(myhtml_tree_t* tree)
{
    // THIS! IS! -(SPARTA!)- STOP PARSING
}

bool myhtml_rules_check_for_first_newline(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(tree->flags & MyHTML_TREE_FLAGS_PARSE_FLAG) {
        if(tree->flags &MyHTML_TREE_FLAGS_PARSE_FLAG_EMIT_NEWLINE)
        {
            if(token->tag_id == MyHTML_TAG__TEXT) {
                myhtml_token_node_wait_for_done(tree->token, token);
                
                if(token->str.length > 0) {
                    if(token->str.data[0] == '\n') {
                        token->str.data = mchar_async_crop_first_chars_without_cache(token->str.data, 1);
                        
                        token->str.length--;
                        
                        if(token->str.length == 0) {
                            tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_PARSE_FLAG);
                            return true;
                        }
                    }
                }
                else
                    return true;
            }
        }
        
        tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_PARSE_FLAG);
    }
    
    return false;
}

bool myhtml_rules_tree_dispatcher(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    // for textarea && pre && listen
    if(myhtml_rules_check_for_first_newline(tree, token)) {
        tree->token_last_done = token;
        
        return false;
    }
    
    if(tree->state_of_builder != MyHTML_TOKENIZER_STATE_DATA)
        tree->state_of_builder = MyHTML_TOKENIZER_STATE_DATA;
    
    bool reprocess = false;
    myhtml_tree_node_t* adjusted_node = myhtml_tree_adjusted_current_node(tree);
    
    if(tree->open_elements->length == 0 || adjusted_node->ns == MyHTML_NAMESPACE_HTML) {
        reprocess = tree->myhtml->insertion_func[tree->insert_mode](tree, token);
    }
    else if(myhtml_tree_is_mathml_integration_point(tree, adjusted_node) &&
            ((token->tag_id == MyHTML_TAG__TEXT ||
              (token->tag_id != MyHTML_TAG_MGLYPH && token->tag_id != MyHTML_TAG_MALIGNMARK)) &&
             (token->type & MyHTML_TOKEN_TYPE_CLOSE) == 0))
    {
            reprocess = tree->myhtml->insertion_func[tree->insert_mode](tree, token);
    }
    else if(adjusted_node->tag_id == MyHTML_TAG_ANNOTATION_XML &&
       adjusted_node->ns == MyHTML_NAMESPACE_MATHML &&
       token->tag_id == MyHTML_TAG_SVG && (token->type & MyHTML_TOKEN_TYPE_CLOSE) == 0)
    {
        reprocess = tree->myhtml->insertion_func[tree->insert_mode](tree, token);
    }
    else if(myhtml_tree_is_html_integration_point(tree, adjusted_node) &&
            ((token->type & MyHTML_TOKEN_TYPE_CLOSE) == 0 || token->tag_id == MyHTML_TAG__TEXT))
    {
        reprocess = tree->myhtml->insertion_func[tree->insert_mode](tree, token);
    }
    else if(token->tag_id == MyHTML_TAG__END_OF_FILE)
        reprocess = tree->myhtml->insertion_func[tree->insert_mode](tree, token);
    else
        reprocess = myhtml_insertion_mode_in_foreign_content(tree, token);
    
    if(reprocess == false) {
        tree->token_last_done = token;
    }
    
    return reprocess;
}

mystatus_t myhtml_rules_init(myhtml_t* myhtml)
{
    myhtml->insertion_func = (myhtml_insertion_f*)mycore_malloc(sizeof(myhtml_insertion_f) * MyHTML_INSERTION_MODE_LAST_ENTRY);
    
    if(myhtml->insertion_func == NULL)
        return MyHTML_STATUS_RULES_ERROR_MEMORY_ALLOCATION;
    
    myhtml->insertion_func[MyHTML_INSERTION_MODE_INITIAL] = myhtml_insertion_mode_initial;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_BEFORE_HTML] = myhtml_insertion_mode_before_html;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_BEFORE_HEAD] = myhtml_insertion_mode_before_head;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_HEAD] = myhtml_insertion_mode_in_head;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_HEAD_NOSCRIPT] = myhtml_insertion_mode_in_head_noscript;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_HEAD] = myhtml_insertion_mode_after_head;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_BODY] = myhtml_insertion_mode_in_body;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_TEXT] = myhtml_insertion_mode_text;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_TABLE] = myhtml_insertion_mode_in_table;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_TABLE_TEXT] = myhtml_insertion_mode_in_table_text;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_CAPTION] = myhtml_insertion_mode_in_caption;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_COLUMN_GROUP] = myhtml_insertion_mode_in_column_group;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_TABLE_BODY] = myhtml_insertion_mode_in_table_body;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_ROW] = myhtml_insertion_mode_in_row;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_CELL] = myhtml_insertion_mode_in_cell;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_SELECT] = myhtml_insertion_mode_in_select;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_SELECT_IN_TABLE] = myhtml_insertion_mode_in_select_in_table;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_TEMPLATE] = myhtml_insertion_mode_in_template;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_BODY] = myhtml_insertion_mode_after_body;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_FRAMESET] = myhtml_insertion_mode_in_frameset;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_FRAMESET] = myhtml_insertion_mode_after_frameset;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_AFTER_BODY] = myhtml_insertion_mode_after_after_body;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_AFTER_FRAMESET] = myhtml_insertion_mode_after_after_frameset;
    
    return MyHTML_STATUS_OK;
}



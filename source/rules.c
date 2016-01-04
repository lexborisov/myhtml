//
//  myhtml_rules.c
//  myhtml
//
//  Created by Alexander Borisov on 25.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "rules.h"

mybool_t myhtml_insertion_mode_initial(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    switch (token->tag_ctx_idx)
    {
        case MyTAGS_TAG__TEXT:
        {
            if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                return myfalse;
            }
            
            // default, other token
            tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
            tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HTML;
            break;
        }
            
        case MyTAGS_TAG__COMMENT:
        {
            myhtml_tree_node_insert_comment(tree, token, tree->document);
            return myfalse;
        }
            
        case MyTAGS_TAG__DOCTYPE:
        {
            myhtml_token_node_wait_for_done(token);
            
            myhtml_token_release_and_check_doctype_attributes(tree->token, token, &tree->doctype);
            myhtml_tree_node_insert_doctype(tree, token);
            
            // fix for tokenizer
            if(tree->doctype.is_html == myfalse &&
               (tree->doctype.public == NULL ||
               tree->doctype.system == NULL))
            {
                tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
            }
            
            tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HTML;
            return myfalse;
        }
            
        default:
            tree->compat_mode = MyHTML_TREE_COMPAT_MODE_QUIRKS;
            tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HTML;
            break;
    }
    
    return mytrue;
}

mybool_t myhtml_insertion_mode_before_html(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_HEAD:
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_node_insert_root(tree, NULL, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                return mytrue;
            }
                
            default:
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, tree->document);
                break;
            }
                
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    break;
                }
                
                // default, other token
                myhtml_tree_node_insert_root(tree, NULL, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                return mytrue;
            }
                
            case MyTAGS_TAG_HTML:
            {
                myhtml_tree_node_insert_root(tree, token, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                break;
            }
            
            default:
            {
                myhtml_tree_node_insert_root(tree, NULL, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                return mytrue;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_before_head(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_HEAD:
            case MyTAGS_TAG_BODY:
            {
                tree->node_head = myhtml_tree_node_insert(tree, MyTAGS_TAG_HEAD, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return mytrue;
            }
                
            default:
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    break;
                }
                
                // default, other token
                tree->node_head = myhtml_tree_node_insert(tree, MyTAGS_TAG_HEAD, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return mytrue;
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token);
            }
                
            case MyTAGS_TAG_HEAD:
            {
                tree->node_head = myhtml_tree_node_insert_html_element(tree, token);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                break;
            }
                
            default:
            {
                tree->node_head = myhtml_tree_node_insert(tree, MyTAGS_TAG_HEAD, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return mytrue;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_head(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_HEAD:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                break;
            }
                
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                return mytrue;
            }
                
            case MyTAGS_TAG_TEMPLATE:
            {
                if(myhtml_tree_template_insertion_length(tree) == 0)
                    break;
                
                // oh God...
                myhtml_tree_generate_all_implied_end_tags(tree, 0);
                myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_TEMPLATE, myfalse);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                break;
            }
                
            default:
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
                
                // default, other token
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                return mytrue;
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token);
            }
                
            case MyTAGS_TAG_BASE:
            case MyTAGS_TAG_BASEFONT:
            case MyTAGS_TAG_BGSOUND:
            case MyTAGS_TAG_LINK:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                break;
            }
                
            case MyTAGS_TAG_META:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                // if the element has an http-equiv attribute
                break;
            }
                
            case MyTAGS_TAG_TITLE:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                
                break;
            }
                
            case MyTAGS_TAG_NOSCRIPT:
            {
                if(tree->flags & MyHTML_TREE_FLAGS_SCRIPT) {
                    myhtml_tree_node_insert_html_element(tree, token);
                    
                    tree->orig_insert_mode = tree->insert_mode;
                    tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                }
                else {
                    myhtml_tree_node_insert_html_element(tree, token);
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD_NOSCRIPT;
                }
                
                break;
            }
                
            case MyTAGS_TAG_STYLE:
            case MyTAGS_TAG_NOFRAMES:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                
                break;
            }
                
            case MyTAGS_TAG_SCRIPT:
            {
                // state 1
                enum myhtml_tree_insertion_mode insert_mode;
                myhtml_tree_node_t* adjusted_location = myhtml_tree_appropriate_place_inserting(tree, NULL, &insert_mode);
                
                // state 2
                myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
                
                node->tag_idx   = MyTAGS_TAG_SCRIPT;
                node->token     = token;
                node->namespace = MyHTML_NAMESPACE_HTML;
                node->flags     = MyHTML_TREE_NODE_PARSER_INSERTED|MyHTML_TREE_NODE_BLOCKING;
                
                myhtml_tree_node_insert_by_mode(tree, adjusted_location, node, insert_mode);
                myhtml_tree_open_elements_append(tree, node);
                
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                
                break;
            }
                
            case MyTAGS_TAG_TEMPLATE:
            {
                myhtml_tree_node_t* new_idx = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append(tree, new_idx); // set marker
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TEMPLATE;
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_TEMPLATE);
                
                break;
            }
                
            case MyTAGS_TAG_HEAD:
                break;
                
            default:
            {
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                return mytrue;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_head_noscript(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_NOSCRIPT:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                break;
            }
                
            case MyTAGS_TAG_BR:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return mytrue;
            }
                
            default:
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token);
            }
                
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                    return myhtml_insertion_mode_in_head(tree, token);
                
                // default, other token
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return mytrue;
            }
                
            case MyTAGS_TAG_BASEFONT:
            case MyTAGS_TAG_BGSOUND:
            case MyTAGS_TAG_LINK:
            case MyTAGS_TAG_META:
            case MyTAGS_TAG_NOFRAMES:
            case MyTAGS_TAG_STYLE:
            case MyTAGS_TAG__COMMENT:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyTAGS_TAG_HEAD:
            case MyTAGS_TAG_NOSCRIPT:
                break;
                
            default:
            {
                myhtml_tree_open_elements_pop(tree);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                return mytrue;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_head(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_node_insert(tree, MyTAGS_TAG_BODY, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return mytrue;
            }
                
            case MyTAGS_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            default:
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
                
                // default, other token
                myhtml_tree_node_insert(tree, MyTAGS_TAG_BODY, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return mytrue;
            }
                
            case MyTAGS_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                break;
            }
                
            case MyTAGS_TAG_FRAMESET:
                myhtml_tree_node_insert_html_element(tree, token);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_FRAMESET;
                break;
                
            case MyTAGS_TAG_BASE:
            case MyTAGS_TAG_BASEFONT:
            case MyTAGS_TAG_BGSOUND:
            case MyTAGS_TAG_LINK:
            case MyTAGS_TAG_META:
            case MyTAGS_TAG_NOFRAMES:
            case MyTAGS_TAG_SCRIPT:
            case MyTAGS_TAG_STYLE:
            case MyTAGS_TAG_TEMPLATE:
            case MyTAGS_TAG_TITLE:
            {
                myhtml_tree_open_elements_append(tree, tree->node_head);
                myhtml_insertion_mode_in_head(tree, token);
                myhtml_tree_open_elements_remove(tree, tree->node_head);
            }
                
            case MyTAGS_TAG_HEAD:
            {
                break;
            }
                
            default:
            {
                myhtml_tree_node_insert(tree, MyTAGS_TAG_BODY, MyHTML_NAMESPACE_HTML);
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return mytrue;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_body_other_end_tag(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    mytags_context_t* tags_context = tree->myhtml->tags->context;
    
    // step 1
    size_t i = tree->open_elements->length;
    while(i) {
        i--;
        
        myhtml_tree_node_t* node = tree->open_elements->list[i];
        
        // step 2
        while (node->tag_idx == token->tag_ctx_idx) {
            myhtml_tree_generate_implied_end_tags(tree, token->tag_ctx_idx);
            myhtml_tree_open_elements_pop_until_by_node(tree, node, myfalse);
            
            return myfalse;
        }
        
        if(tags_context[node->tag_idx].cats[node->namespace] & MyTAGS_CATEGORIES_SPECIAL) {
            break;
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_body(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_node_t* body_node = myhtml_tree_open_elements_find_by_tag_idx(tree, MyTAGS_TAG_BODY, NULL);
                
                if(body_node == NULL)
                    break;
                
                for (size_t i = 0; i < tree->open_elements->length; i++) {
                    switch (tree->open_elements->list[i]->tag_idx) {
                        case MyTAGS_TAG_DD:
                        case MyTAGS_TAG_DT:
                        case MyTAGS_TAG_LI:
                        case MyTAGS_TAG_OPTGROUP:
                        case MyTAGS_TAG_OPTION:
                        case MyTAGS_TAG_P:
                        case MyTAGS_TAG_RB:
                        case MyTAGS_TAG_RP:
                        case MyTAGS_TAG_RT:
                        case MyTAGS_TAG_RTC:
                        case MyTAGS_TAG_TBODY:
                        case MyTAGS_TAG_TD:
                        case MyTAGS_TAG_TFOOT:
                        case MyTAGS_TAG_TH:
                        case MyTAGS_TAG_THEAD:
                        case MyTAGS_TAG_TR:
                        case MyTAGS_TAG_BODY:
                        case MyTAGS_TAG_HTML:
                            // set parse error
                            break;
                            
                        default:
                            break;
                    }
                }
                
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_BODY;
                break;
            }
                
            case MyTAGS_TAG_HTML:
            {
                myhtml_tree_node_t* body_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_BODY, MyTAGS_CATEGORIES_SCOPE);
                
                if(body_node == NULL)
                    break;
                
                for (size_t i = 0; i < tree->open_elements->length; i++) {
                    switch (tree->open_elements->list[i]->tag_idx) {
                        case MyTAGS_TAG_DD:
                        case MyTAGS_TAG_DT:
                        case MyTAGS_TAG_LI:
                        case MyTAGS_TAG_OPTGROUP:
                        case MyTAGS_TAG_OPTION:
                        case MyTAGS_TAG_P:
                        case MyTAGS_TAG_RB:
                        case MyTAGS_TAG_RP:
                        case MyTAGS_TAG_RT:
                        case MyTAGS_TAG_RTC:
                        case MyTAGS_TAG_TBODY:
                        case MyTAGS_TAG_TD:
                        case MyTAGS_TAG_TFOOT:
                        case MyTAGS_TAG_TH:
                        case MyTAGS_TAG_THEAD:
                        case MyTAGS_TAG_TR:
                        case MyTAGS_TAG_BODY:
                        case MyTAGS_TAG_HTML:
                            // set parse error
                            break;
                            
                        default:
                            break;
                    }
                }
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_BODY;
                
                return mytrue;
            }
                
            case MyTAGS_TAG_ADDRESS:
            case MyTAGS_TAG_ARTICLE:
            case MyTAGS_TAG_ASIDE:
            case MyTAGS_TAG_BLOCKQUOTE:
            case MyTAGS_TAG_BUTTON:
            case MyTAGS_TAG_CENTER:
            case MyTAGS_TAG_DETAILS:
            case MyTAGS_TAG_DIALOG:
            case MyTAGS_TAG_DIR:
            case MyTAGS_TAG_DIV:
            case MyTAGS_TAG_DL:
            case MyTAGS_TAG_FIELDSET:
            case MyTAGS_TAG_FIGCAPTION:
            case MyTAGS_TAG_FIGURE:
            case MyTAGS_TAG_FOOTER:
            case MyTAGS_TAG_HEADER:
            case MyTAGS_TAG_HGROUP:
            case MyTAGS_TAG_LISTING:
            case MyTAGS_TAG_MAIN:
            case MyTAGS_TAG_MENU:
            case MyTAGS_TAG_NAV:
            case MyTAGS_TAG_OL:
            case MyTAGS_TAG_PRE:
            case MyTAGS_TAG_SECTION:
            case MyTAGS_TAG_SUMMARY:
            case MyTAGS_TAG_UL:
            {
                if(myhtml_tree_element_in_scope(tree, token->tag_ctx_idx, MyTAGS_CATEGORIES_SCOPE) == NULL)
                    break;
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, 0);
                
                // step 2
                //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                //if(current_node->namespace != MyHTML_NAMESPACE_HTML)
                //    parse error
                
                // step 3
                myhtml_tree_open_elements_pop_until(tree, token->tag_ctx_idx, myfalse);
                break;
            }
                
            case MyTAGS_TAG_FORM:
            {
                myhtml_tree_node_t* template_node = myhtml_tree_open_elements_find_by_tag_idx(tree, MyTAGS_TAG_TEMPLATE, NULL);
                
                if(template_node == NULL)
                {
                    // step 1
                    myhtml_tree_node_t* node = tree->node_form;
                    
                    // step 2
                    tree->node_form = NULL;
                    
                    // step 3
                    if(node == NULL || myhtml_tree_element_in_scope_by_node(tree, node, MyTAGS_CATEGORIES_SCOPE) == myfalse) {
                        // parse error
                        break;
                    }
                    
                    // step 4
                    myhtml_tree_generate_implied_end_tags(tree, 0);
                    
                    // step 5
                    //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                    //if(current_node != node)
                    //    // parse error
                    
                    // step 6
                    myhtml_tree_open_elements_remove(tree, node);
                }
                else {
                    // step 1
                    myhtml_tree_node_t* form_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_FORM, MyTAGS_CATEGORIES_SCOPE);
                    
                    if(form_node)
                        // parse error
                        break;
                    
                    // step 2
                    myhtml_tree_generate_implied_end_tags(tree, 0);
                    
                    // step 3
                    //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                    //if(current_node != node)
                    //    // parse error
                    
                    // step 4
                    myhtml_tree_open_elements_pop_until_by_node(tree, form_node, myfalse);
                }
                
                break;
            }
                
            case MyTAGS_TAG_P:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON) == NULL) {
                    // parse error
                    myhtml_tree_node_insert(tree, MyTAGS_TAG_P, MyHTML_NAMESPACE_HTML);
                }
                
                myhtml_tree_tags_close_p(tree);
                break;
            }
                
            case MyTAGS_TAG_LI:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_LI, MyTAGS_CATEGORIES_SCOPE_LIST_ITEM) == NULL) {
                    // parse error
                    break;
                }
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, MyTAGS_TAG_LI);
                
                // step 2
                //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                //if(current_node->tag_idx != MyTAGS_TAG_LI)
                //    // parse error
                
                // step 3
                myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_LI, myfalse);
                
                break;
            }
               
            case MyTAGS_TAG_DT:
            case MyTAGS_TAG_DD:
            {
                if(myhtml_tree_element_in_scope(tree, token->tag_ctx_idx, MyTAGS_CATEGORIES_SCOPE) == NULL) {
                    // parse error
                    break;
                }
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, token->tag_ctx_idx);
                
                // step 2
                //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                //if(current_node->tag_idx != token->tag_ctx_idx)
                //    // parse error
                
                // step 3
                myhtml_tree_open_elements_pop_until(tree, token->tag_ctx_idx, myfalse);
                
                break;
            }
                
            case MyTAGS_TAG_H1:
            case MyTAGS_TAG_H2:
            case MyTAGS_TAG_H3:
            case MyTAGS_TAG_H4:
            case MyTAGS_TAG_H5:
            case MyTAGS_TAG_H6:
            {
                myhtml_tree_node_t** list = tree->open_elements->list;
                mytags_context_t* tags_context = tree->myhtml->tags->context;
                
                myhtml_tree_node_t* node = NULL;
                size_t i = tree->open_elements->length;
                while(i) {
                    i--;
                    
                    if(list[i]->tag_idx == MyTAGS_TAG_H1 ||
                       list[i]->tag_idx == MyTAGS_TAG_H2 ||
                       list[i]->tag_idx == MyTAGS_TAG_H3 ||
                       list[i]->tag_idx == MyTAGS_TAG_H4 ||
                       list[i]->tag_idx == MyTAGS_TAG_H5 ||
                       list[i]->tag_idx == MyTAGS_TAG_H6) {
                        node = list[i];
                        break;
                    }
                    else if(tags_context[list[i]->tag_idx].cats[list[i]->namespace] & MyTAGS_CATEGORIES_SCOPE)
                        break;
                }
                
                if(node == NULL) {
                    // parse error
                    break;
                }
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, 0);
                
                // step 2
                //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                //if(current_node->tag_idx != token->tag_ctx_idx)
                //    // parse error
                
                // step 3
                while(tree->open_elements->length) {
                    tree->open_elements->length--;
                    
                    if(list[tree->open_elements->length]->tag_idx == MyTAGS_TAG_H1 ||
                       list[tree->open_elements->length]->tag_idx == MyTAGS_TAG_H2 ||
                       list[tree->open_elements->length]->tag_idx == MyTAGS_TAG_H3 ||
                       list[tree->open_elements->length]->tag_idx == MyTAGS_TAG_H4 ||
                       list[tree->open_elements->length]->tag_idx == MyTAGS_TAG_H5 ||
                       list[tree->open_elements->length]->tag_idx == MyTAGS_TAG_H6)
                    {
                        break;
                    }
                }
                
                break;
            }
                
            case MyTAGS_TAG_A:
            case MyTAGS_TAG_B:
            case MyTAGS_TAG_BIG:
            case MyTAGS_TAG_CODE:
            case MyTAGS_TAG_EM:
            case MyTAGS_TAG_FONT:
            case MyTAGS_TAG_I:
            case MyTAGS_TAG_NOBR:
            case MyTAGS_TAG_S:
            case MyTAGS_TAG_SMALL:
            case MyTAGS_TAG_STRIKE:
            case MyTAGS_TAG_STRONG:
            case MyTAGS_TAG_TT:
            case MyTAGS_TAG_U:
            {
                if(myhtml_tree_adoption_agency_algorithm(tree, token->tag_ctx_idx))
                    myhtml_insertion_mode_in_body_other_end_tag(tree, token);
                
                break;
            }
                
            case MyTAGS_TAG_APPLET:
            case MyTAGS_TAG_MARQUEE:
            case MyTAGS_TAG_OBJECT:
            {
                if(myhtml_tree_element_in_scope(tree, token->tag_ctx_idx, MyTAGS_CATEGORIES_SCOPE) == NULL) {
                    // parse error
                    break;
                }
                
                // step 1
                myhtml_tree_generate_implied_end_tags(tree, 0);
                
                // step 2
                //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                //if(current_node->tag_idx != token->tag_ctx_idx)
                //    // parse error
                
                // step 3
                myhtml_tree_open_elements_pop_until(tree, token->tag_ctx_idx, myfalse);
                
                // step 4
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                break;
            }
                
            case MyTAGS_TAG_BR:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                token->type = MyHTML_TOKEN_TYPE_OPEN;
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
            }
                
            default:
            {
                return myhtml_insertion_mode_in_body_other_end_tag(tree, token);
            }
        }
    }
    // open elements
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                myhtml_tree_node_insert_text(tree, token);
                
                if((token->type & MyHTML_TOKEN_TYPE_WHITESPACE) == 0)
                    tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                break;
            }
                
            case MyTAGS_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
            {
                if(myhtml_tree_open_elements_find_by_tag_idx(tree, MyTAGS_TAG_TEMPLATE, NULL))
                    break;
                
                if(tree->open_elements->length > 0) {
                    myhtml_tree_node_t* top_node = tree->open_elements->list[0];
                    
                    if(top_node->token) {
                        myhtml_token_node_wait_for_done(token);
                        myhtml_token_node_wait_for_done(top_node->token);
                        myhtml_token_node_attr_copy(tree->token, token, top_node->token, tree->mcasync_attr_id);
                    }
                }
                
                break;
            }
                
            case MyTAGS_TAG_BASE:
            case MyTAGS_TAG_BASEFONT:
            case MyTAGS_TAG_BGSOUND:
            case MyTAGS_TAG_LINK:
            case MyTAGS_TAG_META:
            case MyTAGS_TAG_NOFRAMES:
            case MyTAGS_TAG_SCRIPT:
            case MyTAGS_TAG_STYLE:
            case MyTAGS_TAG_TEMPLATE:
            case MyTAGS_TAG_TITLE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            case MyTAGS_TAG_BODY:
            {
                if(tree->open_elements->length > 1)
                {
                    if(tree->open_elements->list[1]->tag_idx != MyTAGS_TAG_BODY ||
                       myhtml_tree_open_elements_find_by_tag_idx(tree, MyTAGS_TAG_TEMPLATE, NULL))
                        break;
                }
                else
                    break;
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                if(tree->open_elements->length > 1) {
                    myhtml_tree_node_t* top_node = tree->open_elements->list[1];
                    
                    if(top_node->token) {
                        myhtml_token_node_wait_for_done(token);
                        myhtml_token_node_wait_for_done(top_node->token);
                        myhtml_token_node_attr_copy(tree->token, token, top_node->token, tree->mcasync_attr_id);
                    }
                }
                
                break;
            }
                
            case MyTAGS_TAG_FRAMESET:
            {
                if(tree->open_elements->length > 1)
                {
                    if(tree->open_elements->list[1]->tag_idx != MyTAGS_TAG_BODY)
                        break;
                }
                else
                    break;
                
                if((tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK) == 0)
                    break;
                
                myhtml_tree_node_t* node = tree->open_elements->list[1];
                
                myhtml_tree_node_remove(tree, node);
                myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_HTML, mytrue);
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_FRAMESET;
                break;
            }
                
            case MyTAGS_TAG__END_OF_FILE:
            {
                if(tree->template_insertion->length)
                    return myhtml_insertion_mode_in_template(tree, token);
                
                myhtml_tree_node_t** list = tree->open_elements->list;
                for(size_t i = 0; i < tree->open_elements->length; i++) {
                    if(list[i]->tag_idx != MyTAGS_TAG_DD     && list[i]->tag_idx != MyTAGS_TAG_DT       &&
                       list[i]->tag_idx != MyTAGS_TAG_LI     && list[i]->tag_idx != MyTAGS_TAG_OPTGROUP &&
                       list[i]->tag_idx != MyTAGS_TAG_OPTION && list[i]->tag_idx != MyTAGS_TAG_P        &&
                       list[i]->tag_idx != MyTAGS_TAG_RB     && list[i]->tag_idx != MyTAGS_TAG_RP       &&
                       list[i]->tag_idx != MyTAGS_TAG_RT     && list[i]->tag_idx != MyTAGS_TAG_RTC      &&
                       list[i]->tag_idx != MyTAGS_TAG_TBODY  && list[i]->tag_idx != MyTAGS_TAG_TD       &&
                       list[i]->tag_idx != MyTAGS_TAG_TFOOT  && list[i]->tag_idx != MyTAGS_TAG_TH       &&
                       list[i]->tag_idx != MyTAGS_TAG_THEAD  && list[i]->tag_idx != MyTAGS_TAG_TR       &&
                       list[i]->tag_idx != MyTAGS_TAG_BODY   && list[i]->tag_idx != MyTAGS_TAG_HTML)
                    {
                        // parse error
                    }
                }
                
                myhtml_rules_stop_parsing(tree);
                break;
            }
                
            case MyTAGS_TAG_ADDRESS:
            case MyTAGS_TAG_ARTICLE:
            case MyTAGS_TAG_ASIDE:
            case MyTAGS_TAG_BLOCKQUOTE:
            case MyTAGS_TAG_CENTER:
            case MyTAGS_TAG_DETAILS:
            case MyTAGS_TAG_DIALOG:
            case MyTAGS_TAG_DIR:
            case MyTAGS_TAG_DIV:
            case MyTAGS_TAG_DL:
            case MyTAGS_TAG_FIELDSET:
            case MyTAGS_TAG_FIGCAPTION:
            case MyTAGS_TAG_FIGURE:
            case MyTAGS_TAG_FOOTER:
            case MyTAGS_TAG_HEADER:
            case MyTAGS_TAG_HGROUP:
            case MyTAGS_TAG_MAIN:
            case MyTAGS_TAG_MENU:
            case MyTAGS_TAG_NAV:
            case MyTAGS_TAG_OL:
            case MyTAGS_TAG_P:
            case MyTAGS_TAG_SECTION:
            case MyTAGS_TAG_SUMMARY:
            case MyTAGS_TAG_UL:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyTAGS_TAG_H1:
            case MyTAGS_TAG_H2:
            case MyTAGS_TAG_H3:
            case MyTAGS_TAG_H4:
            case MyTAGS_TAG_H5:
            case MyTAGS_TAG_H6:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                switch (current_node->tag_idx) {
                    case MyTAGS_TAG_H1:
                        case MyTAGS_TAG_H2:
                        case MyTAGS_TAG_H3:
                        case MyTAGS_TAG_H4:
                        case MyTAGS_TAG_H5:
                        case MyTAGS_TAG_H6:
                        myhtml_tree_open_elements_pop(tree);
                        break;
                        
                    default:
                        break;
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyTAGS_TAG_PRE:
            case MyTAGS_TAG_LISTING:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
                
            case MyTAGS_TAG_FORM:
            {
                myhtml_tree_node_t* is_in_node = myhtml_tree_open_elements_find_by_tag_idx(tree, MyTAGS_TAG_TEMPLATE, NULL);
                if(tree->node_form && is_in_node == NULL)
                    break;
                
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_t* current = myhtml_tree_node_insert_html_element(tree, token);
                
                if(is_in_node == NULL)
                    tree->node_form = current;
                
                break;
            }
                
            case MyTAGS_TAG_LI:
            {
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                mytags_context_t* tags_context = tree->myhtml->tags->context;
                size_t oel_index = tree->open_elements->length;
                
                while (oel_index) {
                    oel_index--;
                    
                    myhtml_tree_node_t* node = tree->open_elements->list[oel_index];
                    
                    if(node->tag_idx == MyTAGS_TAG_LI) {
                        myhtml_tree_generate_implied_end_tags(tree, MyTAGS_TAG_LI);
                        myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_LI, myfalse);
                        break;
                    }
                    else if(tags_context[node->tag_idx].cats[node->namespace] & MyTAGS_CATEGORIES_SPECIAL &&
                            node->tag_idx != MyTAGS_TAG_ADDRESS && node->tag_idx != MyTAGS_TAG_DIV &&
                            node->tag_idx != MyTAGS_TAG_P)
                    {
                        break;
                    }
                }
                
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
            
            case MyTAGS_TAG_DT:
            case MyTAGS_TAG_DD:
            {
                // this is copy/past
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                mytags_context_t* tags_context = tree->myhtml->tags->context;
                size_t oel_index = tree->open_elements->length;
                
                while (oel_index) {
                    oel_index--;
                    
                    myhtml_tree_node_t* node = tree->open_elements->list[oel_index];
                    
                    if(node->tag_idx == MyTAGS_TAG_DD) {
                        myhtml_tree_generate_implied_end_tags(tree, MyTAGS_TAG_DD);
                        myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_DD, myfalse);
                        break;
                    }
                    else if(node->tag_idx == MyTAGS_TAG_DT) {
                        myhtml_tree_generate_implied_end_tags(tree, MyTAGS_TAG_DT);
                        myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_DT, myfalse);
                        break;
                    }
                    else if(tags_context[node->tag_idx].cats[node->namespace] & MyTAGS_CATEGORIES_SPECIAL &&
                            node->tag_idx != MyTAGS_TAG_ADDRESS && node->tag_idx != MyTAGS_TAG_DIV &&
                            node->tag_idx != MyTAGS_TAG_P)
                    {
                        break;
                    }
                }
                
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyTAGS_TAG_PLAINTEXT:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyTAGS_TAG_BUTTON:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_BUTTON, MyTAGS_CATEGORIES_SCOPE)) {
                    myhtml_tree_generate_implied_end_tags(tree, 0);
                    myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_BUTTON, myfalse);
                }
                
                myhtml_tree_active_formatting_reconstruction(tree);
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
               
            case MyTAGS_TAG_A:
            {
                myhtml_tree_node_t* node = myhtml_tree_active_formatting_between_last_marker(tree, MyTAGS_TAG_A, NULL);
                
                if(node) {
                    myhtml_tree_adoption_agency_algorithm(tree, MyTAGS_TAG_A);
                    myhtml_tree_open_elements_remove(tree, node);
                    myhtml_tree_active_formatting_remove(tree, node);
                }
                
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_t* current = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append_with_check(tree, current);
                break;
            }
                
            case MyTAGS_TAG_B:
            case MyTAGS_TAG_BIG:
            case MyTAGS_TAG_CODE:
            case MyTAGS_TAG_EM:
            case MyTAGS_TAG_FONT:
            case MyTAGS_TAG_I:
            case MyTAGS_TAG_S:
            case MyTAGS_TAG_SMALL:
            case MyTAGS_TAG_STRIKE:
            case MyTAGS_TAG_STRONG:
            case MyTAGS_TAG_TT:
            case MyTAGS_TAG_U:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_t* current = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append_with_check(tree, current);
                break;
            }

            case MyTAGS_TAG_NOBR:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_NOBR, MyTAGS_CATEGORIES_SCOPE)) {
                    myhtml_tree_adoption_agency_algorithm(tree, MyTAGS_TAG_NOBR);
                    myhtml_tree_active_formatting_reconstruction(tree);
                }
                
                myhtml_tree_node_t* current = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append_with_check(tree, current);
                break;
            }

            case MyTAGS_TAG_APPLET:
            case MyTAGS_TAG_MARQUEE:
            case MyTAGS_TAG_OBJECT:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_t* current = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append(tree, current); // marker
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
                
            case MyTAGS_TAG_TABLE:
            {
                if((tree->compat_mode & MyHTML_TREE_COMPAT_MODE_QUIRKS) == 0 &&
                   myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON))
                {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                break;
            }
                
            case MyTAGS_TAG_AREA:
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_EMBED:
            case MyTAGS_TAG_IMG:
            case MyTAGS_TAG_KEYGEN:
            case MyTAGS_TAG_WBR:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
                
            case MyTAGS_TAG_INPUT:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                myhtml_token_node_wait_for_done(token);
                if(myhtml_token_attr_match_case(tree->token, token, "type", 4, "hidden", 6)) {
                    tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                }
                
                break;
            }
                
            case MyTAGS_TAG_MENUITEM:
            case MyTAGS_TAG_PARAM:
            case MyTAGS_TAG_SOURCE:
            case MyTAGS_TAG_TRACK:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                break;
            }
                
            case MyTAGS_TAG_HR:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                break;
            }
                
            case MyTAGS_TAG_IMAGE:
            {
                token->tag_ctx_idx = MyTAGS_TAG_IMG;
                return mytrue;
            }
                
            case MyTAGS_TAG_ISINDEX:
            {
                myhtml_tree_node_t* is_in_node = myhtml_tree_open_elements_find_by_tag_idx(tree, MyTAGS_TAG_TEMPLATE, NULL);
                if(tree->node_form && is_in_node == NULL)
                    break;
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_node_t* form = myhtml_tree_node_insert(tree, MyTAGS_TAG_FORM, MyHTML_NAMESPACE_HTML);
                myhtml_tree_node_t* template = myhtml_tree_open_elements_find_by_tag_idx(tree, MyTAGS_TAG_TEMPLATE, NULL);
                
                if(template == NULL)
                    tree->node_form = form;
                
                myhtml_token_node_wait_for_done(token);
                myhtml_token_attr_t *attr_action = myhtml_token_attr_remove_by_name(token, "action", 6);
                
                if(attr_action) {
                    myhtml_token_node_malloc(tree->token, form->token, tree->mcasync_token_id);
                    myhtml_token_node_set_done(form->token);
                    
                    myhtml_token_attr_copy(tree->token, attr_action, form->token, tree->mcasync_attr_id);
                }
                
                myhtml_tree_node_insert(tree, MyTAGS_TAG_HR, MyHTML_NAMESPACE_HTML);
                myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert(tree, MyTAGS_TAG_LABEL, MyHTML_NAMESPACE_HTML);
                
                // Insert characters (see below for what they should say).
                // Prompt: If the token has an attribute with the name "prompt",
                // then the first stream of characters must be the same string as given in that attribute,
                // and the second stream of characters must be empty. Otherwise,
                // the two streams of character tokens together should, together with the input element,
                // express the equivalent of "This is a searchable index. Enter search keywords: (input field)" in the user's preferred language.
                
                myhtml_token_attr_t *attr_prompt = myhtml_token_attr_remove_by_name(token, "prompt", 6);
                //myhtml_token_attr_t *attr_name   =
                myhtml_token_attr_remove_by_name(token, "name", 4);
                
                myhtml_tree_node_t* text_node = myhtml_tree_node_insert(tree, MyTAGS_TAG__TEXT, MyHTML_NAMESPACE_HTML);
                myhtml_tree_open_elements_pop(tree);
                
                myhtml_token_node_malloc(tree->token, text_node->token, tree->mcasync_token_id);
                myhtml_token_node_set_done(text_node->token);
                
                const char message[] = "This is a searchable index. Enter search keywords: (input field)";
                
                if(attr_prompt && attr_prompt->value_length) {
                    myhtml_token_node_text_append(tree->token, text_node->token, &token->my_str_tm.data[ attr_prompt->value_begin ], attr_prompt->value_length);
                }
                else {
                    myhtml_token_node_text_append(tree->token, text_node->token, message, strlen(message));
                }
                
                // Insert an HTML element for an "input" start tag token
                // with all the attributes from the "isindex" token except "name", "action", and "prompt",
                // and with an attribute named "name" with the value "isindex".
                // (This creates an input element with the name attribute set to the magic value "isindex".)
                // Immediately pop the current node off the stack of open elements.
                myhtml_tree_node_t* input_node = myhtml_tree_node_insert(tree, MyTAGS_TAG_INPUT, MyHTML_NAMESPACE_HTML);
                
                myhtml_token_node_malloc(tree->token, input_node->token, tree->mcasync_token_id);
                myhtml_token_node_set_done(input_node->token);
                
                if(token->attr_first) {
                    myhtml_token_attr_t* attr_isindex = token->attr_first;
                    
                    while (attr_isindex)
                    {
                        myhtml_token_attr_copy(tree->token, attr_isindex, input_node->token, tree->mcasync_attr_id);
                        attr_isindex = attr_isindex->next;
                    }
                }
                myhtml_token_node_attr_append(tree->token, input_node->token, "name", 4, "isindex", 7, tree->mcasync_attr_id);
                myhtml_tree_open_elements_pop(tree);
                
                // Insert more characters (see below for what they should say).
                text_node = myhtml_tree_node_insert(tree, MyTAGS_TAG__TEXT, MyHTML_NAMESPACE_HTML);
                myhtml_tree_open_elements_pop(tree);
                
                myhtml_token_node_malloc(tree->token, text_node->token, tree->mcasync_token_id);
                myhtml_token_node_set_done(text_node->token);
                
                if(attr_prompt && attr_prompt->value_length) {
                    myhtml_token_node_text_append(tree->token, text_node->token, &token->my_str_tm.data[ attr_prompt->value_begin ], attr_prompt->value_length);
                }
                else {
                    myhtml_token_node_text_append(tree->token, text_node->token, message, strlen(message));
                }
                
                // and the end
                myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_node_insert(tree, MyTAGS_TAG_HR, MyHTML_NAMESPACE_HTML);
                myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_open_elements_pop(tree);
                
                if(template == NULL)
                    tree->node_form = NULL;
                
                break;
            }
                
            case MyTAGS_TAG_TEXTAREA:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                
                // TODO: If the next token is a U+000A LINE FEED (LF) character token,
                // then ignore that token and move on to the next one.
                // (Newlines at the start of textarea elements are ignored as an authoring convenience.)
                
                tree->orig_insert_mode = tree->insert_mode;
                tree->flags           ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                tree->insert_mode      = MyHTML_INSERTION_MODE_TEXT;
                
                break;
            }

            case MyTAGS_TAG_XMP:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_P, MyTAGS_CATEGORIES_SCOPE_BUTTON)) {
                    myhtml_tree_tags_close_p(tree);
                }
                
                myhtml_tree_active_formatting_reconstruction(tree);
                
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                myhtml_tree_generic_raw_text_element_parsing_algorithm(tree, token);
                break;
            }

            case MyTAGS_TAG_IFRAME:
            {
                tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                myhtml_tree_generic_raw_text_element_parsing_algorithm(tree, token);
                break;
            }
                
            case MyTAGS_TAG_NOEMBED:
            {
                myhtml_tree_generic_raw_text_element_parsing_algorithm(tree, token);
                break;
            }
                
            case MyTAGS_TAG_NOSCRIPT:
            {
                if(tree->flags & MyHTML_TREE_FLAGS_SCRIPT) {
                    myhtml_tree_generic_raw_text_element_parsing_algorithm(tree, token);
                }
                
                break;
            }
                
            case MyTAGS_TAG_SELECT:
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
                
            case MyTAGS_TAG_OPTGROUP:
            case MyTAGS_TAG_OPTION:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx == MyTAGS_TAG_OPTION)
                    myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyTAGS_TAG_RB:
            case MyTAGS_TAG_RTC:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_RUBY, MyTAGS_CATEGORIES_SCOPE)) {
                    myhtml_tree_generate_implied_end_tags(tree, 0);
                }
                
                // myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                // if(current_node->tag_idx != MyTAGS_TAG_RUBY) PARSE_ERROR
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyTAGS_TAG_RP:
            case MyTAGS_TAG_RT:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_RUBY, MyTAGS_CATEGORIES_SCOPE)) {
                    myhtml_tree_generate_implied_end_tags(tree, MyTAGS_TAG_RTC);
                }
                
                // myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                // if(current_node->tag_idx != MyTAGS_TAG_RTC ||
                //    current_node->tag_idx != MyTAGS_TAG_RUBY) PARSE_ERROR
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyTAGS_TAG_MATH:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_token_node_wait_for_done(token);
                
                myhtml_token_adjust_mathml_attributes(token);
                myhtml_token_adjust_foreign_attributes(token);
                
                myhtml_tree_node_t* current_node = myhtml_tree_node_insert_foreign_element(tree, token);
                current_node->namespace = MyHTML_NAMESPACE_MATHML;
                
                if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF)
                    myhtml_tree_open_elements_pop(tree);
                
                break;
            }
                
            case MyTAGS_TAG_SVG:
            {
                myhtml_tree_active_formatting_reconstruction(tree);
                
                myhtml_token_node_wait_for_done(token);
                
                myhtml_token_adjust_svg_attributes(token);
                myhtml_token_adjust_foreign_attributes(token);
                
                myhtml_tree_node_t* current_node = myhtml_tree_node_insert_foreign_element(tree, token);
                current_node->namespace = MyHTML_NAMESPACE_SVG;
                
                if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF)
                    myhtml_tree_open_elements_pop(tree);
                
                break;
            }
                
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_COL:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_FRAME:
            case MyTAGS_TAG_HEAD:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_TH:
            case MyTAGS_TAG_THEAD:
            case MyTAGS_TAG_TR:
            {
                // Ignore this token.
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
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_text(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_SCRIPT:
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
        if(token->tag_ctx_idx == MyTAGS_TAG__END_OF_FILE)
        {
            myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
            
            if(current_node->tag_idx == MyTAGS_TAG_SCRIPT)
                current_node->flags |= MyHTML_TREE_FLAGS_ALREADY_STARTED;
            
            myhtml_tree_open_elements_pop(tree);
            
            tree->insert_mode = tree->orig_insert_mode;
            return mytrue;
        }
        
        myhtml_tree_node_insert_text(tree, token);
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_table(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_TABLE:
            {
                myhtml_tree_node_t* table_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TABLE, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(table_node == NULL)
                     // parse error
                    break;
                
                myhtml_tree_open_elements_pop_until_by_node(tree, table_node, myfalse);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                break;
            }
                
            case MyTAGS_TAG_BODY:
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_COL:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_TH:
            case MyTAGS_TAG_THEAD:
            case MyTAGS_TAG_TR:
            {
                // parse error
                break;
            }
                
            case MyTAGS_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            default: {
                // parse error
                tree->foster_parenting = mytrue;
                myhtml_insertion_mode_in_body(tree, token);
                tree->foster_parenting = myfalse;
                
                break;
            }
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx == MyTAGS_TAG_TABLE ||
                   current_node->tag_idx == MyTAGS_TAG_TBODY ||
                   current_node->tag_idx == MyTAGS_TAG_TFOOT ||
                   current_node->tag_idx == MyTAGS_TAG_THEAD ||
                   current_node->tag_idx == MyTAGS_TAG_TR)
                {
                    myhtml_tree_token_list_clean(tree->token_list);
                    
                    tree->orig_insert_mode = tree->insert_mode;
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_TEXT;
                    
                    return mytrue;
                }
            }
                
            case MyTAGS_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_CAPTION:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                
                myhtml_tree_node_t* node = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append(tree, node);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_CAPTION;
                break;
            }
                
            case MyTAGS_TAG_COLGROUP:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_COLUMN_GROUP;
                break;
            }
                
            case MyTAGS_TAG_COL:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                myhtml_tree_node_insert(tree, MyTAGS_TAG_COLGROUP, MyHTML_NAMESPACE_HTML);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_COLUMN_GROUP;
                return mytrue;
            }
                
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_THEAD:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                break;
            }
               
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TH:
            case MyTAGS_TAG_TR:
            {
                myhtml_tree_clear_stack_back_table_context(tree);
                myhtml_tree_node_insert(tree, MyTAGS_TAG_TBODY, MyHTML_NAMESPACE_HTML);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return mytrue;
            }
                
            case MyTAGS_TAG_TABLE:
            {
                // parse error
                myhtml_tree_node_t* table_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TABLE, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(table_node == NULL)
                    break;
                
                myhtml_tree_open_elements_pop_until_by_node(tree, table_node, myfalse);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return mytrue;
            }
                
            case MyTAGS_TAG_STYLE:
            case MyTAGS_TAG_SCRIPT:
            case MyTAGS_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            case MyTAGS_TAG_INPUT:
            {
                // If the token does not have an attribute with the name "type",
                // or if it does, but that attribute's value is not an ASCII
                // case-insensitive match for the string "hidden",
                // then: act as described in the "anything else" entry below.
                //if() {
                
                    myhtml_tree_node_insert_html_element(tree, token);
                    myhtml_tree_open_elements_pop(tree);
                    break;
                //}
            }
                
            case MyTAGS_TAG_FORM:
            {
                // parse error
                
                myhtml_tree_node_t* template = myhtml_tree_open_elements_find_by_tag_idx(tree, MyTAGS_TAG_TEMPLATE, NULL);
                if(tree->node_form == NULL || template)
                    break;
                
                tree->node_form = myhtml_tree_node_insert_html_element(tree, token);
                
                myhtml_tree_open_elements_pop(tree);
            }
                
            case MyTAGS_TAG__END_OF_FILE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            default:
            {
                // parse error
                tree->foster_parenting = mytrue;
                myhtml_insertion_mode_in_body(tree, token);
                tree->foster_parenting = myfalse;
                
                break;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_table_text(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    // skip NULL, we replaced earlier
    if(token->tag_ctx_idx == MyTAGS_TAG__TEXT)
    {
        myhtml_tree_token_list_append(tree->token_list, token);
    }
    else {
        myhtml_tree_token_list_t* token_list = tree->token_list;
        
        for(size_t i = 0; i < token_list->length; i++) {
            if((token->type & MyHTML_TOKEN_TYPE_WHITESPACE) == 0)
            {
                tree->foster_parenting = mytrue;
                myhtml_insertion_mode_in_body(tree, token_list->list[i]);
                tree->foster_parenting = myfalse;
            }
            else
                myhtml_tree_node_insert_text(tree, token_list->list[i]);
        }
        
        tree->insert_mode = tree->orig_insert_mode;
        return mytrue;
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_caption(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_CAPTION:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_CAPTION, MyTAGS_CATEGORIES_SCOPE_TABLE) == myfalse) {
                    // parse error
                    break;
                }
                
                myhtml_tree_generate_implied_end_tags(tree, 0);
                
                //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                //if(current_node->tag_idx != MyTAGS_TAG_CAPTION) {
                //    // parse error
                //}
                
                myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_CAPTION, myfalse);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                break;
            }
              
            case MyTAGS_TAG_TABLE:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_CAPTION, MyTAGS_CATEGORIES_SCOPE_TABLE) == myfalse) {
                    // parse error
                    break;
                }
                
                myhtml_tree_generate_implied_end_tags(tree, 0);
                
                //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                //if(current_node->tag_idx != MyTAGS_TAG_CAPTION) {
                //    // parse error
                //}
                
                myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_CAPTION, myfalse);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return mytrue;
            }
                
            case MyTAGS_TAG_BODY:
            case MyTAGS_TAG_COL:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_TH:
            case MyTAGS_TAG_THEAD:
            case MyTAGS_TAG_TR:
            {
                break;
            }
                
            default:
                return myhtml_insertion_mode_in_body(tree, token);
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_COL:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_TH:
            case MyTAGS_TAG_THEAD:
            case MyTAGS_TAG_TR:
            {
                if(myhtml_tree_element_in_scope(tree, MyTAGS_TAG_CAPTION, MyTAGS_CATEGORIES_SCOPE_TABLE) == myfalse) {
                    // parse error
                    break;
                }
                
                myhtml_tree_generate_implied_end_tags(tree, 0);
                
                //myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                //if(current_node->tag_idx != MyTAGS_TAG_CAPTION) {
                //    // parse error
                //}
                
                myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_CAPTION, myfalse);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return mytrue;
            }
                
            default:
                return myhtml_insertion_mode_in_body(tree, token);
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_column_group(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_COLGROUP:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node == NULL || current_node->tag_idx != MyTAGS_TAG_COLGROUP)
                    break;
                
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                break;
            }
                
            case MyTAGS_TAG_COL:
            {
                break;
            }
                
            case MyTAGS_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            default:
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, 0);
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token);
            }
                
            case MyTAGS_TAG_COL:
            {
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                break;
            }
                
            case MyTAGS_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token);
            }
                
            case MyTAGS_TAG__END_OF_FILE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            default:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node && current_node->tag_idx == MyTAGS_TAG_COLGROUP)
                {
                    myhtml_tree_open_elements_pop(tree);
                    
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                    return mytrue;
                }
                
                // parse error
                break;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_table_body(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_THEAD:
            {
                myhtml_tree_node_t* node = myhtml_tree_element_in_scope(tree, token->tag_ctx_idx, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_clear_stack_back_table_body_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                break;
            }
                
            case MyTAGS_TAG_TABLE:
            {
                myhtml_tree_node_t* tbody_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TBODY, MyTAGS_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* tfoot_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TFOOT, MyTAGS_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* thead_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_THEAD, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(tbody_node == NULL && tfoot_node == NULL && thead_node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_clear_stack_back_table_body_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return mytrue;
            }
               
            case MyTAGS_TAG_BODY:
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_COL:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TH:
            case MyTAGS_TAG_TR:
            {
                // parse error
                break;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG_TR:
            {
                myhtml_tree_clear_stack_back_table_body_context(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
                break;
            }
                
            case MyTAGS_TAG_TH:
            case MyTAGS_TAG_TD:
            {
                // parse error
                myhtml_tree_clear_stack_back_table_body_context(tree);
                
                myhtml_tree_node_insert(tree, MyTAGS_TAG_TR, MyHTML_NAMESPACE_HTML);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
                return mytrue;
            }
                
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_COL:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_THEAD:
            {
                myhtml_tree_node_t* tbody_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TBODY, MyTAGS_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* tfoot_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TFOOT, MyTAGS_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* thead_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_THEAD, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(tbody_node == NULL && tfoot_node == NULL && thead_node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_clear_stack_back_table_body_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return mytrue;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_row(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_TR:
            {
                myhtml_tree_node_t* tr_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TR, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(tr_node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_clear_stack_back_table_row_context(tree);
                
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                break;
            }
                
            case MyTAGS_TAG_TABLE:
            {
                myhtml_tree_node_t* tr_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TR, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(tr_node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_clear_stack_back_table_row_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return mytrue;
            }
                
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_THEAD:
            {
                myhtml_tree_node_t* node = myhtml_tree_element_in_scope(tree, token->tag_ctx_idx, MyTAGS_CATEGORIES_SCOPE_TABLE);
                if(node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_node_t* tr_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TR, MyTAGS_CATEGORIES_SCOPE_TABLE);
                if(tr_node == NULL)
                    break;
                
                myhtml_tree_clear_stack_back_table_row_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return mytrue;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG_TH:
            case MyTAGS_TAG_TD:
            {
                myhtml_tree_clear_stack_back_table_row_context(tree);
                
                myhtml_tree_node_t* current_node = myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_active_formatting_append(tree, current_node);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_CELL;
                break;
            }
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_COL:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_THEAD:
            case MyTAGS_TAG_TR:
            {
                myhtml_tree_node_t* tr_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TR, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(tr_node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_clear_stack_back_table_row_context(tree);
                myhtml_tree_open_elements_pop(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return mytrue;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_cell(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TH:
            {
                myhtml_tree_node_t* node = myhtml_tree_element_in_scope(tree, token->tag_ctx_idx, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_generate_implied_end_tags(tree, 0);
                
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->namespace != MyHTML_NAMESPACE_HTML ||
                   current_node->tag_idx != token->tag_ctx_idx)
                {
                    // parse error
                }
                
                myhtml_tree_open_elements_pop_until(tree, token->tag_ctx_idx, myfalse);
                
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
                break;
            }
                
            case MyTAGS_TAG_BODY:
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_COL:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_HTML:
                break;
                
            case MyTAGS_TAG_TABLE:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_THEAD:
            case MyTAGS_TAG_TR:
            {
                myhtml_tree_node_t* node = myhtml_tree_element_in_scope(tree, token->tag_ctx_idx, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_close_cell(tree, node);
                return mytrue;
            }
                
            default:
                return myhtml_insertion_mode_in_table(tree, token);
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_COL:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_TH:
            case MyTAGS_TAG_THEAD:
            case MyTAGS_TAG_TR:
            {
                myhtml_tree_node_t* td_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TD, MyTAGS_CATEGORIES_SCOPE_TABLE);
                myhtml_tree_node_t* th_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_TH, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(td_node == NULL && th_node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_close_cell(tree, (td_node == NULL ? th_node : td_node));
                
                return mytrue;
            }
                
            default:
                return myhtml_insertion_mode_in_body(tree, token);
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_select(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_OPTGROUP:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx == MyTAGS_TAG_OPTION)
                {
                    if(tree->open_elements->length > 1) {
                        if(tree->open_elements->list[ tree->open_elements->length - 2 ]->tag_idx == MyTAGS_TAG_OPTGROUP)
                            myhtml_tree_open_elements_pop(tree);
                    }
                    else
                        fprintf(stderr, "ERROR: in select state; open elements length < 2");
                }
                
                current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx == MyTAGS_TAG_OPTGROUP)
                    myhtml_tree_open_elements_pop(tree);
                else
                    // parse error
                    break;
                
                break;
            }
                
            case MyTAGS_TAG_OPTION:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx == MyTAGS_TAG_OPTION)
                    myhtml_tree_open_elements_pop(tree);
                else
                    // parse error
                    break;
                
                break;
            }
                
            case MyTAGS_TAG_SELECT:
            {
                // parse error
                myhtml_tree_node_t* select_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_SELECT, MyTAGS_CATEGORIES_SCOPE_SELECT);
                
                if(select_node == NULL)
                    // parse error
                    break;
                
                myhtml_tree_open_elements_pop_until_by_node(tree, select_node, myfalse);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                break;
            }
                
            case MyTAGS_TAG_TEMPLATE:
                return myhtml_insertion_mode_in_head(tree, token);
                
            default:
                // parse error
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
                myhtml_tree_node_insert_text(tree, token);
                break;
                
            case MyTAGS_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, NULL);
                break;
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyTAGS_TAG_OPTION:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx == token->tag_ctx_idx)
                    myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyTAGS_TAG_OPTGROUP:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx == MyTAGS_TAG_OPTION)
                    myhtml_tree_open_elements_pop(tree);
                
                current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx == token->tag_ctx_idx)
                    myhtml_tree_open_elements_pop(tree);
                
                myhtml_tree_node_insert_html_element(tree, token);
                break;
            }
                
            case MyTAGS_TAG_SELECT:
            {
                // parse error
                myhtml_tree_node_t* select_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_SELECT, MyTAGS_CATEGORIES_SCOPE_SELECT);
                
                if(select_node == NULL)
                    break;
                
                myhtml_tree_open_elements_pop_until_by_node(tree, select_node, myfalse);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                break;
            }
                
            case MyTAGS_TAG_INPUT:
            case MyTAGS_TAG_KEYGEN:
            case MyTAGS_TAG_TEXTAREA:
            {
                // parse error
                myhtml_tree_node_t* select_node = myhtml_tree_element_in_scope(tree, MyTAGS_TAG_SELECT, MyTAGS_CATEGORIES_SCOPE_SELECT);
                
                if(select_node == NULL)
                    break;
                
                myhtml_tree_open_elements_pop_until_by_node(tree, select_node, myfalse);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return mytrue;
            }
                
            case MyTAGS_TAG_SCRIPT:
            case MyTAGS_TAG_TEMPLATE:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyTAGS_TAG__END_OF_FILE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            default:
                // parse error
                break;
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_select_in_table(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_TABLE:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_THEAD:
            case MyTAGS_TAG_TR:
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TH:
            {
                // parse error
                myhtml_tree_node_t* some_node = myhtml_tree_element_in_scope(tree, token->tag_ctx_idx, MyTAGS_CATEGORIES_SCOPE_TABLE);
                
                if(some_node == NULL)
                    break;
                
                myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_SELECT, myfalse);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return mytrue;
            }
                
            default:
                return myhtml_insertion_mode_in_select(tree, token);
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_TABLE:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_THEAD:
            case MyTAGS_TAG_TR:
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TH:
            {
                // parse error
                myhtml_tree_open_elements_pop_until(tree, MyTAGS_TAG_SELECT, myfalse);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return mytrue;
            }
            
            default:
                return myhtml_insertion_mode_in_select(tree, token);
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_template(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_TEMPLATE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            default:
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            case MyTAGS_TAG__COMMENT:
            case MyTAGS_TAG__DOCTYPE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyTAGS_TAG_BASE:
            case MyTAGS_TAG_BASEFONT:
            case MyTAGS_TAG_BGSOUND:
            case MyTAGS_TAG_LINK:
            case MyTAGS_TAG_META:
            case MyTAGS_TAG_NOFRAMES:
            case MyTAGS_TAG_SCRIPT:
            case MyTAGS_TAG_STYLE:
            case MyTAGS_TAG_TEMPLATE:
            case MyTAGS_TAG_TITLE:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyTAGS_TAG_CAPTION:
            case MyTAGS_TAG_COLGROUP:
            case MyTAGS_TAG_TBODY:
            case MyTAGS_TAG_TFOOT:
            case MyTAGS_TAG_THEAD:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_TABLE);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                break;
                
            case MyTAGS_TAG_COL:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_COLUMN_GROUP);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_COLUMN_GROUP;
                break;
                
            case MyTAGS_TAG_TR:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_TABLE_BODY);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                break;
                
            case MyTAGS_TAG_TD:
            case MyTAGS_TAG_TH:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_ROW);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
                break;
                
            case MyTAGS_TAG__END_OF_FILE:
            {
                myhtml_tree_node_t* node = myhtml_tree_open_elements_find_by_tag_idx(tree, MyTAGS_TAG_TEMPLATE, NULL);
                
                if(node == NULL) {
                    myhtml_rules_stop_parsing(tree);
                    break;
                }
                
                // parse error
                myhtml_tree_open_elements_pop_until_by_node(tree, node, myfalse);
                myhtml_tree_active_formatting_up_to_last_marker(tree);
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_reset_insertion_mode_appropriately(tree);
                
                return mytrue;
            }
                
             default:
                myhtml_tree_template_insertion_pop(tree);
                myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_BODY);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                break;
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_body(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_HTML:
            {
                if(tree->fragment) {
                    // parse error
                    break;
                }
                
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_AFTER_BODY;
                break;
            }
                
            default:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return mytrue;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                    return myhtml_insertion_mode_in_body(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return mytrue;
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                if(tree->open_elements->length == 0) {
                    fprintf(stderr, "ERROR: after body state; open_elements length < 1\n");
                    break;
                }
                
                myhtml_tree_node_t* adjusted_location = tree->open_elements->list[0];
                
                // state 2
                myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
                
                node->tag_idx   = MyTAGS_TAG__COMMENT;
                node->token     = token;
                node->namespace = adjusted_location->namespace;
                
                myhtml_tree_node_add_child(tree, adjusted_location, node);
                
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                // parse error
                break;
                
            case MyTAGS_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyTAGS_TAG__END_OF_FILE:
                myhtml_rules_stop_parsing(tree);
                break;
                
            default:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return mytrue;
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_frameset(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_FRAMESET:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx == MyTAGS_TAG_HTML)
                    // parse error
                    break;
                
                myhtml_tree_open_elements_pop(tree);
                
                current_node = myhtml_tree_current_node(tree);
                
                if(tree->fragment == NULL &&
                   current_node->tag_idx != MyTAGS_TAG_FRAMESET)
                {
                    tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_FRAMESET;
                }
                
                break;
            }
                
            default:
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
                
                // parse error
                break;
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, NULL);
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                // parse error
                break;
                
            case MyTAGS_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyTAGS_TAG_FRAMESET:
                myhtml_tree_node_insert_html_element(tree, token);
                break;
                
            case MyTAGS_TAG_FRAME:
                myhtml_tree_node_insert_html_element(tree, token);
                myhtml_tree_open_elements_pop(tree);
                break;
                
            case MyTAGS_TAG_NOFRAMES:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyTAGS_TAG__END_OF_FILE:
            {
                myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
                
                if(current_node->tag_idx != MyTAGS_TAG_HTML) {
                    // parse error
                }
                
                myhtml_rules_stop_parsing(tree);
                break;
            }
                
            default:
                break;
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_frameset(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (token->tag_ctx_idx) {
            case MyTAGS_TAG_HTML:
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_AFTER_FRAMESET;
                break;
                
            default:
                break;
        }
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE) {
                    myhtml_tree_node_insert_text(tree, token);
                    break;
                }
                
                // parse error
                break;
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_node_insert_comment(tree, token, NULL);
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                // parse error
                break;
                
            case MyTAGS_TAG_HTML:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyTAGS_TAG_NOFRAMES:
                return myhtml_insertion_mode_in_head(tree, token);
                
            case MyTAGS_TAG__END_OF_FILE:
                myhtml_rules_stop_parsing(tree);
                break;
                
            default:
                break;
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_after_body(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE)
    {
        tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
        return mytrue;
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_node_t* adjusted_location = tree->document;
                myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
                
                node->tag_idx   = MyTAGS_TAG__COMMENT;
                node->token     = token;
                node->namespace = adjusted_location->namespace;
                
                myhtml_tree_node_add_child(tree, adjusted_location, node);
                break;
            }
                
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                    return myhtml_insertion_mode_in_body(tree, token);
                
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return mytrue;
            }
                
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG__DOCTYPE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyTAGS_TAG__END_OF_FILE:
                myhtml_rules_stop_parsing(tree);
                break;
                
            default:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return mytrue;
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_after_frameset(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE) {
        return myfalse;
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_node_t* adjusted_location = tree->document;
                myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
                
                node->tag_idx   = MyTAGS_TAG__COMMENT;
                node->token     = token;
                node->namespace = adjusted_location->namespace;
                
                myhtml_tree_node_add_child(tree, adjusted_location, node);
                break;
            }
                
            case MyTAGS_TAG__TEXT:
            {
                if(token->type & MyHTML_TOKEN_TYPE_WHITESPACE)
                    return myhtml_insertion_mode_in_body(tree, token);
                
                // parse error
                break;
            }
                
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG__DOCTYPE:
                return myhtml_insertion_mode_in_body(tree, token);
                
            case MyTAGS_TAG__END_OF_FILE:
                myhtml_rules_stop_parsing(tree);
                break;
                
            case MyTAGS_TAG_NOFRAMES:
                return myhtml_insertion_mode_in_head(tree, token);
                
            default:
                // parse error
                break;
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_foreign_content_end_other(myhtml_tree_t* tree, myhtml_tree_node_t* current_node, myhtml_token_node_t* token)
{
    if(current_node->tag_idx != token->tag_ctx_idx) {
        // parse error
    }
    
    if(tree->open_elements->length)
    {
        myhtml_tree_node_t** list = tree->open_elements->list;
        size_t i = tree->open_elements->length - 1;
        
        while (i > 1)
        {
            current_node = list[i];
            
            if(current_node->tag_idx == token->tag_ctx_idx) {
                myhtml_tree_open_elements_pop_until_by_node(tree, current_node, myfalse);
                return myfalse;
            }
            
            i--;
            
            if(list[i]->namespace == MyHTML_NAMESPACE_HTML)
                break;
        }
    }
    
    return tree->myhtml->insertion_func[tree->insert_mode](tree, token);
}

mybool_t myhtml_insertion_mode_in_foreign_content_start_other(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    myhtml_tree_node_t* adjusted_node = myhtml_tree_adjusted_current_node(tree);
    
    myhtml_token_node_wait_for_done(token);
    
    if(adjusted_node->namespace == MyHTML_NAMESPACE_MATHML) {
        myhtml_token_adjust_mathml_attributes(token);
    }
    else if(adjusted_node->namespace == MyHTML_NAMESPACE_SVG) {
        myhtml_token_adjust_svg_attributes(token);
    }
    
    myhtml_token_adjust_foreign_attributes(token);
    
    myhtml_tree_node_t* node = myhtml_tree_node_insert_foreign_element(tree, token);
    node->namespace = adjusted_node->namespace;
    
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF)
    {
        if(token->tag_ctx_idx == MyTAGS_TAG_SCRIPT &&
           node->namespace == MyHTML_NAMESPACE_SVG)
        {
            return myhtml_insertion_mode_in_foreign_content_end_other(tree, myhtml_tree_current_node(tree), token);
        }
        else {
            myhtml_tree_open_elements_pop(tree);
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_foreign_content(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE) {
        myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
        
        if(token->tag_ctx_idx == MyTAGS_TAG_SCRIPT &&
           current_node->tag_idx == MyTAGS_TAG_SCRIPT &&
           current_node->namespace == MyHTML_NAMESPACE_SVG)
        {
            myhtml_tree_open_elements_pop(tree);
            // TODO: now script is disable, skip this
            return myfalse;
        }
        
        return myhtml_insertion_mode_in_foreign_content_end_other(tree, current_node, token);
    }
    else {
        switch (token->tag_ctx_idx)
        {
            case MyTAGS_TAG__TEXT:
            {
                myhtml_tree_node_insert_text(tree, token);
                
                if((token->type & MyHTML_TOKEN_TYPE_WHITESPACE) == 0)
                    tree->flags ^= (tree->flags & MyHTML_TREE_FLAGS_FRAMESET_OK);
                
                break;
            }
                
            case MyTAGS_TAG__COMMENT:
                myhtml_tree_node_insert_comment(tree, token, NULL);
                break;
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_B:
            case MyTAGS_TAG_BIG:
            case MyTAGS_TAG_BLOCKQUOTE:
            case MyTAGS_TAG_BODY:
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_CENTER:
            case MyTAGS_TAG_CODE:
            case MyTAGS_TAG_DD:
            case MyTAGS_TAG_DIV:
            case MyTAGS_TAG_DL:
            case MyTAGS_TAG_DT:
            case MyTAGS_TAG_EM:
            case MyTAGS_TAG_EMBED:
            case MyTAGS_TAG_H1:
            case MyTAGS_TAG_H2:
            case MyTAGS_TAG_H3:
            case MyTAGS_TAG_H4:
            case MyTAGS_TAG_H5:
            case MyTAGS_TAG_H6:
            case MyTAGS_TAG_HEAD:
            case MyTAGS_TAG_HR:
            case MyTAGS_TAG_I:
            case MyTAGS_TAG_IMG:
            case MyTAGS_TAG_LI:
            case MyTAGS_TAG_LISTING:
            case MyTAGS_TAG_MENU:
            case MyTAGS_TAG_META:
            case MyTAGS_TAG_NOBR:
            case MyTAGS_TAG_OL:
            case MyTAGS_TAG_P:
            case MyTAGS_TAG_PRE:
            case MyTAGS_TAG_RUBY:
            case MyTAGS_TAG_S:
            case MyTAGS_TAG_SMALL:
            case MyTAGS_TAG_SPAN:
            case MyTAGS_TAG_STRONG:
            case MyTAGS_TAG_STRIKE:
            case MyTAGS_TAG_SUB:
            case MyTAGS_TAG_SUP:
            case MyTAGS_TAG_TABLE:
            case MyTAGS_TAG_TT:
            case MyTAGS_TAG_U:
            case MyTAGS_TAG_UL:
            case MyTAGS_TAG_VAR:
            case MyTAGS_TAG_FONT:
            {
                // parse error
                if(token->tag_ctx_idx == MyTAGS_TAG_FONT)
                {
                    myhtml_token_node_wait_for_done(token);
                    
                    if(myhtml_token_attr_by_name(token, "color", 5) == NULL &&
                       myhtml_token_attr_by_name(token, "face" , 4) == NULL &&
                       myhtml_token_attr_by_name(token, "size" , 4) == NULL)
                        return myhtml_insertion_mode_in_foreign_content_start_other(tree, token);
                }
                
                if(tree->fragment == NULL) {
                    myhtml_tree_node_t* current_node;
                    
                    do {
                        myhtml_tree_open_elements_pop(tree);
                        current_node = myhtml_tree_current_node(tree);
                    }
                    while(current_node && !(myhtml_tree_is_mathml_integration_point(tree, current_node) ||
                                            myhtml_tree_is_html_integration_point(tree, current_node) ||
                                            current_node->namespace == MyHTML_NAMESPACE_HTML));
                    
                    return mytrue;
                }
            }
                
            default:
                return myhtml_insertion_mode_in_foreign_content_start_other(tree, token);
        }
    }
    
    return myfalse;
}

void myhtml_rules_stop_parsing(myhtml_tree_t* tree)
{
    // THIS! IS! -(SPARTA!)- STOP PARSING
}

mybool_t myhtml_rules_tree_dispatcher(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    myhtml_tree_node_t* adjusted_node = myhtml_tree_adjusted_current_node(tree);
    
    if(tree->open_elements->length == 0 || adjusted_node->namespace == MyHTML_NAMESPACE_HTML) {
        return tree->myhtml->insertion_func[tree->insert_mode](tree, token);
    }
    else if(myhtml_tree_is_mathml_integration_point(tree, adjusted_node))
    {
        if(token->tag_ctx_idx == MyTAGS_TAG__TEXT ||
           (token->tag_ctx_idx != MyTAGS_TAG_MGLYPH && token->tag_ctx_idx != MyTAGS_TAG_MALIGNMARK)) {
            return tree->myhtml->insertion_func[tree->insert_mode](tree, token);
        }
    }
    
    if(adjusted_node->tag_idx == MyTAGS_TAG_ANNOTATION_XML &&
       adjusted_node->namespace == MyHTML_NAMESPACE_MATHML &&
       token->tag_ctx_idx != MyTAGS_TAG_SVG)
    {
        return tree->myhtml->insertion_func[tree->insert_mode](tree, token);
    }
    else if(myhtml_tree_is_html_integration_point(tree, adjusted_node) &&
            (token->type & MyHTML_TOKEN_TYPE_OPEN || token->tag_ctx_idx == MyTAGS_TAG__TEXT))
    {
        return tree->myhtml->insertion_func[tree->insert_mode](tree, token);
    }
    else if(token->tag_ctx_idx == MyTAGS_TAG__END_OF_FILE)
        return tree->myhtml->insertion_func[tree->insert_mode](tree, token);
    
    return myhtml_insertion_mode_in_foreign_content(tree, token);
}

myhtml_status_t myhtml_rules_init(myhtml_t* myhtml)
{
    myhtml->insertion_func = (myhtml_insertion_f*)mymalloc(sizeof(myhtml_insertion_f) * MyHTML_INSERTION_MODE_LAST_ENTRY);
    
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



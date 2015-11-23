//
//  myhtml_rules.c
//  myhtml
//
//  Created by Alexander Borisov on 25.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_rules.h"

myhtml_tree_index_t myhtml_rules_find_in_scope_tag_id(myhtml_tree_t* tree, myhtml_tree_index_t tree_idx, mytags_ctx_index_t tag_idx)
{
    myhtml_tree_node_t* nodes = tree->nodes;
    
    while(tree_idx && nodes[tree_idx].tag_idx != tag_idx) {
        tree_idx = nodes[tree_idx].parent;
    }
    
    return tree_idx;
}

myhtml_tree_index_t myhtml_rules_create_htree_node_by_token(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    myhtml_tree_node_t* nodes = tree->nodes;
    myhtml_tree_index_t htree_idx = myhtml_tree_node_create(tree);
    
    nodes[htree_idx].tag_idx = mh_token_get(token_idx, tag_ctx_idx);
    nodes[htree_idx].token = token_idx;
    
    return htree_idx;
}

myhtml_tree_index_t myhtml_rules_create_htree_node(myhtml_tree_t* tree, mytags_ctx_index_t tag_idx)
{
    myhtml_tree_node_t* nodes = tree->nodes;
    myhtml_tree_index_t htree_idx = myhtml_tree_node_create(tree);
    
    nodes[htree_idx].token = 0;
    nodes[htree_idx].tag_idx = tag_idx;
    
    return htree_idx;
}

mybool_t myhtml_insertion_mode_initial(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    //myhtml_t* myhtml = tree->myhtml;
    
    switch (mh_token_get(token_idx, tag_ctx_idx))
    {
        case MyTAGS_TAG__TEXT:
        {
            if(myhtml_token_is_whithspace(tree, token_idx)) {
                return myfalse;
            }
        }
            
        case MyTAGS_TAG__COMMENT:
        {
            myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
            myhtml_tree_node_add_child(tree, tree->document, htree_idx);
            
            return myfalse;
        }
            
        case MyTAGS_TAG__DOCTYPE:
        {
            myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
            myhtml_tree_node_add_child(tree, tree->document, htree_idx);
            
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

mybool_t myhtml_insertion_mode_before_html(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    if(mh_token_get(token_idx, type) & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (mh_token_get(token_idx, tag_ctx_idx)) {
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_HEAD:
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node(tree, MyTAGS_TAG_HTML);
                myhtml_tree_node_add_child(tree, tree->document, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                
                return mytrue;
            }
                
            default:
                break;
        }
    }
    else {
        switch (mh_token_get(token_idx, tag_ctx_idx))
        {
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->document, htree_idx);
                break;
            }
                
            case MyTAGS_TAG__TEXT:
            {
                if(myhtml_token_is_whithspace(tree, token_idx)) {
                    break;
                }
            }

            case MyTAGS_TAG_HTML:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->document, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                
                break;
            }
            
            default:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node(tree, MyTAGS_TAG_HTML);
                myhtml_tree_node_add_child(tree, tree->document, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                
                return mytrue;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_before_head(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    if(mh_token_get(token_idx, type) & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (mh_token_get(token_idx, tag_ctx_idx)) {
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_HEAD:
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node(tree, MyTAGS_TAG_HEAD);
                myhtml_tree_node_add_child(tree, tree->document, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                
                return mytrue;
            }
                
            default:
                break;
        }
    }
    else {
        switch (mh_token_get(token_idx, tag_ctx_idx))
        {
            case MyTAGS_TAG__TEXT:
            {
                if(myhtml_token_is_whithspace(tree, token_idx)) {
                    break;
                }
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token_idx);
            }
                
            case MyTAGS_TAG_HEAD:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                
                break;
            }
                
            default:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node(tree, MyTAGS_TAG_HEAD);
                myhtml_tree_node_add_child(tree, tree->document, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                
                return mytrue;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_head(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    if(mh_token_get(token_idx, type) & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (mh_token_get(token_idx, tag_ctx_idx)) {
            case MyTAGS_TAG_HEAD:
            {
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                break;
            }
                
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_BODY:
            {
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                return mytrue;
            }
                
            case MyTAGS_TAG_TEMPLATE:
            {
                //tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                break;
            }
                
            default:
                break;
        }
    }
    else {
        switch (mh_token_get(token_idx, tag_ctx_idx))
        {
            case MyTAGS_TAG__TEXT:
            {
                if(myhtml_token_is_whithspace(tree, token_idx))
                {
                    myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                    myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                    
                    break;
                }
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token_idx);
            }
                
            case MyTAGS_TAG_BASE:
            case MyTAGS_TAG_BASEFONT:
            case MyTAGS_TAG_BGSOUND:
            case MyTAGS_TAG_LINK:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                break;
            }
                
            case MyTAGS_TAG_META:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                break;
            }
                
            case MyTAGS_TAG_TITLE:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                
                break;
            }
                
            case MyTAGS_TAG_NOSCRIPT:
            {
                if(tree->flags & MyHTML_TREE_FLAGS_SCRIPT) {
                    myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                    myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                    
                    tree->current = htree_idx;
                    tree->orig_insert_mode = tree->insert_mode;
                    tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                }
                else {
                    myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                    myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                    
                    tree->current = htree_idx;
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD_NOSCRIPT;
                }
                
                break;
            }
            
            case MyTAGS_TAG_STYLE:
            case MyTAGS_TAG_NOFRAMES:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                
                break;
            }
                
            case MyTAGS_TAG_SCRIPT:
            {
                // TODO: s
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                
                break;
            }
                
            case MyTAGS_TAG_HEAD:
                break;
                
            default:
            {
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                return mytrue;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_head_noscript(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_head(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    if(mh_token_get(token_idx, type) & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (mh_token_get(token_idx, tag_ctx_idx)) {
            case MyTAGS_TAG_BR:
            case MyTAGS_TAG_HTML:
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node(tree, MyTAGS_TAG_BODY);
                myhtml_tree_node_insert_after(tree, tree->document, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                
                return mytrue;
            }
                
            case MyTAGS_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token_idx);
            }
                
            default:
                break;
        }
    }
    else {
        switch (mh_token_get(token_idx, tag_ctx_idx))
        {
            case MyTAGS_TAG__TEXT:
            {
                if(myhtml_token_is_whithspace(tree, token_idx))
                {
                    myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                    myhtml_tree_node_insert_after(tree, tree->current, htree_idx);
                    break;
                }
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_insert_after(tree, tree->current, htree_idx);
                
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
            {
                return myhtml_insertion_mode_in_body(tree, token_idx);
            }
                
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_insert_after(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                
                break;
            }
                
            case MyTAGS_TAG_FRAMESET:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_insert_after(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_FRAMESET;
                
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
                return myhtml_insertion_mode_in_head(tree, token_idx);
            }
                
            case MyTAGS_TAG_HEAD:
            {
                break;
            }
                
            default:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node(tree, MyTAGS_TAG_BODY);
                myhtml_tree_node_insert_after(tree, tree->document, htree_idx);
                
                tree->current = htree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                
                return mytrue;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_body(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    if(mh_token_get(token_idx, type) & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (mh_token_get(token_idx, tag_ctx_idx)) {
            case MyTAGS_TAG_TEMPLATE:
            {
                return myhtml_insertion_mode_in_head(tree, token_idx);
            }
                
            case MyTAGS_TAG_BODY:
            {
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, MyTAGS_TAG_BODY);
                
                if(tree_idx == 0) {
                    break;
                }
                
                tree->current = tree_idx;
                tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_BODY;
                
                break;
            }
                
            case MyTAGS_TAG_HTML:
            {
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, MyTAGS_TAG_BODY);
                
                if(tree_idx == 0) {
                    break;
                }
                
                tree->current = tree_idx;
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
                // TODO: If the stack of open elements does not have an element in scope that is an
                // HTML element with the same tag name as that of the token, then this is a parse error; ignore the token.
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, mh_token_get(token_idx, tag_ctx_idx));
                
                if(tree_idx == 0)
                    break;
                
                tree->current = myhtml_tree_node_get(tree, tree_idx, parent);
                break;
            }
               
            case MyTAGS_TAG_FORM:
            {
                // TODO: If there is no template element on the stack of open elements, then run these substeps:
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, mh_token_get(token_idx, tag_ctx_idx));
                
                if(tree_idx == 0)
                    break;
                
                tree->current = myhtml_tree_node_get(tree, tree_idx, parent);
                break;
            }
                
            case MyTAGS_TAG_P:
            {
                // TODO: If the stack of open elements does not have a p element in button scope,
                // then this is a parse error; insert an HTML element for a "p" start tag token with no attributes.
                // Close a p element.
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, mh_token_get(token_idx, tag_ctx_idx));
                
                if(tree_idx == 0)
                    break;
                
                tree->current = myhtml_tree_node_get(tree, tree_idx, parent);
                break;
            }
                
            case MyTAGS_TAG_LI:
            {
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, mh_token_get(token_idx, tag_ctx_idx));
                
                if(tree_idx == 0)
                    break;
                
                tree->current = myhtml_tree_node_get(tree, tree_idx, parent);
                
                // TODO: Otherwise, run these steps:
                break;
            }
               
            case MyTAGS_TAG_DT:
            case MyTAGS_TAG_DD:
            {
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, mh_token_get(token_idx, tag_ctx_idx));
                
                if(tree_idx == 0)
                    break;
                
                tree->current = myhtml_tree_node_get(tree, tree_idx, parent);
                
                // TODO: Otherwise, run these steps:
                break;
            }
                
            case MyTAGS_TAG_H1:
            case MyTAGS_TAG_H2:
            case MyTAGS_TAG_H3:
            case MyTAGS_TAG_H4:
            case MyTAGS_TAG_H5:
            case MyTAGS_TAG_H6:
            {
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, mh_token_get(token_idx, tag_ctx_idx));
                
                if(tree_idx == 0)
                    break;
                
                tree->current = myhtml_tree_node_get(tree, tree_idx, parent);
                
                // TODO: Otherwise, run these steps:
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
                // Run the adoption agency algorithm for the token's tag name.
                break;
            }
                
            case MyTAGS_TAG_APPLET:
            case MyTAGS_TAG_MARQUEE:
            case MyTAGS_TAG_OBJECT:
            {
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, mh_token_get(token_idx, tag_ctx_idx));
                
                if(tree_idx == 0)
                    break;
                
                // TODO: Otherwise, run these steps:
                break;
            }
                
            case MyTAGS_TAG_BR:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node(tree, MyTAGS_TAG_BR);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
            }
                
            default:
            {
                myhtml_tree_index_t tree_idx = myhtml_rules_find_in_scope_tag_id(tree, tree->current, mh_token_get(token_idx, tag_ctx_idx));
                
                if(tree_idx == 0)
                    break;
                
                tree->current = myhtml_tree_node_get(tree, tree_idx, parent);
                break;
            }
        }
    }
    else {
        switch (mh_token_get(token_idx, tag_ctx_idx))
        {
            case MyTAGS_TAG__TEXT:
            {
                // TODO: Reconstruct the active formatting elements, if any.
                if(myhtml_token_is_whithspace(tree, token_idx))
                {
                    myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                    myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                    break;
                }
                else {
                    myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                    myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                    break;
                }
            }
                
            case MyTAGS_TAG__COMMENT:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                break;
            }
                
            case MyTAGS_TAG__DOCTYPE:
                break;
                
            case MyTAGS_TAG_HTML:
            {
                // TODO: copy attrs to current html tag
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
                return myhtml_insertion_mode_in_head(tree, token_idx);
            }
                
            case MyTAGS_TAG_BODY:
            {
                // TODO: copy attrs to current html tag
                break;
            }
                
            case MyTAGS_TAG_FRAMESET:
            {
                // TODO: see spec
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
                // TODO: If the stack of open elements has a p element in button scope, then close a p element.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                break;
            }
                
            case MyTAGS_TAG_H1:
            case MyTAGS_TAG_H2:
            case MyTAGS_TAG_H3:
            case MyTAGS_TAG_H4:
            case MyTAGS_TAG_H5:
            case MyTAGS_TAG_H6:
            {
                // TODO: If the stack of open elements has a p element in button scope, then close a p element.
                // If the current node is an HTML element whose tag name is one of "h1", "h2", "h3", "h4", "h5", or "h6",
                // then this is a parse error; pop the current node off the stack of open elements.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                break;
            }
                
            case MyTAGS_TAG_PRE:
            case MyTAGS_TAG_LISTING:
            {
                // TODO: If the stack of open elements has a p element in button scope, then close a p element.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                break;
            }
                
            case MyTAGS_TAG_FORM:
            {
                // TODO: If the form element pointer is not null, and there is no template element on the stack of open elements,
                // then this is a parse error; ignore the token.
                // If the stack of open elements has a p element in button scope, then close a p element.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                break;
            }
                
            case MyTAGS_TAG_LI:
            {
                // TODO: Run these steps:
                break;
            }
            
            case MyTAGS_TAG_DT:
            case MyTAGS_TAG_DD:
            {
                // TODO: Run these steps:
                break;
            }
                
            case MyTAGS_TAG_PLAINTEXT:
            {
                // TODO: If the stack of open elements has a p element in button scope, then close a p element.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                break;
            }
                
            case MyTAGS_TAG_BUTTON:
            {
                // TODO: If the stack of open elements has a button element in scope, then run these substeps:
                break;
            }
               
            case MyTAGS_TAG_A:
            {
                // TODO: If the list of active formatting elements contains an a element between
                // the end of the list and the last marker on the list (or the start of the list
                // if there is no marker on the list), then this is a parse error; run the adoption agency algorithm
                // for the tag name "a", then remove that element from the list of active formatting elements and
                // the stack of open elements if the adoption agency algorithm didn't already remove it
                // (it might not have if the element is not in table scope).
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
                // TODO: Reconstruct the active formatting elements, if any.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                // TODO: Push onto the list of active formatting elements that element.
                
                tree->current = htree_idx;
                break;
            }
               
            case MyTAGS_TAG_NOBR:
            {
                // TODO: Reconstruct the active formatting elements, if any.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                // TODO: Push onto the list of active formatting elements that element.
                
                tree->current = htree_idx;
                break;
            }
                
            case MyTAGS_TAG_APPLET:
            case MyTAGS_TAG_MARQUEE:
            case MyTAGS_TAG_OBJECT:
            {
                // TODO: Reconstruct the active formatting elements, if any.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                // TODO: Insert a marker at the end of the list of active formatting elements.
                break;
            }
                
            case MyTAGS_TAG_TABLE:
            {
                // TODO: If the Document is not set to quirks mode, and the stack of open
                // elements has a p element in button scope, then close a p element.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
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
                // TODO: Reconstruct the active formatting elements, if any.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                break;
            }
                
            case MyTAGS_TAG_INPUT:
            {
                // TODO: Reconstruct the active formatting elements, if any.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                // TODO: If the token does not have an attribute with the name "type", or if it does,
                // but that attribute's value is not an ASCII case-insensitive match for the string "hidden",
                // then: set the frameset-ok flag to "not ok".
                break;
            }
                
            case MyTAGS_TAG_MENUITEM:
            case MyTAGS_TAG_PARAM:
            case MyTAGS_TAG_SOURCE:
            case MyTAGS_TAG_TRACK:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                break;
            }
                
            case MyTAGS_TAG_HR:
            {
                // TODO: If the stack of open elements has a p element in button scope, then close a p element.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                break;
            }
               
            case MyTAGS_TAG_IMAGE:
            {
                mh_token_set(token_idx, tag_ctx_idx) = MyTAGS_TAG_IMG;
                return mytrue;
            }
                
            case MyTAGS_TAG_ISINDEX:
            {
                // TODO: see spec
                break;
            }
                
            case MyTAGS_TAG_TEXTAREA:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                // If the next token is a U+000A LINE FEED (LF) character token, then
                // ignore that token and move on to the next one.
                // (Newlines at the start of textarea elements are ignored as an authoring convenience.)
                
                tree->current = htree_idx;
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_TEXT;
                
                break;
            }
                
            case MyTAGS_TAG_XMP:
            {
                // TODO: If the stack of open elements has a p element in button scope, then close a p element.
                // Reconstruct the active formatting elements, if any.
                // Follow the generic raw text element parsing algorithm.
                break;
            }
                
            case MyTAGS_TAG_IFRAME:
            {
                // Set the frameset-ok flag to "not ok".
                // Follow the generic raw text element parsing algorithm.
                break;
            }
                
            case MyTAGS_TAG_NOEMBED:
            {
                // Follow the generic raw text element parsing algorithm.
                break;
            }
                
            case MyTAGS_TAG_NOSCRIPT:
            {
                if(tree->flags & MyHTML_TREE_FLAGS_SCRIPT) {
                    // Follow the generic raw text element parsing algorithm.
                }
                
                break;
            }
                
            case MyTAGS_TAG_SELECT:
            {
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                // If the insertion mode is one of "in table", "in caption", "in table body", "in row", or "in cell",
                // then switch the insertion mode to "in select in table". Otherwise, switch the insertion mode to "in select".
                
                tree->current = htree_idx;
                tree->orig_insert_mode = tree->insert_mode;
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_SELECT;
                
                break;
            }
               
            case MyTAGS_TAG_OPTGROUP:
            case MyTAGS_TAG_OPTION:
            {
                // TODO: If the current node is an option element, then pop the current node off the stack of open elements.
                // Reconstruct the active formatting elements, if any.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                tree->orig_insert_mode = tree->insert_mode;
                
                break;
            }
                
            case MyTAGS_TAG_RB:
            case MyTAGS_TAG_RTC:
            {
                // TODO: If the stack of open elements has a ruby element in scope, then generate implied end tags.
                // If the current node is not now a ruby element, this is a parse error.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                tree->orig_insert_mode = tree->insert_mode;
                
                break;
            }
                
            case MyTAGS_TAG_RP:
            case MyTAGS_TAG_RT:
            {
                // TODO: If the stack of open elements has a ruby element in scope, then generate implied end tags,
                // except for rtc elements. If the current node is not now a rtc element or a ruby element, this is a parse error.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                tree->orig_insert_mode = tree->insert_mode;
                
                break;
            }
                
            case MyTAGS_TAG_MATH:
            {
                // TODO: Reconstruct the active formatting elements, if any.
                // Insert a foreign element for the token, in the MathML namespace.
                break;
            }
                
            case MyTAGS_TAG_SVG:
            {
                // TODO: Reconstruct the active formatting elements, if any.
                // Insert a foreign element for the token, in the MathML namespace.
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
                // Ignore the token.
                break;
            }
                
            default:
            {
                // TODO: Reconstruct the active formatting elements, if any.
                myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
                myhtml_tree_node_add_child(tree, tree->current, htree_idx);
                
                tree->current = htree_idx;
                break;
            }
        }
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_text(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    if(mh_token_get(token_idx, type) & MyHTML_TOKEN_TYPE_CLOSE)
    {
        switch (mh_token_get(token_idx, tag_ctx_idx)) {
            case MyTAGS_TAG_SCRIPT:
            {
                tree->current = myhtml_tree_node_get(tree, tree->current, parent);
                tree->insert_mode = tree->orig_insert_mode;
                break;
            }
                
            default:
            {
                tree->current = myhtml_tree_node_get(tree, tree->current, parent);
                tree->insert_mode = tree->orig_insert_mode;
                break;
            }
        }
    }
    else {
        myhtml_tree_index_t htree_idx = myhtml_rules_create_htree_node_by_token(tree, token_idx);
        myhtml_tree_node_add_child(tree, tree->current, htree_idx);
    }
    
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_table(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_table_text(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_caption(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_column_group(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_table_body(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_row(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_cell(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_select(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_select_in_table(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_template(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_body(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_in_frameset(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_frameset(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_after_body(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

mybool_t myhtml_insertion_mode_after_after_frameset(myhtml_tree_t* tree, myhtml_token_index_t token_idx)
{
    return myfalse;
}

void myhtml_rules_init(myhtml_t* myhtml)
{
    myhtml->insertion_func = (myhtml_insertion_f*)malloc(sizeof(myhtml_insertion_f) * MyHTML_INSERTION_MODE_LAST_ENTRY);
    
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
}



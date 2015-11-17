//
//  myhtml_token.c
//  myhtml
//
//  Created by Alexander Borisov on 26.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_token.h"

void myhtml_token_callback_for_new(mcobject_t* mcobject, size_t idx)
{
    myhtml_token_node_t* nodes = (myhtml_token_node_t*)mcobject->nodes;
    
    nodes[idx].entry.data   = NULL;
    nodes[idx].entry.length = 0;
    nodes[idx].entry.size   = 0;
}

myhtml_token_t * myhtml_token_create(size_t size)
{
    myhtml_token_t* token = (myhtml_token_t*)malloc(sizeof(myhtml_token_t));
    
    token->nodes_obj   = mcobject_create(size, sizeof(myhtml_token_node_t), &token->nodes, myhtml_token_callback_for_new);
    token->attr_obj    = mcobject_create((size * 2), sizeof(myhtml_token_attr_t), &token->attr, NULL);
    
    myhtml_token_clean(token);
    
    return token;
}

void myhtml_token_clean(myhtml_token_t* token)
{
    mcobject_clean(token->nodes_obj);
    mcobject_clean(token->attr_obj);
    
    size_t id_clean = mcobject_malloc(token->nodes_obj);
    myhtml_token_node_clean(&token->nodes[id_clean]);
    
    // first (0) is NULL for all
    id_clean = mcobject_malloc(token->attr_obj);
    myhtml_token_attr_clean(&token->attr[id_clean]);
}

myhtml_token_t * myhtml_token_destroy(myhtml_token_t* token)
{
    if(token == NULL)
        return NULL;
    
    size_t i;
    for(i = 0; i < mcobject_length(token->nodes_obj); i++) {
        myhtml_string_destroy(&token->nodes[i].entry, myfalse);
    }
    
    if(token->nodes_obj)
        token->nodes_obj = mcobject_destroy(token->nodes_obj);
    
    if(token->attr_obj)
        token->attr_obj = mcobject_destroy(token->attr_obj);
    
    return NULL;
}

void myhtml_token_node_clean(myhtml_token_node_t* node)
{
    node->tag_ctx_idx    = MyTAGS_TAG__TEXT;
    node->type           = MyHTML_TOKEN_TYPE_OPEN;
    node->attr_first     = 0;
    node->attr_last      = 0;
    node->begin          = 0;
    node->length         = 0;
    
    myhtml_string_clean(&node->entry);
}

void myhtml_token_attr_clean(myhtml_token_attr_t* attr)
{
    attr->next         = 0;
    attr->prev         = 0;
    attr->name_begin   = 0;
    attr->name_length  = 0;
    attr->value_begin  = 0;
    attr->value_length = 0;
}

void myhtml_token_delete(myhtml_token_t* token, myhtml_token_index_t idx)
{
    mcobject_free(token->nodes_obj, idx);
}

mybool_t myhtml_token_is_whithspace(myhtml_tree_t* tree, myhtml_token_index_t idx)
{
    const char* html = mh_token_get(idx, entry).data;
    
    size_t i;
    for (i = mh_token_get(idx, begin); i < mh_token_get(idx, length); i++)
    {
        // TODO: see and use myhtml_whithspace
        // U+0009 // U+000A // U+000C // U+000D // U+0020
        if(html[i] != '\t' && html[i] != '\n' && html[i] != '\f' && html[i] != '\r' && html[i] != ' ') {
            return myfalse;
        }
    }
    
    return mytrue;
}

void myhtml_token_print_param_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_index_t idx, FILE* out)
{
    myhtml_token_node_t* token = &myhtml_tree->token->nodes[idx];
    
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE) {
        fprintf(out, "</");
    }
    else {
        fprintf(out, "<");
    }
    
    fprintf(out, "tag_ctx_idx=%lu; body_begin=%lu; body_length=%lu; attr_first=%lu; attr_last=%lu",
            token->tag_ctx_idx, token->begin, token->length,
            token->attr_first, token->attr_last);
    
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF) {
        fprintf(out, " />\n");
    }
    else {
        fprintf(out, ">\n");
    }
}

void myhtml_token_print_by_idx(myhtml_tree_t* tree, myhtml_token_index_t idx, FILE* out)
{
    myhtml_t* myhtml            = tree->myhtml;
    mctree_node_t* mctree_nodes = myhtml->tags->tree->nodes;
    myhtml_token_node_t* token  = &tree->token->nodes[idx];
    
    size_t mctree_id = mh_tags_get(token->tag_ctx_idx, mctree_id);
    size_t tag_name_size = mctree_nodes[mctree_id].str_size;
    
    if(token->tag_ctx_idx == MyTAGS_TAG__TEXT ||
       token->tag_ctx_idx == MyTAGS_TAG__COMMENT)
    {
        if(token->length) {
            fprintf(out, "%.*s: %.*s\n", (int)tag_name_size, mctree_nodes[mctree_id].str,
                    (int)token->length, &token->entry.data[token->begin]);
        }
        else {
            fprintf(out, "%.*s is empty\n", (int)tag_name_size, mctree_nodes[mctree_id].str);
        }
    }
    else
    {
        if(token->type & MyHTML_TOKEN_TYPE_CLOSE) {
            fprintf(out, "</");
        }
        else {
            fprintf(out, "<");
        }
        
        fprintf(out, "%.*s tagid=\"%lu\"", (int)tag_name_size, mctree_nodes[mctree_id].str,
                mh_tags_get(token->tag_ctx_idx, id));
        
        myhtml_token_print_attr(tree, idx, out);
        
        if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF) {
            fprintf(out, " />\n");
        }
        else {
            fprintf(out, ">\n");
        }
    }
}

void myhtml_token_print_attr(myhtml_tree_t* tree, myhtml_token_index_t idx, FILE* out)
{
    myhtml_token_node_t* token  = &tree->token->nodes[idx];
    const char* html = token->entry.data;
    
    size_t attr_id = token->attr_first;
    
    while(attr_id)
    {
        myhtml_token_attr_t* attr = &myhtml_token_attr(tree->token, attr_id);
        
        fprintf(out, " ");
        
        if(attr->value_begin)
        {
            fprintf(out, "%.*s=\"%.*s\"",
                    (int)attr->name_length, &html[attr->name_begin],
                    (int)attr->value_length, &html[attr->value_begin]);
        }
        else {
            fprintf(out, "%.*s", (int)attr->name_length, &html[attr->name_begin]);
        }
        
        attr_id = attr->next;
    }
}



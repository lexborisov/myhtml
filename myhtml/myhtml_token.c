//
//  myhtml_token.c
//  myhtml
//
//  Created by Alexander Borisov on 26.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_token.h"


myhtml_token_t * myhtml_token_create(size_t size)
{
    myhtml_token_t* token = (myhtml_token_t*)malloc(sizeof(myhtml_token_t));
    
    token->strings_obj = mcobject_create(4096, sizeof(myhtml_string_t), &token->strings);
    token->nodes_obj   = mcobject_create(size, sizeof(myhtml_token_node_t), &token->nodes);
    token->attr_obj    = mcobject_create((token->nodes_obj->nodes_size * 2), sizeof(myhtml_token_node_t), &token->attr);
    
    myhtml_token_clean(token);
    
    return token;
}

void myhtml_token_clean(myhtml_token_t* token)
{
    myhtml_token_stings_clean(token);
    
    mcobject_clean(token->nodes_obj);
    mcobject_clean(token->attr_obj);
    
    size_t id_clean = mcobject_malloc(token->nodes_obj);
    myhtml_token_node_clean(&token->nodes[id_clean], 0);
    
    // first (0) is NULL for all
    id_clean = mcobject_malloc(token->attr_obj);
    myhtml_token_attr_clean(&token->attr[id_clean]);
}

myhtml_token_t * myhtml_token_destroy(myhtml_token_t* token)
{
    if(token == NULL)
        return NULL;
    
    myhtml_token_stings_destroy(token);
    
    if(token->nodes_obj)
        token->nodes_obj = mcobject_destroy(token->nodes_obj);
    
    if(token->attr_obj)
        token->attr_obj = mcobject_destroy(token->attr_obj);
    
    return NULL;
}

void myhtml_token_node_clean(myhtml_token_node_t* node, size_t tag_begin)
{
    node->tag_ctx_idx    = MyTAGS_TAG__TEXT;
    node->tagname_begin  = 0;
    node->tagname_length = 0;
    node->tag_begin      = tag_begin;
    node->tag_length     = 0;
    node->type           = MyHTML_TOKEN_TYPE_OPEN;
    node->attr_first     = 0;
    node->attr_last      = 0;
    node->string_idx     = 0;
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

void myhtml_token_stings_clean(myhtml_token_t* token)
{
    //    size_t i;
    //    for(i = 0; i < tree->strings_obj->nodes_length; i++) {
    //        myhtml_string_clean(&tree->strings[i]);
    //    }
    //
    mcobject_clean(token->strings_obj);
}

void myhtml_token_stings_destroy(myhtml_token_t* token)
{
    size_t i;
    for(i = 0; i < token->strings_obj->nodes_length; i++) {
        myhtml_string_destroy(&token->strings[i], myfalse);
    }
    
    mcobject_destroy(token->strings_obj);
}

void myhtml_token_delete(myhtml_token_t* token, myhtml_token_index_t idx)
{
    mcobject_free(token->nodes_obj, idx);
}

mybool_t myhtml_token_is_whithspace(myhtml_tree_t* tree, myhtml_token_index_t idx)
{
//    const char* html = tree->html;
//    
//    size_t i;
//    for (i = mh_token_get(idx, tag_begin); i < mh_token_get(idx, tag_length); i++)
//    {
//        // TODO: see and use myhtml_whithspace
//        // U+0009 // U+000A // U+000C // U+000D // U+0020
//        if(html[i] != '\t' && html[i] != '\n' && html[i] != '\f' && html[i] != '\r' && html[i] != ' ') {
//            return myfalse;
//        }
//    }
    
    return mytrue;
}

void myhtml_token_print_param_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_index_t idx, FILE* out)
{
    myhtml_token_node_t* token = &myhtml_tree->token->nodes[idx];
    
    //mctree_node_t* mctree_nodes = myhtml->tags->tree->nodes;
    //size_t mctree_id = mh_tags_get(nodes[idx].tag_ctx_idx, mctree_id);
    //size_t tag_name_size = mctree_nodes[mctree_id].str_size;
    
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE) {
        fprintf(out, "</");
    }
    else {
        fprintf(out, "<");
    }
    
    fprintf(out, "tag_ctx_idx=%lu; tag_begin=%lu; tag_length=%lu; tagname_begin=%lu; tagname_length=%lu; attr_first=%lu; attr_last=%lu",
            token->tag_ctx_idx, token->tag_begin, token->tag_length,
            token->tagname_begin, token->tagname_length,
            token->attr_first, token->attr_last);
    
    //myhtml_print_queue_attr_by_last_tree(myhtml, myhtml_tree, idx, out);
    
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF) {
        fprintf(out, " />\n");
    }
    else {
        fprintf(out, ">\n");
    }
}

void myhtml_token_print_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_index_t idx, FILE* out)
{
    myhtml_t* myhtml            = myhtml_tree->myhtml;
    mctree_node_t* mctree_nodes = myhtml->tags->tree->nodes;
    myhtml_token_node_t* token  = &myhtml_tree->token->nodes[idx];
    
    size_t mctree_id = mh_tags_get(token->tag_ctx_idx, mctree_id);
    size_t tag_name_size = mctree_nodes[mctree_id].str_size;
    
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE) {
        fprintf(out, "</");
    }
    else {
        fprintf(out, "<");
    }
    
    fprintf(out, "%.*s tagid=\"%lu\"", (int)tag_name_size, mctree_nodes[mctree_id].str,
            mh_tags_get(token->tag_ctx_idx, id));
    
    //myhtml_token_print_attr(myhtml_tree, idx, out);
    
    if(token->type & MyHTML_TOKEN_TYPE_CLOSE_SELF) {
        fprintf(out, " />\n");
    }
    else {
        fprintf(out, ">\n");
    }
}

void myhtml_token_print_attr(myhtml_tree_t* myhtml_tree, myhtml_token_index_t idx, FILE* out)
{
//    myhtml_token_node_t* token  = &myhtml_tree->token->nodes[idx];
//    myhtml_token_attr_t* attrs  = myhtml_tree->token->attr;
//    
//    size_t attr_id = token->attr_first;
//    
//    while(attr_id)
//    {
//        fprintf(out, " ");
//        
//        if(attrs[attr_id].value_begin)
//        {
//            fprintf(out, "%.*s=\"%.*s\"",
//                    (int)attrs[attr_id].name_length, &(myhtml_tree->html[attrs[attr_id].name_begin]),
//                    (int)attrs[attr_id].value_length, &(myhtml_tree->html[attrs[attr_id].value_begin]));
//        }
//        else {
//            fprintf(out, "%.*s", (int)attrs[attr_id].name_length, &(myhtml_tree->html[attrs[attr_id].name_begin]));
//        }
//        
//        attr_id = attrs[attr_id].next;
//    }
}



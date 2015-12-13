//
//  myhtml_token.c
//  myhtml
//
//  Created by Alexander Borisov on 26.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_token.h"

void myhtml_token_callback_for_new(mcobject_async_t* mcobj_async, size_t idx)
{
    myhtml_token_node_t* nodes = (myhtml_token_node_t*)mcobj_async->mem;
    
    nodes[idx].entry.data   = NULL;
    nodes[idx].entry.length = 0;
    nodes[idx].entry.size   = 0;
}

myhtml_token_t * myhtml_token_create(size_t size)
{
    myhtml_token_t* token = (myhtml_token_t*)mymalloc(sizeof(myhtml_token_t));
    
    token->nodes_obj = mcobject_async_create(256, size, sizeof(myhtml_token_node_t));
    token->attr_obj  = mcobject_async_create(256, size, sizeof(myhtml_token_attr_t));
    
    mcobject_async_node_add(token->nodes_obj, 4096);
    mcobject_async_node_add(token->attr_obj , 4096);
    
    return token;
}

void myhtml_token_clean(myhtml_token_t* token)
{
    mcobject_async_clean(token->nodes_obj);
    mcobject_async_clean(token->attr_obj);
}

myhtml_token_t * myhtml_token_destroy(myhtml_token_t* token)
{
    if(token == NULL)
        return NULL;
    
//    size_t i;
//    for(i = 0; i < token->nodes_obj->mem_length; i++) {
//        myhtml_string_destroy(&token->nodes[i].entry, myfalse);
//    }
    
    if(token->nodes_obj)
        token->nodes_obj = mcobject_async_destroy(token->nodes_obj, 1);
    
    if(token->attr_obj)
        token->attr_obj = mcobject_async_destroy(token->attr_obj, 1);
    
    return NULL;
}

void myhtml_token_node_clean(myhtml_token_node_t* node)
{
    node->tag_ctx_idx    = MyTAGS_TAG__TEXT;
    node->type           = MyHTML_TOKEN_TYPE_OPEN;
    node->attr_first     = NULL;
    node->attr_last      = NULL;
    node->begin          = 0;
    node->length         = 0;
    node->is_done        = myfalse;
    
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

void myhtml_token_node_wait_for_done(myhtml_token_node_t* node)
{
    while(node->is_done == myfalse) {}
}

myhtml_token_node_t * myhtml_token_clone(myhtml_token_t* token, myhtml_token_node_t* node, size_t thread_idx)
{
    if(node == NULL)
        return NULL;
    
    myhtml_token_node_t* new_node = mcobject_async_malloc(token->nodes_obj, thread_idx);
    
    new_node->tag_ctx_idx = node->tag_ctx_idx;
    new_node->type        = node->type;
    new_node->attr_first  = 0;
    new_node->attr_last   = 0;
    new_node->begin       = node->begin;
    new_node->length      = node->length;
    
    myhtml_string_init(&new_node->entry, node->entry.size);
    myhtml_token_attr_copy(token, node, new_node, thread_idx);
    
    return new_node;
}

// TODO: use tree for this
void myhtml_token_attr_copy(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_node_t* dest, size_t thread_idx)
{
    myhtml_string_t* targ_string = &target->entry;
    myhtml_string_t* dest_string = &dest->entry;
    
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr) {
        myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx);
        new_attr->next = 0;
        
        if(attr->name_length)
        {
            new_attr->name_begin = myhtml_string_len(dest_string);
            new_attr->name_length = attr->name_length;
            
            myhtml_string_append_lowercase_with_null(dest_string,
                                                     &targ_string->data[attr->name_begin],
                                                     attr->name_length);
        }
        else {
            attr->name_begin  = 0;
            attr->name_length = 0;
        }
        
        if(attr->value_length)
        {
            new_attr->value_begin = myhtml_string_len(dest_string);
            new_attr->value_length = attr->value_length;
            
            myhtml_string_append_lowercase_with_null(dest_string,
                                                     &targ_string->data[attr->value_begin],
                                                     attr->value_length);
        }
        else {
            attr->value_begin  = 0;
            attr->value_length = 0;
        }
        
        if(dest->attr_first == NULL) {
            new_attr->prev = 0;
            
            dest->attr_first = new_attr;
            dest->attr_last = new_attr;
        }
        else {
            dest->attr_last->next = new_attr;
            new_attr->prev = dest->attr_last;
            
            dest->attr_last = new_attr;
        }
        
        attr = attr->next;
    }
}

mybool_t myhtml_token_attr_compare(myhtml_token_node_t* target, myhtml_token_node_t* dest)
{
    if(target == NULL || dest == NULL)
        return myfalse;
    
    char* target_char = target->entry.data;
    char* dest_char   = dest->entry.data;
    
    myhtml_token_attr_t* target_attr = target->attr_first;
    myhtml_token_attr_t* dest_attr   = dest->attr_first;
    
    while (target_attr && dest_attr)
    {
        if(target_attr->name_length == dest_attr->name_length &&
           target_attr->value_length == dest_attr->value_length)
        {
            if(strcmp(&target_char[target_attr->name_begin], &dest_char[dest_attr->name_begin]) != 0)
                break;
            
            if(strcasecmp(&target_char[target_attr->value_begin], &dest_char[dest_attr->value_begin]) != 0)
                break;
        }
        else
            break;
        
        target_attr = target_attr->next;
        dest_attr   = dest_attr->next;
    }
    
    if(target_attr == NULL && dest_attr == NULL)
        return mytrue;
    
    return myfalse;
}

myhtml_token_attr_t * myhtml_token_attr_by_name(myhtml_token_t* token, myhtml_token_node_t* node, const char* name, size_t name_length)
{
    myhtml_token_attr_t* attr = node->attr_first;
    char* buff = node->entry.data;
    
    while (attr)
    {
        if(name_length == attr->name_length) {
            if(strncasecmp(name, &buff[attr->name_begin], name_length) == 0)
                break;
        }
        
        attr = attr->next;
    }
    
    return attr;
}

void myhtml_token_delete(myhtml_token_t* token, myhtml_token_node_t* node)
{
    mcobject_async_free(token->nodes_obj, 0, node);
}

mybool_t myhtml_token_is_whithspace(myhtml_tree_t* tree, myhtml_token_node_t* node)
{
    const char* html = node->entry.data;
    
    size_t i;
    for (i = node->begin; i < node->length; i++)
    {
        // TODO: see and use myhtml_whithspace
        // U+0009 // U+000A // U+000C // U+000D // U+0020
        if(html[i] != '\t' && html[i] != '\n' && html[i] != '\f' && html[i] != '\r' && html[i] != ' ') {
            return myfalse;
        }
    }
    
    return mytrue;
}

void myhtml_token_print_param_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_node_t* node, FILE* out)
{
    if(node->type & MyHTML_TOKEN_TYPE_CLOSE) {
        fprintf(out, "</");
    }
    else {
        fprintf(out, "<");
    }
    
    fprintf(out, "tag_ctx_idx=%lu; body_begin=%lu; body_length=%lu; attr_first=%lu; attr_last=%lu",
            node->tag_ctx_idx, node->begin, node->length,
            (size_t)node->attr_first, (size_t)node->attr_last);
    
    if(node->type & MyHTML_TOKEN_TYPE_CLOSE_SELF) {
        fprintf(out, " />\n");
    }
    else {
        fprintf(out, ">\n");
    }
}

void myhtml_token_print_by_idx(myhtml_tree_t* tree, myhtml_token_node_t* node, FILE* out)
{
    myhtml_t* myhtml            = tree->myhtml;
    mctree_node_t* mctree_nodes = myhtml->tags->tree->nodes;
    
    size_t mctree_id = mh_tags_get(node->tag_ctx_idx, mctree_id);
    size_t tag_name_size = mctree_nodes[mctree_id].str_size;
    
    if(node->tag_ctx_idx == MyTAGS_TAG__TEXT ||
       node->tag_ctx_idx == MyTAGS_TAG__COMMENT)
    {
        if(node->length) {
            fprintf(out, "%.*s: %.*s\n", (int)tag_name_size, mctree_nodes[mctree_id].str,
                    (int)node->length, &node->entry.data[node->begin]);
        }
        else {
            fprintf(out, "%.*s is empty\n", (int)tag_name_size, mctree_nodes[mctree_id].str);
        }
    }
    else
    {
        if(node->type & MyHTML_TOKEN_TYPE_CLOSE) {
            fprintf(out, "</");
        }
        else {
            fprintf(out, "<");
        }
        
        fprintf(out, "%.*s tagid=\"%lu\"", (int)tag_name_size, mctree_nodes[mctree_id].str,
                mh_tags_get(node->tag_ctx_idx, id));
        
        myhtml_token_print_attr(tree, node, out);
        
        if(node->type & MyHTML_TOKEN_TYPE_CLOSE_SELF) {
            fprintf(out, " />\n");
        }
        else {
            fprintf(out, ">\n");
        }
    }
}

void myhtml_token_print_attr(myhtml_tree_t* tree, myhtml_token_node_t* node, FILE* out)
{
    const char* html = node->entry.data;
    
    myhtml_token_attr_t* attr = node->attr_first;
    
    while(attr)
    {
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
        
        attr = attr->next;
    }
}



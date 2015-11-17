//
//  myhtml_token.h
//  myhtml
//
//  Created by Alexander Borisov on 26.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_token_h
#define myhtml_token_h

#include "myosi.h"
#include "mytags.h"
#include "myhtml.h"
#include "mcobject.h"
#include "myhtml_string.h"

// attr
#define myhtml_token_attr(__token__, __idx__) __token__->attr[__idx__]
#define myhtml_token_attr_get(__token__, __idx__, __patam__) myhtml_token_attr(__token__, __idx__).__patam__

#define myhtml_token_attr_malloc(__token__, __attr_idx__)             \
    __attr_idx__ = mcobject_malloc(__token__->attr_obj)

// nodes
#define myhtml_token_node_get(__token__, __idx__, __patam__) __token__->nodes[__idx__].__patam__

#define myhtml_token_node_malloc(__token__, __node_idx__)                          \
    __node_idx__ = mcobject_malloc(__token__->nodes_obj);                          \
    myhtml_token_node_clean(&__token__->nodes[__node_idx__])

struct myhtml_token_attr {
    size_t next;
    size_t prev;
    
    size_t name_begin;
    size_t name_length;
    size_t value_begin;
    size_t value_length;
};

struct myhtml_token_node {
    mytags_ctx_index_t tag_ctx_idx;
    
    myhtml_string_t entry;
    
    size_t begin;
    size_t length;
    
    size_t attr_first;
    size_t attr_last;
    
    enum myhtml_token_type type;
};

struct myhtml_token {
    myhtml_token_node_t* nodes;
    mcobject_t* nodes_obj; // myhtml_token_node_t
    
    myhtml_token_attr_t* attr;
    mcobject_t* attr_obj;  // myhtml_token_attr_t
};

myhtml_token_t * myhtml_token_create(size_t size);
void myhtml_token_clean(myhtml_token_t* token);
myhtml_token_t * myhtml_token_destroy(myhtml_token_t* token);

void myhtml_token_node_clean(myhtml_token_node_t* node);
void myhtml_token_attr_clean(myhtml_token_attr_t* attr);

void myhtml_token_delete(myhtml_token_t* token, myhtml_token_index_t idx);

mybool_t myhtml_token_is_whithspace(myhtml_tree_t* tree, myhtml_token_index_t idx);

void myhtml_token_print_param_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_index_t idx, FILE* out);
void myhtml_token_print_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_index_t idx, FILE* out);
void myhtml_token_print_attr(myhtml_tree_t* myhtml_tree, myhtml_token_index_t idx, FILE* out);

#endif /* myhtml_token_h */

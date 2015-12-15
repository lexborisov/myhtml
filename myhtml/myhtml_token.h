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
#include "mcobject_async.h"
#include "myhtml_string.h"
#include "mcsync.h"

// attr
#define myhtml_token_attr(__token__, __idx__) __token__->attr[__idx__]
#define myhtml_token_attr_get(__token__, __idx__, __patam__) myhtml_token_attr(__token__, __idx__).__patam__

#define myhtml_token_attr_malloc(__token__, __attr_idx__)             \
    __attr_idx__ = mcobject_async_malloc(__token__->attr_obj, 0)

// nodes
#define myhtml_token_node_get(__token__, __idx__, __patam__) __token__->nodes[__idx__].__patam__

#define myhtml_token_node_malloc(__token__, __token_node__)                                 \
    __token_node__ = (myhtml_token_node_t*)mcobject_async_malloc(__token__->nodes_obj, 0);  \
    myhtml_token_node_clean(__token_node__)

struct myhtml_token_replacement_entry {
    char* from;
    size_t from_size;
    
    char* to;
    size_t to_size;
};

struct myhtml_token_namespace_replacement {
    char* from;
    size_t from_size;
    
    char* to;
    size_t to_size;
    
    enum myhtml_namespace namespace;
};

struct myhtml_token_attr {
    myhtml_token_attr_t* next;
    myhtml_token_attr_t* prev;
    
    size_t name_begin;
    size_t name_length;
    size_t value_begin;
    size_t value_length;
    
    enum myhtml_namespace namespace;
};

struct myhtml_token_node {
    mytags_ctx_index_t tag_ctx_idx;
    
    myhtml_string_t entry;
    
    size_t begin;
    size_t length;
    
    myhtml_token_attr_t* attr_first;
    myhtml_token_attr_t* attr_last;
    
    enum myhtml_token_type type;
    volatile mybool_t is_done;
};

struct myhtml_token {
    mcobject_async_t* nodes_obj; // myhtml_token_node_t
    mcobject_async_t* attr_obj;  // myhtml_token_attr_t
};

myhtml_token_t * myhtml_token_create(size_t size);
void myhtml_token_clean(myhtml_token_t* token);
myhtml_token_t * myhtml_token_destroy(myhtml_token_t* token);

void myhtml_token_node_clean(myhtml_token_node_t* node);
void myhtml_token_attr_clean(myhtml_token_attr_t* attr);
void myhtml_token_delete(myhtml_token_t* token, myhtml_token_node_t* node);
void myhtml_token_node_wait_for_done(myhtml_token_node_t* node);

mybool_t myhtml_token_is_whithspace(myhtml_tree_t* tree, myhtml_token_node_t* node);

myhtml_token_node_t * myhtml_token_clone(myhtml_token_t* token, myhtml_token_node_t* node, size_t thread_idx);
myhtml_token_attr_t * myhtml_token_attr_match(myhtml_token_t* token, myhtml_token_node_t* target, const char* key, size_t key_size, const char* value, size_t value_size);
myhtml_token_attr_t * myhtml_token_attr_match_case(myhtml_token_t* token, myhtml_token_node_t* target, const char* key, size_t key_size, const char* value, size_t value_size);

void myhtml_token_adjust_mathml_attributes(myhtml_token_node_t* target);
void myhtml_token_adjust_svg_attributes(myhtml_token_node_t* target);
void myhtml_token_adjust_foreign_attributes(myhtml_token_node_t* target);

void myhtml_token_attr_copy(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_node_t* dest, size_t thread_idx);
myhtml_token_attr_t * myhtml_token_attr_by_name(myhtml_token_node_t* node, const char* name, size_t name_size);
mybool_t myhtml_token_attr_compare(myhtml_token_node_t* target, myhtml_token_node_t* dest);

void myhtml_token_print_param_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_node_t* node, FILE* out);
void myhtml_token_print_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_node_t* node, FILE* out);
void myhtml_token_print_attr(myhtml_tree_t* myhtml_tree, myhtml_token_node_t* node, FILE* out);


#endif /* myhtml_token_h */

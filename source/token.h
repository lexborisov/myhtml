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

#ifndef MyHTML_TOKEN_H
#define MyHTML_TOKEN_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "myosi.h"
#include "utils.h"
#include "tag.h"
#include "myhtml.h"
#include "mystring.h"
#include "utils/mcobject_async.h"
#include "utils/mchar_async.h"
#include "utils/mcsync.h"

// attr
#define myhtml_token_attr(__token__, __idx__) __token__->attr[__idx__]
#define myhtml_token_attr_get(__token__, __idx__, __patam__) myhtml_token_attr(__token__, __idx__).__patam__

#define myhtml_token_attr_malloc(__token__, __attr_idx__, __thread_idx__)                  \
    __attr_idx__ = mcobject_async_malloc(__token__->attr_obj, __thread_idx__, NULL);       \
    myhtml_token_attr_clean(__attr_idx__)

// nodes
#define myhtml_token_node_get(__token__, __idx__, __patam__) __token__->nodes[__idx__].__patam__

#define myhtml_token_node_set_done(__token_node__) __token_node__->type |= MyHTML_TOKEN_TYPE_DONE

#define myhtml_token_node_malloc(__token__, __token_node__, __thread_id__)                                    \
    __token_node__ = (myhtml_token_node_t*)mcobject_async_malloc(__token__->nodes_obj, __thread_id__, NULL);  \
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
    
    enum myhtml_namespace my_namespace;
};

struct myhtml_token_attr {
    myhtml_token_attr_t* next;
    myhtml_token_attr_t* prev;
    
    myhtml_string_t entry;
    
    size_t name_begin;
    size_t name_length;
    size_t value_begin;
    size_t value_length;
    
    enum myhtml_namespace my_namespace;
};

struct myhtml_token_node {
    myhtml_tag_id_t tag_ctx_idx;
    
    myhtml_string_t my_str_tm;
    
    size_t begin;
    size_t length;
    
    myhtml_token_attr_t* attr_first;
    myhtml_token_attr_t* attr_last;
    
    volatile enum myhtml_token_type type;
};

struct myhtml_token {
    myhtml_tree_t* tree; // ref
    
    mcobject_async_t* nodes_obj; // myhtml_token_node_t
    mcobject_async_t* attr_obj;  // myhtml_token_attr_t
    
    // def thread node id
    size_t mcasync_token_id;
    size_t mcasync_attr_id;
    
    mybool_t is_new_tmp;
};

myhtml_token_t * myhtml_token_create(myhtml_tree_t* tree, size_t size);
void myhtml_token_clean(myhtml_token_t* token);
void myhtml_token_clean_all(myhtml_token_t* token);
myhtml_token_t * myhtml_token_destroy(myhtml_token_t* token);

void myhtml_token_node_clean(myhtml_token_node_t* node);
void myhtml_token_attr_clean(myhtml_token_attr_t* attr);
myhtml_token_attr_t * myhtml_token_attr_remove(myhtml_token_node_t* node, myhtml_token_attr_t* attr);
myhtml_token_attr_t * myhtml_token_attr_remove_by_name(myhtml_token_node_t* node, const char* name, size_t name_length);
void myhtml_token_attr_delete_all(myhtml_token_t* token, myhtml_token_node_t* node);
void myhtml_token_delete(myhtml_token_t* token, myhtml_token_node_t* node);
void myhtml_token_node_wait_for_done(myhtml_token_node_t* node);

mybool_t myhtml_token_is_whithspace(myhtml_tree_t* tree, myhtml_token_node_t* node);

myhtml_token_attr_t * myhtml_token_attr_match(myhtml_token_t* token, myhtml_token_node_t* target, const char* key, size_t key_size, const char* value, size_t value_size);
myhtml_token_attr_t * myhtml_token_attr_match_case(myhtml_token_t* token, myhtml_token_node_t* target, const char* key, size_t key_size, const char* value, size_t value_size);

mybool_t myhtml_token_release_and_check_doctype_attributes(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_tree_doctype_t* return_doctype);

void myhtml_token_adjust_mathml_attributes(myhtml_token_node_t* target);
void myhtml_token_adjust_svg_attributes(myhtml_token_node_t* target);
void myhtml_token_adjust_foreign_attributes(myhtml_token_node_t* target);

myhtml_token_attr_t * myhtml_token_node_attr_append(myhtml_token_t* token, myhtml_token_node_t* dest, const char* key, size_t key_len, const char* value, size_t value_len, size_t thread_idx);
void myhtml_token_node_text_append(myhtml_token_t* token, myhtml_token_node_t* dest, const char* text, size_t text_len);
void myhtml_token_node_attr_copy(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_node_t* dest, size_t thread_idx);
myhtml_token_node_t * myhtml_token_node_clone(myhtml_token_t* token, myhtml_token_node_t* node, size_t token_thread_idx, size_t attr_thread_idx);
mybool_t myhtml_token_attr_copy(myhtml_token_t* token, myhtml_token_attr_t* attr, myhtml_token_node_t* dest, size_t thread_idx);
myhtml_token_attr_t * myhtml_token_attr_by_name(myhtml_token_node_t* node, const char* name, size_t name_size);
mybool_t myhtml_token_attr_compare(myhtml_token_node_t* target, myhtml_token_node_t* dest);

void myhtml_token_print_param_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_node_t* node, FILE* out);
void myhtml_token_print_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_node_t* node, FILE* out);
void myhtml_token_print_attr(myhtml_tree_t* myhtml_tree, myhtml_token_node_t* node, FILE* out);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_token_h */

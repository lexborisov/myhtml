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

#ifndef MyHTML_TOKEN_H
#define MyHTML_TOKEN_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "myhtml/myosi.h"
#include "mycore/utils.h"
#include "myhtml/tag.h"
#include "myhtml/myhtml.h"
#include "myhtml/mystring.h"
#include "mycore/utils/mcobject_async.h"
#include "mycore/utils/mchar_async.h"
#include "mycore/utils/mcsync.h"

#define myhtml_token_node_set_done(token_node) token_node->type |= MyHTML_TOKEN_TYPE_DONE

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
    
    enum myhtml_namespace ns;
};

struct myhtml_token_attr {
    myhtml_token_attr_t* next;
    myhtml_token_attr_t* prev;
    
    mycore_string_t key;
    mycore_string_t value;
    
    size_t raw_key_begin;
    size_t raw_key_length;
    size_t raw_value_begin;
    size_t raw_value_length;
    
    enum myhtml_namespace ns;
};

struct myhtml_token_node {
    myhtml_tag_id_t tag_id;
    
    mycore_string_t str;
    
    size_t raw_begin;
    size_t raw_length;
    
    size_t element_begin;
    size_t element_length;
    
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
    
    bool is_new_tmp;
};

myhtml_token_t * myhtml_token_create(myhtml_tree_t* tree, size_t size);
void myhtml_token_clean(myhtml_token_t* token);
void myhtml_token_clean_all(myhtml_token_t* token);
myhtml_token_t * myhtml_token_destroy(myhtml_token_t* token);

myhtml_tag_id_t myhtml_token_node_tag_id(myhtml_token_node_t *token_node);
myhtml_position_t myhtml_token_node_raw_position(myhtml_token_node_t *token_node);
myhtml_position_t myhtml_token_node_element_position(myhtml_token_node_t *token_node);

myhtml_tree_attr_t * myhtml_token_node_attribute_first(myhtml_token_node_t *token_node);
myhtml_tree_attr_t * myhtml_token_node_attribute_last(myhtml_token_node_t *token_node);

const char * myhtml_token_node_text(myhtml_token_node_t *token_node, size_t *length);
mycore_string_t * myhtml_token_node_string(myhtml_token_node_t *token_node);

bool myhtml_token_node_is_close(myhtml_token_node_t *token_node);
bool myhtml_token_node_is_close_self(myhtml_token_node_t *token_node);

myhtml_token_node_t * myhtml_token_node_create(myhtml_token_t* token, size_t async_node_id);
void myhtml_token_node_clean(myhtml_token_node_t* node);

myhtml_token_attr_t * myhtml_token_attr_create(myhtml_token_t* token, size_t async_node_id);
void myhtml_token_attr_clean(myhtml_token_attr_t* attr);
myhtml_token_attr_t * myhtml_token_attr_remove(myhtml_token_node_t* node, myhtml_token_attr_t* attr);
myhtml_token_attr_t * myhtml_token_attr_remove_by_name(myhtml_token_node_t* node, const char* name, size_t name_length);
void myhtml_token_attr_delete_all(myhtml_token_t* token, myhtml_token_node_t* node);

void myhtml_token_delete(myhtml_token_t* token, myhtml_token_node_t* node);
void myhtml_token_node_wait_for_done(myhtml_token_t* token, myhtml_token_node_t* node);
void myhtml_token_set_done(myhtml_token_node_t* node);

myhtml_token_attr_t * myhtml_token_attr_match(myhtml_token_t* token, myhtml_token_node_t* target, const char* key, size_t key_size, const char* value, size_t value_size);
myhtml_token_attr_t * myhtml_token_attr_match_case(myhtml_token_t* token, myhtml_token_node_t* target, const char* key, size_t key_size, const char* value, size_t value_size);

bool myhtml_token_release_and_check_doctype_attributes(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_tree_doctype_t* return_doctype);

void myhtml_token_adjust_mathml_attributes(myhtml_token_node_t* target);
void myhtml_token_adjust_svg_attributes(myhtml_token_node_t* target);
void myhtml_token_adjust_foreign_attributes(myhtml_token_node_t* target);

myhtml_token_attr_t * myhtml_token_node_attr_append(myhtml_token_t* token, myhtml_token_node_t* dest, const char* key, size_t key_len, const char* value, size_t value_len, size_t thread_idx);
myhtml_token_attr_t * myhtml_token_node_attr_append_with_convert_encoding(myhtml_token_t* token, myhtml_token_node_t* dest, const char* key, size_t key_len, const char* value, size_t value_len, size_t thread_idx, myencoding_t encoding);
void myhtml_token_node_text_append(myhtml_token_t* token, myhtml_token_node_t* dest, const char* text, size_t text_len);
void myhtml_token_node_attr_copy(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_node_t* dest, size_t thread_idx);
void myhtml_token_node_attr_copy_with_check(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_node_t* dest, size_t thread_idx);
myhtml_token_node_t * myhtml_token_node_clone(myhtml_token_t* token, myhtml_token_node_t* node, size_t token_thread_idx, size_t attr_thread_idx);
bool myhtml_token_attr_copy(myhtml_token_t* token, myhtml_token_attr_t* attr, myhtml_token_node_t* dest, size_t thread_idx);
myhtml_token_attr_t * myhtml_token_attr_by_name(myhtml_token_node_t* node, const char* name, size_t name_size);
bool myhtml_token_attr_compare(myhtml_token_node_t* target, myhtml_token_node_t* dest);
myhtml_token_node_t * myhtml_token_merged_two_token_string(myhtml_tree_t* tree, myhtml_token_node_t* token_to, myhtml_token_node_t* token_from, bool cp_reverse);
void myhtml_token_set_replacement_character_for_null_token(myhtml_tree_t* tree, myhtml_token_node_t* node);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_token_h */

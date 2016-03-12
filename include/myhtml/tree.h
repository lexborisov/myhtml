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

#ifndef MyHTML_TREE_H
#define MyHTML_TREE_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/myhtml.h"
#include "myhtml/mystring.h"
#include "myhtml/token.h"
#include "myhtml/utils/mcsync.h"
#include "myhtml/utils/mchar_async.h"
#include "myhtml/utils/mcobject_async.h"

#define myhtml_tree_get(__tree__, __attr__) __tree__->__attr__
#define myhtml_tree_set(__tree__, __attr__) __tree__->__attr__

#define myhtml_tree_token_current(__tree__) myhtml_tree_get(__tree__, token_current)
#define myhtml_tree_token_attr_current(__tree__) myhtml_tree_get(__tree__, attr_current)

#define myhtml_tree_node_get(__tree__, __node_id__, __attr__) __tree__->nodes[__node_id__].__attr__

enum myhtml_tree_node_type {
    MyHTML_TYPE_NONE    = 0,
    MyHTML_TYPE_BLOCK   = 1,
    MyHTML_TYPE_INLINE  = 2,
    MyHTML_TYPE_TABLE   = 3,
    MyHTML_TYPE_META    = 4,
    MyHTML_TYPE_COMMENT = 5
};

enum myhtml_close_type {
    MyHTML_CLOSE_TYPE_NONE  = 0,
    MyHTML_CLOSE_TYPE_NOW   = 1,
    MyHTML_CLOSE_TYPE_SELF  = 2,
    MyHTML_CLOSE_TYPE_BLOCK = 3
};

enum myhtml_tree_node_flags {
    MyHTML_TREE_NODE_UNDEF           = 0,
    MyHTML_TREE_NODE_PARSER_INSERTED = 1,
    MyHTML_TREE_NODE_BLOCKING        = 2
};

struct myhtml_tree_node {
    enum myhtml_tree_node_flags flags;
    
    myhtml_tag_id_t tag_idx;
    enum myhtml_namespace my_namespace;
    
    myhtml_tree_node_t* prev;                // предыдущий ид элемента этого же уровня
    myhtml_tree_node_t* next;                // следеющий ид эелента этого же уровня
    myhtml_tree_node_t* child;               // ид чилда
    myhtml_tree_node_t* parent;              // ид родителя
    
    myhtml_tree_node_t* last_child;          //
    
    myhtml_token_node_t* token;
};

enum myhtml_tree_compat_mode {
    MyHTML_TREE_COMPAT_MODE_NO_QUIRKS       = 0x00,
    MyHTML_TREE_COMPAT_MODE_QUIRKS          = 0x01,
    MyHTML_TREE_COMPAT_MODE_LIMITED_QUIRKS  = 0x02
};

enum myhtml_tree_doctype_id {
    MyHTML_TREE_DOCTYPE_ID_NAME   = 0x00,
    MyHTML_TREE_DOCTYPE_ID_SYSTEM = 0x01,
    MyHTML_TREE_DOCTYPE_ID_PUBLIC = 0x02
};

enum myhtml_tree_insertion_mode {
    MyHTML_TREE_INSERTION_MODE_DEFAULT     = 0x00,
    MyHTML_TREE_INSERTION_MODE_BEFORE      = 0x01,
    MyHTML_TREE_INSERTION_MODE_AFTER       = 0x02
};

struct myhtml_async_args {
    size_t mchar_node_id;
};

struct myhtml_tree_doctype {
    bool is_html;
    char* attr_name;
    char* attr_public;
    char* attr_system;
};

struct myhtml_tree_indexes {
    myhtml_tag_index_t* tags;
};

struct myhtml_tree_list {
    myhtml_tree_node_t** list;
    volatile size_t length;
    size_t size;
};

struct myhtml_tree_token_list {
    myhtml_token_node_t** list;
    size_t length;
    size_t size;
};

struct myhtml_tree_insertion_list {
    enum myhtml_insertion_mode* list;
    size_t length;
    size_t size;
};

struct myhtml_tree_temp_tag_name {
    char   *data;
    size_t  length;
    size_t  size;
};

struct myhtml_tree_special_token {
    myhtml_token_node_t *token;
    myhtml_namespace_t my_namespace;
}
typedef myhtml_tree_special_token_t;

struct myhtml_tree_special_token_list {
    myhtml_tree_special_token_t *list;
    size_t  length;
    size_t  size;
}
typedef myhtml_tree_special_token_list_t;

struct myhtml_tree_temp_stream {
    struct myhtml_tree_temp_tag_name** data;
    size_t length;
    size_t size;
    
    myhtml_encoding_result_t res;
    struct myhtml_tree_temp_tag_name* current;
};

struct myhtml_tree {
    // ref
    myhtml_t*             myhtml;
    mchar_async_t*        mchar;
    myhtml_token_t*       token;
    mcobject_async_t*     tree_obj;
    mcsync_t*             sync;
    mythread_queue_t*     queue;
    mythread_queue_t*     single_queue;
    myhtml_tag_t*         tags;
    
    // init id's
    size_t                  mcasync_token_id;
    size_t                  mcasync_attr_id;
    size_t                  mcasync_tree_id;
    size_t                  mchar_node_id;
    size_t                  mcasync_incoming_buf_id;
    myhtml_token_attr_t*    attr_current;
    myhtml_tag_id_t         tmp_tag_id;
    mythread_queue_node_t*  current_qnode;
    myhtml_incoming_buf_t*  incoming_buf;
    myhtml_incoming_buf_t*  incoming_buf_first;
    
    myhtml_tree_indexes_t* indexes;
    
    // ref for nodes
    myhtml_tree_node_t*   document;
    myhtml_tree_node_t*   fragment;
    myhtml_tree_node_t*   node_head;
    myhtml_tree_node_t*   node_html;
    myhtml_tree_node_t*   node_body;
    myhtml_tree_node_t*   node_form;
    myhtml_tree_doctype_t doctype;
    
    // for build tree
    myhtml_tree_list_t*           active_formatting;
    myhtml_tree_list_t*           open_elements;
    myhtml_tree_list_t*           other_elements;
    myhtml_tree_token_list_t*     token_list;
    myhtml_tree_insertion_list_t* template_insertion;
    myhtml_async_args_t*          async_args;
    myhtml_tree_temp_stream_t*    temp_stream;
    volatile myhtml_token_node_t* token_last_done;
    
    // for detect namespace out of tree builder
    myhtml_token_node_t*          token_namespace;
    
    // tree params
    enum myhtml_tokenizer_state     state;
    enum myhtml_tokenizer_state     state_of_builder;
    enum myhtml_insertion_mode      insert_mode;
    enum myhtml_insertion_mode      orig_insert_mode;
    enum myhtml_tree_compat_mode    compat_mode;
    volatile enum myhtml_tree_flags flags;
    bool                        foster_parenting;
    size_t                          global_offset;
    
    myhtml_encoding_t            encoding;
    myhtml_encoding_t            encoding_usereq;
    myhtml_tree_temp_tag_name_t  temp_tag_name;
};

// base
myhtml_tree_t * myhtml_tree_create(void);
myhtml_status_t myhtml_tree_init(myhtml_tree_t* tree, myhtml_t* myhtml);
void myhtml_tree_clean(myhtml_tree_t* tree);
void myhtml_tree_clean_all(myhtml_tree_t* tree);
myhtml_tree_t * myhtml_tree_destroy(myhtml_tree_t* tree);

myhtml_t * myhtml_tree_get_myhtml(myhtml_tree_t* tree);
myhtml_tag_t * myhtml_tree_get_tag(myhtml_tree_t* tree);
myhtml_tag_index_t * myhtml_tree_get_tag_index(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_tree_get_document(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_tree_get_node_html(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_tree_get_node_body(myhtml_tree_t* tree);

mchar_async_t * myhtml_tree_get_mchar(myhtml_tree_t* tree);
size_t myhtml_tree_get_mchar_node_id(myhtml_tree_t* tree);

// list
myhtml_tree_list_t * myhtml_tree_list_init(void);
void myhtml_tree_list_clean(myhtml_tree_list_t* list);
myhtml_tree_list_t * myhtml_tree_list_destroy(myhtml_tree_list_t* list, bool destroy_self);

void myhtml_tree_list_append(myhtml_tree_list_t* list, myhtml_tree_node_t* node);
void myhtml_tree_list_append_after_index(myhtml_tree_list_t* list, myhtml_tree_node_t* node, size_t index);
void myhtml_tree_list_insert_by_index(myhtml_tree_list_t* list, myhtml_tree_node_t* node, size_t index);
myhtml_tree_node_t * myhtml_tree_list_current_node(myhtml_tree_list_t* list);

// token list
myhtml_tree_token_list_t * myhtml_tree_token_list_init(void);
void myhtml_tree_token_list_clean(myhtml_tree_token_list_t* list);
myhtml_tree_token_list_t * myhtml_tree_token_list_destroy(myhtml_tree_token_list_t* list, bool destroy_self);

void myhtml_tree_token_list_append(myhtml_tree_token_list_t* list, myhtml_token_node_t* token);
void myhtml_tree_token_list_append_after_index(myhtml_tree_token_list_t* list, myhtml_token_node_t* token, size_t index);
myhtml_token_node_t * myhtml_tree_token_list_current_node(myhtml_tree_token_list_t* list);

// active formatting
myhtml_tree_list_t * myhtml_tree_active_formatting_init(myhtml_tree_t* tree);
void myhtml_tree_active_formatting_clean(myhtml_tree_t* tree);
myhtml_tree_list_t * myhtml_tree_active_formatting_destroy(myhtml_tree_t* tree);

bool myhtml_tree_active_formatting_is_marker(myhtml_tree_t* tree, myhtml_tree_node_t* idx);
myhtml_tree_node_t* myhtml_tree_active_formatting_between_last_marker(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, size_t* return_idx);

void myhtml_tree_active_formatting_append(myhtml_tree_t* tree, myhtml_tree_node_t* node);
void myhtml_tree_active_formatting_append_with_check(myhtml_tree_t* tree, myhtml_tree_node_t* node);
void myhtml_tree_active_formatting_pop(myhtml_tree_t* tree);
void myhtml_tree_active_formatting_remove(myhtml_tree_t* tree, myhtml_tree_node_t* node);
void myhtml_tree_active_formatting_remove_by_index(myhtml_tree_t* tree, size_t idx);

void myhtml_tree_active_formatting_reconstruction(myhtml_tree_t* tree);
void myhtml_tree_active_formatting_up_to_last_marker(myhtml_tree_t* tree);

bool myhtml_tree_active_formatting_find(myhtml_tree_t* tree, myhtml_tree_node_t* idx, size_t* return_idx);
myhtml_tree_node_t* myhtml_tree_active_formatting_current_node(myhtml_tree_t* tree);

// open elements
myhtml_tree_list_t * myhtml_tree_open_elements_init(myhtml_tree_t* tree);
void myhtml_tree_open_elements_clean(myhtml_tree_t* tree);
myhtml_tree_list_t * myhtml_tree_open_elements_destroy(myhtml_tree_t* tree);

myhtml_tree_node_t* myhtml_tree_current_node(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_tree_adjusted_current_node(myhtml_tree_t* tree);

void myhtml_tree_open_elements_append(myhtml_tree_t* tree, myhtml_tree_node_t* node);
void myhtml_tree_open_elements_append_after_index(myhtml_tree_t* tree, myhtml_tree_node_t* node, size_t index);
void myhtml_tree_open_elements_pop(myhtml_tree_t* tree);
void myhtml_tree_open_elements_pop_until(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, myhtml_namespace_t mynamespace, bool is_exclude);
void myhtml_tree_open_elements_pop_until_by_node(myhtml_tree_t* tree, myhtml_tree_node_t* node_idx, bool is_exclude);
void myhtml_tree_open_elements_pop_until_by_index(myhtml_tree_t* tree, size_t idx, bool is_exclude);
void myhtml_tree_open_elements_remove(myhtml_tree_t* tree, myhtml_tree_node_t* node);

bool myhtml_tree_open_elements_find(myhtml_tree_t* tree, myhtml_tree_node_t* idx, size_t* pos);
bool myhtml_tree_open_elements_find_reverse(myhtml_tree_t* tree, myhtml_tree_node_t* idx, size_t* pos);
myhtml_tree_node_t * myhtml_tree_open_elements_find_by_tag_idx(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, myhtml_namespace_t mynamespace, size_t* return_index);
myhtml_tree_node_t * myhtml_tree_open_elements_find_by_tag_idx_reverse(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, myhtml_namespace_t mynamespace, size_t* return_index);
myhtml_tree_node_t * myhtml_tree_element_in_scope(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, myhtml_namespace_t mynamespace, enum myhtml_tag_categories category);
bool myhtml_tree_element_in_scope_by_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, enum myhtml_tag_categories category);
void myhtml_tree_generate_implied_end_tags(myhtml_tree_t* tree, myhtml_tag_id_t exclude_tag_idx, myhtml_namespace_t mynamespace);
void myhtml_tree_generate_all_implied_end_tags(myhtml_tree_t* tree, myhtml_tag_id_t exclude_tag_idx, myhtml_namespace_t mynamespace);
myhtml_tree_node_t * myhtml_tree_appropriate_place_inserting(myhtml_tree_t* tree, myhtml_tree_node_t* override_target, enum myhtml_tree_insertion_mode* mode);
myhtml_tree_node_t * myhtml_tree_appropriate_place_inserting_in_tree(myhtml_tree_t* tree, myhtml_tree_node_t* target, enum myhtml_tree_insertion_mode* mode);

// template insertion
myhtml_tree_insertion_list_t * myhtml_tree_template_insertion_init(myhtml_tree_t* tree);
void myhtml_tree_template_insertion_clean(myhtml_tree_t* tree);
myhtml_tree_insertion_list_t * myhtml_tree_template_insertion_destroy(myhtml_tree_t* tree);

void myhtml_tree_template_insertion_append(myhtml_tree_t* tree, enum myhtml_insertion_mode insert_mode);
void myhtml_tree_template_insertion_pop(myhtml_tree_t* tree);

void myhtml_tree_reset_insertion_mode_appropriately(myhtml_tree_t* tree);

bool myhtml_tree_adoption_agency_algorithm(myhtml_tree_t* tree, myhtml_tag_id_t subject_tag_idx);
size_t myhtml_tree_template_insertion_length(myhtml_tree_t* tree);

// other for a tree
myhtml_tree_node_t * myhtml_tree_node_create(myhtml_tree_t* tree);
void myhtml_tree_node_delete(myhtml_tree_t* tree, myhtml_tree_node_t* node);
void myhtml_tree_node_delete_recursive(myhtml_tree_t* tree, myhtml_tree_node_t* node);
void myhtml_tree_node_clean(myhtml_tree_node_t* tree_node);
void myhtml_tree_node_free(myhtml_tree_t* tree, myhtml_tree_node_t* node);
myhtml_tree_node_t * myhtml_tree_node_clone(myhtml_tree_t* tree, myhtml_tree_node_t* node);

void myhtml_tree_print_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, FILE* out);
void myhtml_tree_print_node_childs(myhtml_tree_t* tree, myhtml_tree_node_t* node, FILE* out, size_t inc);
void myhtml_tree_print_by_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, FILE* out, size_t inc);

void myhtml_tree_node_add_child(myhtml_tree_t* myhtml_tree, myhtml_tree_node_t* root, myhtml_tree_node_t* node);
void myhtml_tree_node_insert_before(myhtml_tree_t* myhtml_tree, myhtml_tree_node_t* root, myhtml_tree_node_t* node);
void myhtml_tree_node_insert_after(myhtml_tree_t* myhtml_tree, myhtml_tree_node_t* root, myhtml_tree_node_t* node);
void myhtml_tree_node_insert_by_mode(myhtml_tree_t* tree, myhtml_tree_node_t* adjusted_location, myhtml_tree_node_t* node, enum myhtml_tree_insertion_mode mode);
myhtml_tree_node_t * myhtml_tree_node_remove(myhtml_tree_node_t* node);

myhtml_tree_node_t * myhtml_tree_node_insert_html_element(myhtml_tree_t* tree, myhtml_token_node_t* token);
myhtml_tree_node_t * myhtml_tree_node_insert_foreign_element(myhtml_tree_t* tree, myhtml_token_node_t* token);
myhtml_tree_node_t * myhtml_tree_node_insert_by_token(myhtml_tree_t* tree, myhtml_token_node_t* token, enum myhtml_namespace my_namespace);
myhtml_tree_node_t * myhtml_tree_node_insert(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, enum myhtml_namespace my_namespace);
myhtml_tree_node_t * myhtml_tree_node_insert_by_node(myhtml_tree_t* tree, myhtml_tree_node_t* idx);
myhtml_tree_node_t * myhtml_tree_node_insert_comment(myhtml_tree_t* tree, myhtml_token_node_t* token, myhtml_tree_node_t* parent);
myhtml_tree_node_t * myhtml_tree_node_insert_doctype(myhtml_tree_t* tree, myhtml_token_node_t* token);
myhtml_tree_node_t * myhtml_tree_node_insert_root(myhtml_tree_t* tree, myhtml_token_node_t* token, enum myhtml_namespace my_namespace);
myhtml_tree_node_t * myhtml_tree_node_insert_text(myhtml_tree_t* tree, myhtml_token_node_t* token);
myhtml_tree_node_t * myhtml_tree_node_find_parent_by_tag_id(myhtml_tree_node_t* node, myhtml_tag_id_t tag_id);

// indexes
myhtml_tree_indexes_t * myhtml_tree_index_create(myhtml_tree_t* tree, myhtml_tag_t* tags);
void myhtml_tree_index_clean(myhtml_tree_t* tree, myhtml_tag_t* tags);
myhtml_tree_indexes_t * myhtml_tree_index_destroy(myhtml_tree_t* tree, myhtml_tag_t* tags);

void myhtml_tree_index_append(myhtml_tree_t* tree, myhtml_tree_node_t* node);
myhtml_tree_node_t * myhtml_tree_index_get(myhtml_tree_t* tree, myhtml_tag_id_t tag_id);

// other
void myhtml_tree_wait_for_last_done_token(myhtml_tree_t* tree, myhtml_token_node_t* token_for_wait);

void myhtml_tree_tags_close_p(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_tree_generic_raw_text_element_parsing_algorithm(myhtml_tree_t* tree, myhtml_token_node_t* token_node);
void myhtml_tree_clear_stack_back_table_context(myhtml_tree_t* tree);
void myhtml_tree_clear_stack_back_table_body_context(myhtml_tree_t* tree);
void myhtml_tree_clear_stack_back_table_row_context(myhtml_tree_t* tree);
void myhtml_tree_close_cell(myhtml_tree_t* tree, myhtml_tree_node_t* tr_or_th_node);

bool myhtml_tree_is_mathml_integration_point(myhtml_tree_t* tree, myhtml_tree_node_t* node);
bool myhtml_tree_is_html_integration_point(myhtml_tree_t* tree, myhtml_tree_node_t* node);

// temp tag name
myhtml_status_t myhtml_tree_temp_tag_name_init(myhtml_tree_temp_tag_name_t* temp_tag_name);
void myhtml_tree_temp_tag_name_clean(myhtml_tree_temp_tag_name_t* temp_tag_name);
myhtml_tree_temp_tag_name_t * myhtml_tree_temp_tag_name_destroy(myhtml_tree_temp_tag_name_t* temp_tag_name, bool self_destroy);
myhtml_status_t myhtml_tree_temp_tag_name_append(myhtml_tree_temp_tag_name_t* temp_tag_name, const char* name, size_t name_len);
myhtml_status_t myhtml_tree_temp_tag_name_append_one(myhtml_tree_temp_tag_name_t* temp_tag_name, const char name);

// temp stream
struct myhtml_tree_temp_tag_name * myhtml_tree_temp_stream_alloc(myhtml_tree_t* tree, size_t size);
void myhtml_tree_temp_stream_clean(myhtml_tree_t* tree);
myhtml_tree_temp_stream_t * myhtml_tree_temp_stream_free(myhtml_tree_t* tree);

/* special tonek list */
myhtml_status_t myhtml_tree_special_list_init(myhtml_tree_special_token_list_t* special);
myhtml_status_t myhtml_tree_special_list_append(myhtml_tree_special_token_list_t* special, myhtml_token_node_t *token, myhtml_namespace_t my_namespace);
size_t myhtml_tree_special_list_length(myhtml_tree_special_token_list_t* special);
myhtml_tree_special_token_t * myhtml_tree_special_list_get_last(myhtml_tree_special_token_list_t* special);
size_t myhtml_tree_special_list_pop(myhtml_tree_special_token_list_t* special);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_tree_h */



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
#include "myhtml/stream.h"
#include "mycore/thread_queue.h"
#include "mycore/utils/mcsync.h"
#include "mycore/utils/mchar_async.h"
#include "mycore/utils/mcobject.h"
#include "mycore/utils/mcobject_async.h"

#define myhtml_tree_get(tree, attr) tree->attr
#define myhtml_tree_set(tree, attr) tree->attr

#define myhtml_tree_token_current(tree) myhtml_tree_get(tree, token_current)
#define myhtml_tree_token_attr_current(tree) myhtml_tree_get(tree, attr_current)

#define myhtml_tree_node_get(tree, node_id, attr) tree->nodes[node_id].attr

#define myhtml_tree_node_callback_insert(tree, node) \
    if(tree->callback_tree_node_insert) \
        tree->callback_tree_node_insert(tree, node, tree->callback_tree_node_insert_ctx)

#define myhtml_tree_node_callback_remove(tree, node) \
    if(tree->callback_tree_node_remove) \
        tree->callback_tree_node_remove(tree, node, tree->callback_tree_node_remove_ctx)

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
    
    myhtml_tag_id_t tag_id;
    enum myhtml_namespace ns;
    
    myhtml_tree_node_t* prev;
    myhtml_tree_node_t* next;
    myhtml_tree_node_t* child;
    myhtml_tree_node_t* parent;
    
    myhtml_tree_node_t* last_child;
    
    myhtml_token_node_t* token;
    void* data;
    
    myhtml_tree_t* tree;
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
    myhtml_namespace_t ns;
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
    
    myencoding_result_t res;
    struct myhtml_tree_temp_tag_name* current;
};

struct myhtml_tree {
    // ref
    myhtml_t*                    myhtml;
    mchar_async_t*               mchar;
    myhtml_token_t*              token;
    mcobject_async_t*            tree_obj;
    mcsync_t*                    sync;
    mythread_queue_list_entry_t* queue_entry;
    mythread_queue_t*            queue;
    myhtml_tag_t*                tags;
    void*                        modest;
    void*                        context;
    
    // init id's
    size_t                  mcasync_rules_token_id;
    size_t                  mcasync_rules_attr_id;
    size_t                  mcasync_tree_id;
    /* 
     * mchar_node_id
     * for rules, or if single mode,
     * or for main thread only after parsing
     */
    size_t                  mchar_node_id;
    myhtml_token_attr_t*    attr_current;
    myhtml_tag_id_t         tmp_tag_id;
    myhtml_token_node_t*    current_token_node;
    mythread_queue_node_t*  current_qnode;
    
    mcobject_t*                mcobject_incoming_buf;
    mycore_incoming_buffer_t*  incoming_buf;
    mycore_incoming_buffer_t*  incoming_buf_first;
    
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
    myhtml_stream_buffer_t*       stream_buffer;
    myhtml_token_node_t* volatile token_last_done;
    
    // for detect namespace out of tree builder
    myhtml_token_node_t*          token_namespace;
    
    // tree params
    enum myhtml_tokenizer_state        state;
    enum myhtml_tokenizer_state        state_of_builder;
    enum myhtml_insertion_mode         insert_mode;
    enum myhtml_insertion_mode         orig_insert_mode;
    enum myhtml_tree_compat_mode       compat_mode;
    volatile enum myhtml_tree_flags    flags;
    volatile myhtml_tree_parse_flags_t parse_flags;
    bool                               foster_parenting;
    size_t                             global_offset;
    mystatus_t                         tokenizer_status;
    
    myencoding_t            encoding;
    myencoding_t            encoding_usereq;
    myhtml_tree_temp_tag_name_t  temp_tag_name;
    
    /* callback */
    myhtml_callback_token_f callback_before_token;
    myhtml_callback_token_f callback_after_token;
    
    void* callback_before_token_ctx;
    void* callback_after_token_ctx;
    
    myhtml_callback_tree_node_f callback_tree_node_insert;
    myhtml_callback_tree_node_f callback_tree_node_remove;
    
    void* callback_tree_node_insert_ctx;
    void* callback_tree_node_remove_ctx;
};

// base
myhtml_tree_t * myhtml_tree_create(void);
mystatus_t myhtml_tree_init(myhtml_tree_t* tree, myhtml_t* myhtml);
void myhtml_tree_clean(myhtml_tree_t* tree);
void myhtml_tree_clean_all(myhtml_tree_t* tree);
myhtml_tree_t * myhtml_tree_destroy(myhtml_tree_t* tree);

/* parse flags */
myhtml_tree_parse_flags_t myhtml_tree_parse_flags(myhtml_tree_t* tree);
void myhtml_tree_parse_flags_set(myhtml_tree_t* tree, myhtml_tree_parse_flags_t flags);

myhtml_t * myhtml_tree_get_myhtml(myhtml_tree_t* tree);
myhtml_tag_t * myhtml_tree_get_tag(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_tree_get_document(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_tree_get_node_html(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_tree_get_node_head(myhtml_tree_t* tree);
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
bool myhtml_tree_element_in_scope_by_node(myhtml_tree_node_t* node, enum myhtml_tag_categories category);
void myhtml_tree_generate_implied_end_tags(myhtml_tree_t* tree, myhtml_tag_id_t exclude_tag_idx, myhtml_namespace_t mynamespace);
void myhtml_tree_generate_all_implied_end_tags(myhtml_tree_t* tree, myhtml_tag_id_t exclude_tag_idx, myhtml_namespace_t mynamespace);
myhtml_tree_node_t * myhtml_tree_appropriate_place_inserting(myhtml_tree_t* tree, myhtml_tree_node_t* override_target, enum myhtml_tree_insertion_mode* mode);
myhtml_tree_node_t * myhtml_tree_appropriate_place_inserting_in_tree(myhtml_tree_node_t* target, enum myhtml_tree_insertion_mode* mode);

// template insertion
myhtml_tree_insertion_list_t * myhtml_tree_template_insertion_init(myhtml_tree_t* tree);
void myhtml_tree_template_insertion_clean(myhtml_tree_t* tree);
myhtml_tree_insertion_list_t * myhtml_tree_template_insertion_destroy(myhtml_tree_t* tree);

void myhtml_tree_template_insertion_append(myhtml_tree_t* tree, enum myhtml_insertion_mode insert_mode);
void myhtml_tree_template_insertion_pop(myhtml_tree_t* tree);

void myhtml_tree_reset_insertion_mode_appropriately(myhtml_tree_t* tree);

bool myhtml_tree_adoption_agency_algorithm(myhtml_tree_t* tree, myhtml_token_node_t* token, myhtml_tag_id_t subject_tag_idx);
size_t myhtml_tree_template_insertion_length(myhtml_tree_t* tree);

// other for a tree
myhtml_tree_node_t * myhtml_tree_node_create(myhtml_tree_t* tree);
void myhtml_tree_node_delete(myhtml_tree_node_t* node);
void myhtml_tree_node_delete_recursive(myhtml_tree_node_t* node);
void myhtml_tree_node_clean(myhtml_tree_node_t* tree_node);
void myhtml_tree_node_free(myhtml_tree_node_t* node);
myhtml_tree_node_t * myhtml_tree_node_clone(myhtml_tree_node_t* node);

void myhtml_tree_node_add_child(myhtml_tree_node_t* root, myhtml_tree_node_t* node);
void myhtml_tree_node_insert_before(myhtml_tree_node_t* root, myhtml_tree_node_t* node);
void myhtml_tree_node_insert_after(myhtml_tree_node_t* root, myhtml_tree_node_t* node);
void myhtml_tree_node_insert_by_mode(myhtml_tree_node_t* adjusted_location, myhtml_tree_node_t* node, enum myhtml_tree_insertion_mode mode);
myhtml_tree_node_t * myhtml_tree_node_remove(myhtml_tree_node_t* node);

myhtml_tree_node_t * myhtml_tree_node_insert_html_element(myhtml_tree_t* tree, myhtml_token_node_t* token);
myhtml_tree_node_t * myhtml_tree_node_insert_foreign_element(myhtml_tree_t* tree, myhtml_token_node_t* token);
myhtml_tree_node_t * myhtml_tree_node_insert_by_token(myhtml_tree_t* tree, myhtml_token_node_t* token, myhtml_namespace_t ns);
myhtml_tree_node_t * myhtml_tree_node_insert(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, myhtml_namespace_t ns);
myhtml_tree_node_t * myhtml_tree_node_insert_by_node(myhtml_tree_t* tree, myhtml_tree_node_t* idx);
myhtml_tree_node_t * myhtml_tree_node_insert_comment(myhtml_tree_t* tree, myhtml_token_node_t* token, myhtml_tree_node_t* parent);
myhtml_tree_node_t * myhtml_tree_node_insert_doctype(myhtml_tree_t* tree, myhtml_token_node_t* token);
myhtml_tree_node_t * myhtml_tree_node_insert_root(myhtml_tree_t* tree, myhtml_token_node_t* token, myhtml_namespace_t ns);
myhtml_tree_node_t * myhtml_tree_node_insert_text(myhtml_tree_t* tree, myhtml_token_node_t* token);
myhtml_tree_node_t * myhtml_tree_node_find_parent_by_tag_id(myhtml_tree_node_t* node, myhtml_tag_id_t tag_id);

// other
void myhtml_tree_wait_for_last_done_token(myhtml_tree_t* tree, myhtml_token_node_t* token_for_wait);

void myhtml_tree_tags_close_p(myhtml_tree_t* tree, myhtml_token_node_t* token);
myhtml_tree_node_t * myhtml_tree_generic_raw_text_element_parsing_algorithm(myhtml_tree_t* tree, myhtml_token_node_t* token_node);
void myhtml_tree_clear_stack_back_table_context(myhtml_tree_t* tree);
void myhtml_tree_clear_stack_back_table_body_context(myhtml_tree_t* tree);
void myhtml_tree_clear_stack_back_table_row_context(myhtml_tree_t* tree);
void myhtml_tree_close_cell(myhtml_tree_t* tree, myhtml_tree_node_t* tr_or_th_node, myhtml_token_node_t* token);

bool myhtml_tree_is_mathml_integration_point(myhtml_tree_t* tree, myhtml_tree_node_t* node);
bool myhtml_tree_is_html_integration_point(myhtml_tree_t* tree, myhtml_tree_node_t* node);

// temp tag name
mystatus_t myhtml_tree_temp_tag_name_init(myhtml_tree_temp_tag_name_t* temp_tag_name);
void myhtml_tree_temp_tag_name_clean(myhtml_tree_temp_tag_name_t* temp_tag_name);
myhtml_tree_temp_tag_name_t * myhtml_tree_temp_tag_name_destroy(myhtml_tree_temp_tag_name_t* temp_tag_name, bool self_destroy);
mystatus_t myhtml_tree_temp_tag_name_append(myhtml_tree_temp_tag_name_t* temp_tag_name, const char* name, size_t name_len);
mystatus_t myhtml_tree_temp_tag_name_append_one(myhtml_tree_temp_tag_name_t* temp_tag_name, const char name);

/* special tonek list */
mystatus_t myhtml_tree_special_list_init(myhtml_tree_special_token_list_t* special);
mystatus_t myhtml_tree_special_list_append(myhtml_tree_special_token_list_t* special, myhtml_token_node_t *token, myhtml_namespace_t ns);
size_t myhtml_tree_special_list_length(myhtml_tree_special_token_list_t* special);
myhtml_tree_special_token_t * myhtml_tree_special_list_get_last(myhtml_tree_special_token_list_t* special);
size_t myhtml_tree_special_list_pop(myhtml_tree_special_token_list_t* special);

/* incoming buffer */
mycore_incoming_buffer_t * myhtml_tree_incoming_buffer_first(myhtml_tree_t *tree);
const char * myhtml_tree_incomming_buffer_make_data(myhtml_tree_t *tree, size_t begin, size_t length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_tree_h */



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

#ifndef MyHTML_MYHTML_H
#define MyHTML_MYHTML_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"

#include "mycore/utils/mctree.h"
#include "mycore/utils/mcobject_async.h"
#include "mycore/mythread.h"
#include "mycore/incoming.h"
#include "myencoding/encoding.h"
#include "myhtml/tree.h"
#include "myhtml/tag.h"
#include "myhtml/def.h"
#include "myhtml/parser.h"
#include "myhtml/tokenizer.h"
#include "myhtml/rules.h"
#include "myhtml/token.h"
#include "myhtml/charef.h"
#include "myhtml/callback.h"

#define mh_queue_current() tree->queue
#define myhtml_tokenizer_state_set(tree) myhtml_tree_set(tree, state)

#define mh_queue_get(idx, attr) myhtml->queue->nodes[idx].attr

// space, tab, LF, FF, CR
#define myhtml_whithspace(onechar, action, logic)    \
    onechar action ' '  logic                        \
    onechar action '\t' logic                        \
    onechar action '\n' logic                        \
    onechar action '\f' logic                        \
    onechar action '\r'

#define myhtml_ascii_char_cmp(onechar)      \
    ((onechar >= 'a' && onechar <= 'z') ||  \
    (onechar >= 'A' && onechar <= 'Z'))

#define myhtml_ascii_char_unless_cmp(onechar)         \
    ((onechar < 'a' || onechar > 'z') &&              \
    (onechar < 'A' || onechar > 'Z'))

struct myhtml {
    mythread_t* thread_stream;
    mythread_t* thread_batch;
    mythread_t* thread_list[3];
    size_t      thread_total;
    
    myhtml_tokenizer_state_f* parse_state_func;
    myhtml_insertion_f* insertion_func;
    
    enum myhtml_options opt;
    myhtml_tree_node_t *marker;
};

struct myhtml_collection {
    myhtml_tree_node_t **list;
    size_t size;
    size_t length;
};


myhtml_t * myhtml_create(void);
mystatus_t myhtml_init(myhtml_t* myhtml, enum myhtml_options opt, size_t thread_count, size_t queue_size);
void myhtml_clean(myhtml_t* myhtml);
myhtml_t* myhtml_destroy(myhtml_t* myhtml);

mystatus_t myhtml_parse(myhtml_tree_t* tree, myencoding_t encoding, const char* html, size_t html_size);
mystatus_t myhtml_parse_fragment(myhtml_tree_t* tree, myencoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace ns);

mystatus_t myhtml_parse_single(myhtml_tree_t* tree, myencoding_t encoding, const char* html, size_t html_size);
mystatus_t myhtml_parse_fragment_single(myhtml_tree_t* tree, myencoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace ns);

mystatus_t myhtml_parse_chunk(myhtml_tree_t* tree, const char* html, size_t html_size);
mystatus_t myhtml_parse_chunk_fragment(myhtml_tree_t* tree, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace ns);
mystatus_t myhtml_parse_chunk_single(myhtml_tree_t* tree, const char* html, size_t html_size);
mystatus_t myhtml_parse_chunk_fragment_single(myhtml_tree_t* tree, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace ns);
mystatus_t myhtml_parse_chunk_end(myhtml_tree_t* tree);

// encoding
void myhtml_encoding_set(myhtml_tree_t* tree, myencoding_t encoding);
myencoding_t myhtml_encoding_get(myhtml_tree_t* tree);

myhtml_collection_t * myhtml_get_nodes_by_tag_id(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tag_id_t tag_id, mystatus_t *status);
myhtml_collection_t * myhtml_get_nodes_by_name(myhtml_tree_t* tree, myhtml_collection_t *collection, const char* html, size_t length, mystatus_t *status);
myhtml_collection_t * myhtml_get_nodes_by_attribute_key(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* scope_node, const char* key, size_t key_len, mystatus_t* status);
/* like a [some=value] or #id */
myhtml_collection_t * myhtml_get_nodes_by_attribute_value(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                          bool case_insensitive,
                                                          const char* key, size_t key_len,
                                                          const char* value, size_t value_len,
                                                          mystatus_t* status);
/* like a [some~=value] or .class */
myhtml_collection_t * myhtml_get_nodes_by_attribute_value_whitespace_separated(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                                               bool case_insensitive,
                                                                               const char* key, size_t key_len,
                                                                               const char* value, size_t value_len,
                                                                               mystatus_t* status);
/* like a [some^=value] */
myhtml_collection_t * myhtml_get_nodes_by_attribute_value_begin(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                                bool case_insensitive,
                                                                const char* key, size_t key_len,
                                                                const char* value, size_t value_len,
                                                                mystatus_t* status);
/* like a [some$=value] */
myhtml_collection_t * myhtml_get_nodes_by_attribute_value_end(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                              bool case_insensitive,
                                                              const char* key, size_t key_len,
                                                              const char* value, size_t value_len,
                                                              mystatus_t* status);
/* like a [some*=value] */
myhtml_collection_t * myhtml_get_nodes_by_attribute_value_contain(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                                  bool case_insensitive,
                                                                  const char* key, size_t key_len,
                                                                  const char* value, size_t value_len,
                                                                  mystatus_t* status);
/* like a [some|=value] */
myhtml_collection_t * myhtml_get_nodes_by_attribute_value_hyphen_separated(myhtml_tree_t *tree, myhtml_collection_t* collection, myhtml_tree_node_t* node,
                                                                           bool case_insensitive,
                                                                           const char* key, size_t key_len,
                                                                           const char* value, size_t value_len,
                                                                           mystatus_t* status);

myhtml_collection_t * myhtml_get_nodes_by_tag_id_in_scope(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tree_node_t *node, myhtml_tag_id_t tag_id, mystatus_t *status);
myhtml_collection_t * myhtml_get_nodes_by_name_in_scope(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tree_node_t *node, const char* html, size_t length, mystatus_t *status);

myhtml_tree_node_t * myhtml_node_first(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_node_next(myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_prev(myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_parent(myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_child(myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_last_child(myhtml_tree_node_t *node);

myhtml_tree_node_t * myhtml_node_insert_to_appropriate_place(myhtml_tree_node_t *target, myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_append_child(myhtml_tree_node_t *target, myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_insert_after(myhtml_tree_node_t *target, myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_insert_before(myhtml_tree_node_t *target, myhtml_tree_node_t *node);

myhtml_tree_node_t * myhtml_node_create(myhtml_tree_t* tree, myhtml_tag_id_t tag_id, enum myhtml_namespace ns);
myhtml_tree_node_t * myhtml_node_remove(myhtml_tree_node_t *node);
void myhtml_node_delete(myhtml_tree_node_t *node);
void myhtml_node_delete_recursive(myhtml_tree_node_t *node);
void myhtml_node_free(myhtml_tree_node_t *node);

myhtml_token_node_t* myhtml_node_token(myhtml_tree_node_t *node);
myhtml_namespace_t myhtml_node_namespace(myhtml_tree_node_t *node);
void myhtml_node_namespace_set(myhtml_tree_node_t *node, myhtml_namespace_t ns);
myhtml_tag_id_t myhtml_node_tag_id(myhtml_tree_node_t *node);
const char * myhtml_tag_name_by_id(myhtml_tree_t* tree, myhtml_tag_id_t tag_id, size_t *length);
myhtml_tag_id_t myhtml_tag_id_by_name(myhtml_tree_t* tree, const char *tag_name, size_t length);
bool myhtml_node_is_close_self(myhtml_tree_node_t *node);
bool myhtml_node_is_void_element(myhtml_tree_node_t *node);
myhtml_tree_attr_t * myhtml_node_attribute_first(myhtml_tree_node_t *node);
myhtml_tree_attr_t * myhtml_node_attribute_last(myhtml_tree_node_t *node);
const char * myhtml_node_text(myhtml_tree_node_t *node, size_t *length);
mycore_string_t * myhtml_node_string(myhtml_tree_node_t *node);
myhtml_position_t myhtml_node_raw_position(myhtml_tree_node_t *node);
myhtml_position_t myhtml_node_element_position(myhtml_tree_node_t *node);
void myhtml_node_set_data(myhtml_tree_node_t *node, void* data);
void * myhtml_node_get_data(myhtml_tree_node_t *node);

/* attributes */
myhtml_tree_attr_t * myhtml_attribute_next(myhtml_tree_attr_t *attr);
myhtml_tree_attr_t * myhtml_attribute_prev(myhtml_tree_attr_t *attr);
enum myhtml_namespace myhtml_attribute_namespace(myhtml_tree_attr_t *attr);
void myhtml_attribute_namespace_set(myhtml_tree_attr_t *attr, myhtml_namespace_t ns);

const char * myhtml_attribute_key(myhtml_tree_attr_t *attr, size_t *length);
const char * myhtml_attribute_value(myhtml_tree_attr_t *attr, size_t *length);
mycore_string_t * myhtml_attribute_key_string(myhtml_tree_attr_t* attr);
mycore_string_t * myhtml_attribute_value_string(myhtml_tree_attr_t* attr);
myhtml_tree_attr_t * myhtml_attribute_by_key(myhtml_tree_node_t *node, const char *key, size_t key_len);
myhtml_tree_attr_t * myhtml_attribute_add(myhtml_tree_node_t *node, const char *key, size_t key_len, const char *value, size_t value_len, myencoding_t encoding);
myhtml_tree_attr_t * myhtml_attribute_remove(myhtml_tree_node_t *node, myhtml_tree_attr_t *attr);
myhtml_tree_attr_t * myhtml_attribute_remove_by_key(myhtml_tree_node_t *node, const char *key, size_t key_len);
void myhtml_attribute_delete(myhtml_tree_t *tree, myhtml_tree_node_t *node, myhtml_tree_attr_t *attr);
void myhtml_attribute_free(myhtml_tree_t *tree, myhtml_tree_attr_t *attr);
myhtml_position_t myhtml_attribute_key_raw_position(myhtml_tree_attr_t *attr);
myhtml_position_t myhtml_attribute_value_raw_position(myhtml_tree_attr_t *attr);

/* collection */
myhtml_collection_t * myhtml_collection_create(size_t size, mystatus_t *status);
void myhtml_collection_clean(myhtml_collection_t *collection);
myhtml_collection_t * myhtml_collection_destroy(myhtml_collection_t *collection);
mystatus_t myhtml_collection_check_size(myhtml_collection_t *collection, size_t need, size_t upto_length);

// strings
mycore_string_t * myhtml_node_text_set(myhtml_tree_node_t *node, const char* text, size_t length, myencoding_t encoding);
mycore_string_t * myhtml_node_text_set_with_charef(myhtml_tree_node_t *node, const char* text, size_t length, myencoding_t encoding);

bool myhtml_utils_strcmp(const char* ab, const char* to_lowercase, size_t size);
bool myhtml_is_html_node(myhtml_tree_node_t *node, myhtml_tag_id_t tag_id);

// queue
mystatus_t myhtml_queue_add(myhtml_tree_t *tree, size_t begin, myhtml_token_node_t* token);

/* version */
myhtml_version_t myhtml_version(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

/*
 Copyright 2015-2016 Alexander Borisov
 
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

#ifndef MyHTML_MYHTML_H
#define MyHTML_MYHTML_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"

#include "myhtml/utils/mctree.h"
#include "myhtml/utils/mcobject_async.h"
#include "myhtml/tree.h"
#include "myhtml/tag.h"
#include "myhtml/def.h"
#include "myhtml/parser.h"
#include "myhtml/tokenizer.h"
#include "myhtml/thread.h"
#include "myhtml/rules.h"
#include "myhtml/token.h"
#include "myhtml/charef.h"
#include "myhtml/encoding.h"

#define mh_thread_get(__idx__, __attr__) myhtml_thread_get(myhtml->thread, __idx__, __attr__)
#define mh_thread_set(__idx__, __attr__) myhtml_thread_set(myhtml->thread, __idx__, __attr__)

#define mh_thread_post(__idx__) sem_post(myhtml_thread_get(myhtml->thread, __idx__, sem))
#define mh_thread_wait(__idx__) sem_wait(myhtml_thread_get(myhtml->thread, __idx__, sem))
#define mh_thread_done(__idx__, __param__) myhtml_thread_set(myhtml->thread, __idx__, opt) = __param__;

#define mh_tree_get(__attr__) myhtml_tree_get(tree, __attr__)
#define mh_tree_set(__attr__) myhtml_tree_set(tree, __attr__)

#define mh_queue_current() mh_tree_get(queue)

#define mh_state_get(__tree__) myhtml_tree_get(__tree__, state)
#define mh_state_set(__tree__) myhtml_tree_set(__tree__, state)

#define mh_tags_get(__idx__, __attr__) myhtml_tag_get(tree->myhtml->tags, __idx__, __attr__)

#define mh_queue_last(__attr__) myhtml->queue->nodes[myhtml_queue_node_current(myhtml->queue)].__attr__
#define mh_queue_get(__idx__, __attr__) myhtml->queue->nodes[__idx__].__attr__
#define mh_queue_set(__idx__, __attr__) mh_queue_get(__idx__, __attr__)
#define mh_queue_current_get(__attr__) mh_queue_get(mh_queue_current(), __attr__)
#define mh_queue_current_set(__attr__) mh_queue_current_get(__attr__)

#define mh_queue_add(__tree__, __html__, __begin__, __current__)       \
    if(__tree__->flags & MyHTML_TREE_FLAGS_SINGLE_MODE) { \
        myhtml_parser_worker(0, __tree__->current_qnode); \
        while(myhtml_rules_tree_dispatcher(__tree__, __tree__->current_qnode->token)){}; \
    } \
    __tree__->current_qnode = mythread_queue_node_malloc(__tree__->myhtml->thread, __tree__->queue, __html__, (__tree__->global_offset + __begin__), NULL); \
    __tree__->current_qnode->tree = __tree__; \
    __tree__->current_qnode->prev = __current__; \
    if(__current__) \
        myhtml_tokenizer_calc_current_namespace(__tree__, __current__); \
    myhtml_token_node_malloc(__tree__->token, __tree__->current_qnode->token, __tree__->token->mcasync_token_id)

#define mh_token_get(__idx__, __attr__) tree->token->nodes[__idx__].__attr__
#define mh_token_set(__idx__, __attr__) mh_token_get(__idx__, __attr__)

#define mh_tree_token_current(__attr__) mh_token_get(myhtml_tree_token_current(tree), __attr__)
#define mh_tree_token_attr_current(__attr__) mh_token_get(myhtml_tree_token_attr_current(tree), __attr__)

#define mh_token_attr_get(__idx__, __attr__) tree->token->attr[__idx__].__attr__
#define mh_token_attr_set(__idx__, __attr__) mh_token_attr_get(__idx__, __attr__)
#define mh_token_attr_current(__idx__) mh_token_get(__idx__, attr_last)

#define mh_queue_token_get(__idx__, __attr__) mh_token_get(mh_queue_get(__idx__, token), __attr__)
#define mh_queue_token_set(__idx__, __attr__) mh_queue_token_get(__idx__, __attr__)

// space, tab, LF, FF, CR
#define myhtml_whithspace(__char__, __action__, __logic__)    \
    __char__ __action__ ' ' __logic__                    \
    __char__ __action__ '\t' __logic__                    \
    __char__ __action__ '\n' __logic__                    \
    __char__ __action__ '\f' __logic__                    \
    __char__ __action__ '\r'

#define myhtml_ascii_char_cmp(__char__)     \
    ((__char__ >= 'a' && __char__ <= 'z') || \
    (__char__ >= 'A' && __char__ <= 'Z'))

#define myhtml_ascii_char_unless_cmp(__char__) \
    ((__char__ < 'a' || __char__ > 'z') &&      \
    (__char__ < 'A' || __char__ > 'Z'))

struct myhtml {
    mythread_t          *thread;
    mcobject_async_t    *async_incoming_buf;
    mchar_async_t       *mchar; // for all
    mcobject_async_t    *tag_index;
    
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

struct myhtml_incoming_buf {
    const char* data;
    size_t length;
    size_t size;
    size_t offset;
    
    struct myhtml_incoming_buf* prev;
    struct myhtml_incoming_buf* next;
};

myhtml_t * myhtml_create(void);
myhtml_status_t myhtml_init(myhtml_t* myhtml, enum myhtml_options opt, size_t thread_count, size_t queue_size);
void myhtml_clean(myhtml_t* myhtml);
myhtml_t* myhtml_destroy(myhtml_t* myhtml);

myhtml_status_t myhtml_parse(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size);
myhtml_status_t myhtml_parse_fragment(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace);

myhtml_status_t myhtml_parse_single(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size);
myhtml_status_t myhtml_parse_fragment_single(myhtml_tree_t* tree, myhtml_encoding_t encoding, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace);

myhtml_status_t myhtml_parse_chunk(myhtml_tree_t* tree, const char* html, size_t html_size);
myhtml_status_t myhtml_parse_chunk_fragment(myhtml_tree_t* tree, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace);
myhtml_status_t myhtml_parse_chunk_single(myhtml_tree_t* tree, const char* html, size_t html_size);
myhtml_status_t myhtml_parse_chunk_fragment_single(myhtml_tree_t* tree, const char* html, size_t html_size, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace);
myhtml_status_t myhtml_parse_chunk_end(myhtml_tree_t* tree);

// encoding
void myhtml_encoding_set(myhtml_tree_t* tree, myhtml_encoding_t encoding);
myhtml_encoding_t myhtml_encoding_get(myhtml_tree_t* tree);

myhtml_collection_t * myhtml_get_nodes_by_tag_id(myhtml_tree_t* tree, myhtml_collection_t *collection, myhtml_tag_id_t tag_id, myhtml_status_t *status);
myhtml_collection_t * myhtml_get_nodes_by_name(myhtml_tree_t* tree, myhtml_collection_t *collection, const char* html, size_t length, myhtml_status_t *status);

myhtml_tree_node_t * myhtml_node_first(myhtml_tree_t* tree);
myhtml_tree_node_t * myhtml_node_next(myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_prev(myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_parent(myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_child(myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_last_child(myhtml_tree_node_t *node);

myhtml_tree_node_t * myhtml_node_insert_to_appropriate_place(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_insert_append_child(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_insert_after(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node);
myhtml_tree_node_t * myhtml_node_insert_before(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node);

myhtml_tree_node_t * myhtml_node_create(myhtml_tree_t* tree, myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace);
myhtml_tree_node_t * myhtml_node_remove(myhtml_tree_node_t *node);
void myhtml_node_delete(myhtml_tree_t* tree, myhtml_tree_node_t *node);
void myhtml_node_delete_recursive(myhtml_tree_t* tree, myhtml_tree_node_t *node);
void myhtml_node_free(myhtml_tree_t* tree, myhtml_tree_node_t *node);
    
enum myhtml_namespace myhtml_node_namespace(myhtml_tree_node_t *node);
myhtml_tag_id_t myhtml_node_tag_id(myhtml_tree_node_t *node);
const char * myhtml_tag_name_by_id(myhtml_tree_t* tree, myhtml_tag_id_t tag_id, size_t *length);
myhtml_tag_id_t myhtml_tag_id_by_name(myhtml_tree_t* tree, const char *tag_name, size_t length);
bool myhtml_node_is_close_self(myhtml_tree_node_t *node);
myhtml_tree_attr_t * myhtml_node_attribute_first(myhtml_tree_node_t *node);
myhtml_tree_attr_t * myhtml_node_attribute_last(myhtml_tree_node_t *node);
const char * myhtml_node_text(myhtml_tree_node_t *node, size_t *length);
myhtml_string_t * myhtml_node_string(myhtml_tree_node_t *node);

myhtml_tree_attr_t * myhtml_attribute_next(myhtml_tree_attr_t *attr);
myhtml_tree_attr_t * myhtml_attribute_prev(myhtml_tree_attr_t *attr);
enum myhtml_namespace myhtml_attribute_namespace(myhtml_tree_attr_t *attr);
const char * myhtml_attribute_name(myhtml_tree_attr_t *attr, size_t *length);
const char * myhtml_attribute_value(myhtml_tree_attr_t *attr, size_t *length);
myhtml_tree_attr_t * myhtml_attribute_by_key(myhtml_tree_node_t *node, const char *key, size_t key_len);
myhtml_tree_attr_t * myhtml_attribute_add(myhtml_tree_t *tree, myhtml_tree_node_t *node, const char *key, size_t key_len, const char *value, size_t value_len, myhtml_encoding_t encoding);
myhtml_tree_attr_t * myhtml_attribute_remove(myhtml_tree_node_t *node, myhtml_tree_attr_t *attr);
myhtml_tree_attr_t * myhtml_attribute_remove_by_key(myhtml_tree_node_t *node, const char *key, size_t key_len);
void myhtml_attribute_delete(myhtml_tree_t *tree, myhtml_tree_node_t *node, myhtml_tree_attr_t *attr);
void myhtml_attribute_free(myhtml_tree_t *tree, myhtml_tree_attr_t *attr);

myhtml_collection_t * myhtml_collection_create(size_t size, myhtml_status_t *status);
void myhtml_collection_clean(myhtml_collection_t *collection);
myhtml_collection_t * myhtml_collection_destroy(myhtml_collection_t *collection);
myhtml_status_t myhtml_collection_check_size(myhtml_collection_t *collection, size_t up_to_length);

// strings
myhtml_string_t * myhtml_node_text_set(myhtml_tree_t* tree, myhtml_tree_node_t *node, const char* text, size_t length, myhtml_encoding_t encoding);
myhtml_string_t * myhtml_node_text_set_with_charef(myhtml_tree_t* tree, myhtml_tree_node_t *node, const char* text, size_t length, myhtml_encoding_t encoding);

// incoming buffer
void myhtml_incomming_buf_add(myhtml_t* myhtml, myhtml_tree_t* tree, myhtml_incoming_buf_t *current, const char *html, size_t html_size);
void myhtml_incomming_buf_clean(myhtml_tree_t* tree, myhtml_incoming_buf_t *current);
const char * myhtml_tree_incomming_buf_get_last(myhtml_tree_t *tree, myhtml_incoming_buf_t *inc_buf, size_t current_offset, size_t len);
const char * myhtml_tree_incomming_buf_make_data(myhtml_tree_t *tree, mythread_queue_node_t *qnode, size_t len);

bool myhtml_utils_strcmp(const char* ab, const char* to_lowercase, size_t size);
bool myhtml_is_html_node(myhtml_tree_node_t *node, myhtml_tag_id_t tag_id);

/** 
 * Platform-specific hdef performance clock queries.
 * Implemented in perf.c
 */ 

/** Get clock resolution */
uint64_t myhtml_hperf_res(myhtml_status_t *status);

/** Get current value in clock ticks */
uint64_t myhtml_hperf_clock(myhtml_status_t *status);

/** Print an hperf measure */
myhtml_status_t myhtml_hperf_print(const char *name, uint64_t x, uint64_t y, FILE *fh);
myhtml_status_t myhtml_hperf_print_by_val(const char *name, uint64_t x, FILE *fh);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

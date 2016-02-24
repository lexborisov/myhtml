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

#ifndef MyHTML_TOKENIZER_H
#define MyHTML_TOKENIZER_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/utils.h"
#include "myhtml/myhtml.h"
#include "myhtml/tag.h"
#include "myhtml/thread.h"
#include "myhtml/tokenizer_doctype.h"
#include "myhtml/tokenizer_script.h"
#include "myhtml/tokenizer_end.h"

#define MyHTML_TOKENIZER_CHAR_OTHER        '\000'
#define MyHTML_TOKENIZER_CHAR_A_Z_a_z      '\001'
#define MyHTML_TOKENIZER_CHAR_WHITESPACE   '\002'
    
#define myhtml_tokenizer_inc_html_offset(__offset__, __size__)   \
    __offset__++;                                            \
    if(__offset__ >= __size__)                               \
        return __offset__

void myhtml_tokenizer_begin(myhtml_tree_t* tree, const char* html, size_t html_length);
void myhtml_tokenizer_chunk(myhtml_tree_t* tree, const char* html, size_t html_length);
void myhtml_tokenizer_end(myhtml_tree_t* tree);
void myhtml_tokenizer_set_state(myhtml_tree_t* tree, mythread_queue_node_t* qnode);

void myhtml_tokenizer_calc_current_namespace(myhtml_tree_t* tree, mythread_queue_node_t* qnode);

myhtml_tree_node_t * myhtml_tokenizer_fragment_init(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, enum myhtml_namespace my_namespace);

void myhtml_tokenizer_wait(myhtml_tree_t* tree);
void myhtml_tokenizer_post(myhtml_tree_t* tree);

myhtml_status_t myhtml_tokenizer_state_init(myhtml_t* myhtml);
void myhtml_tokenizer_state_destroy(myhtml_t* myhtml);

mythread_queue_node_t * myhtml_tokenizer_queue_create_text_node_if_need(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, enum myhtml_token_type type);
void myhtml_check_tag_parser(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset);

size_t myhtml_tokenizer_state_bogus_comment(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

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

#ifndef MyHTML_TOKENIZER_SCRIPT_H
#define MyHTML_TOKENIZER_SCRIPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/myhtml.h"
#include "myhtml/tokenizer.h"

size_t myhtml_tokenizer_state_script_data(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_end_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_escape_start(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_escape_start_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_escaped(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_escaped_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_escaped_dash_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_escaped_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_escaped_end_tag_open(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_escaped_end_tag_name(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_double_escape_start(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_double_escaped(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_double_escaped_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_double_escaped_dash_dash(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_double_escaped_less_than_sign(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_script_data_double_escape_end(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_tokenizer_script_h */

//
//  myhtml_tokenizer_script.h
//  myhtml
//
//  Created by Alexander Borisov on 17.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_tokenizer_script_h
#define myhtml_tokenizer_script_h

#include "myosi.h"
#include "myhtml.h"
#include "tokenizer.h"

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


#endif /* myhtml_tokenizer_script_h */

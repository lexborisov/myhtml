//
//  myhtml_tokenizer_doctype.h
//  myhtml
//
//  Created by Alexander Borisov on 12.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_tokenizer_doctype_h
#define myhtml_tokenizer_doctype_h

#include "myosi.h"
#include "myhtml.h"

size_t myhtml_tokenizer_state_before_doctype_name(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_name(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_after_doctype_name(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_before_doctype_public_identifier(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_public_identifier_double_quoted(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_public_identifier_single_quoted(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_after_doctype_public_identifier(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_system_identifier_double_quoted(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_system_identifier_single_quoted(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_after_doctype_system_identifier(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_bogus_doctype(myhtml_tree_t* tree, myhtml_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);

#endif /* myhtml_tokenizer_doctype_h */

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

#ifndef MyHTML_TOKENIZER_DOCTYPE_H
#define MyHTML_TOKENIZER_DOCTYPE_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "mycore/utils.h"
#include "myhtml/myhtml.h"

size_t myhtml_tokenizer_state_doctype(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_before_doctype_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_after_doctype_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_custom_after_doctype_name_a_z(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_before_doctype_public_identifier(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_public_identifier_double_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_public_identifier_single_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_after_doctype_public_identifier(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_system_identifier_double_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_doctype_system_identifier_single_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_after_doctype_system_identifier(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_state_bogus_doctype(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_tokenizer_doctype_h */

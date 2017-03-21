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

#ifndef MyHTML_TOKENIZER_END_H
#define MyHTML_TOKENIZER_END_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/myhtml.h"
#include "myhtml/tokenizer.h"

size_t myhtml_tokenizer_end_state_data(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_tag_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_end_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_self_closing_start_tag(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_markup_declaration_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_before_attribute_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_attribute_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_after_attribute_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_before_attribute_value(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_attribute_value_double_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_attribute_value_single_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_attribute_value_unquoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_after_attribute_value_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_comment_start(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_comment_start_dash(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_comment(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_comment_end(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_comment_end_dash(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_comment_end_bang(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_bogus_comment(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_cdata_section(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);

size_t myhtml_tokenizer_end_state_rcdata(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_rcdata_less_than_sign(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_rcdata_end_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_rcdata_end_tag_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);

size_t myhtml_tokenizer_end_state_rawtext(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_rawtext_less_than_sign(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_rawtext_end_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_rawtext_end_tag_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);

size_t myhtml_tokenizer_end_state_plaintext(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_doctype(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_before_doctype_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_doctype_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_after_doctype_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_custom_after_doctype_name_a_z(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_before_doctype_public_identifier(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_doctype_public_identifier_double_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_doctype_public_identifier_single_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_after_doctype_public_identifier(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_doctype_system_identifier_double_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_doctype_system_identifier_single_quoted(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_after_doctype_system_identifier(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_bogus_doctype(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_less_than_sign(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_end_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_end_tag_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_escape_start(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_escape_start_dash(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_escaped(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_escaped_dash(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_escaped_dash_dash(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_escaped_less_than_sign(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_escaped_end_tag_open(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_escaped_end_tag_name(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_double_escape_start(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_double_escaped(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_double_escaped_dash(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_double_escaped_dash_dash(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_double_escaped_less_than_sign(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_script_data_double_escape_end(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);
size_t myhtml_tokenizer_end_state_parse_error_stop(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_tokenizer_end_h */

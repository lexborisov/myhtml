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

#ifndef MyHTML_TOKENIZER_H
#define MyHTML_TOKENIZER_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <myhtml/myosi.h>
#include <mycore/utils.h>
#include <mycore/mythread.h>
#include <myhtml/myhtml.h>
#include <myhtml/tag.h>
#include <myhtml/tokenizer_doctype.h>
#include <myhtml/tokenizer_script.h>
#include <myhtml/tokenizer_end.h>

#define myhtml_tokenizer_inc_html_offset(offset, size)   \
    offset++;                                            \
    if(offset >= size)                                   \
        return offset

mystatus_t myhtml_tokenizer_begin(myhtml_tree_t* tree, const char* html, size_t html_length);
mystatus_t myhtml_tokenizer_chunk(myhtml_tree_t* tree, const char* html, size_t html_length);
mystatus_t myhtml_tokenizer_chunk_with_stream_buffer(myhtml_tree_t* tree, const char* html, size_t html_length);
mystatus_t myhtml_tokenizer_end(myhtml_tree_t* tree);
void myhtml_tokenizer_set_state(myhtml_tree_t* tree, myhtml_token_node_t* token_node);

void myhtml_tokenizer_calc_current_namespace(myhtml_tree_t* tree, myhtml_token_node_t* token_node);

myhtml_tree_node_t * myhtml_tokenizer_fragment_init(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, enum myhtml_namespace ns);

void myhtml_tokenizer_wait(myhtml_tree_t* tree);
void myhtml_tokenizer_post(myhtml_tree_t* tree);
void myhtml_tokenizer_pause(myhtml_tree_t* tree);

mystatus_t myhtml_tokenizer_state_init(myhtml_t* myhtml);
void myhtml_tokenizer_state_destroy(myhtml_t* myhtml);

myhtml_token_node_t * myhtml_tokenizer_queue_create_text_node_if_need(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t absolute_html_offset, enum myhtml_token_type type);
void myhtml_check_tag_parser(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset);

size_t myhtml_tokenizer_state_bogus_comment(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

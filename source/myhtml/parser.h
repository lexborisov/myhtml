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

#ifndef MyHTML_PARSER_H
#define MyHTML_PARSER_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/myhtml.h"
#include "myhtml/mystring.h"
#include "myhtml/tree.h"
#include "myhtml/token.h"
#include "myhtml/data_process.h"

void myhtml_parser_stream(mythread_id_t thread_id, void* ctx);
void myhtml_parser_worker(mythread_id_t thread_id, void* ctx);
void myhtml_parser_worker_stream(mythread_id_t thread_id, void* ctx);

size_t myhtml_parser_token_data_to_string(myhtml_tree_t *tree, mycore_string_t* str, myhtml_data_process_entry_t* proc_entry, size_t begin, size_t length);
size_t myhtml_parser_token_data_to_string_lowercase(myhtml_tree_t *tree, mycore_string_t* str, myhtml_data_process_entry_t* proc_entry, size_t begin, size_t length);
size_t myhtml_parser_token_data_to_string_charef(myhtml_tree_t *tree, mycore_string_t* str, myhtml_data_process_entry_t* proc_entry, size_t begin, size_t length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_parser_h */

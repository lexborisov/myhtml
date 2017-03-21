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

#ifndef MyHTML_RULES_H
#define MyHTML_RULES_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <myhtml/myosi.h>
#include <myhtml/myhtml.h>
#include <myhtml/tree.h>

mystatus_t myhtml_rules_init(myhtml_t* myhtml);
void myhtml_rules_stop_parsing(myhtml_tree_t* tree);

bool myhtml_rules_tree_dispatcher(myhtml_tree_t* tree, myhtml_token_node_t* token);
bool myhtml_insertion_mode_in_body_other_end_tag(myhtml_tree_t* tree, myhtml_token_node_t* token);
bool myhtml_insertion_mode_in_body(myhtml_tree_t* tree, myhtml_token_node_t* token);
bool myhtml_insertion_mode_in_template(myhtml_tree_t* tree, myhtml_token_node_t* token);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_rules_h */

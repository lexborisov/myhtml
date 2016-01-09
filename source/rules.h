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

#ifndef MyHTML_RULES_H
#define MyHTML_RULES_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myosi.h"
#include "myhtml.h"
#include "tree.h"


myhtml_status_t myhtml_rules_init(myhtml_t* myhtml);
void myhtml_rules_stop_parsing(myhtml_tree_t* tree);

mybool_t myhtml_rules_tree_dispatcher(myhtml_tree_t* tree, myhtml_token_node_t* token);
mybool_t myhtml_insertion_mode_in_body_other_end_tag(myhtml_tree_t* tree, myhtml_token_node_t* token);
mybool_t myhtml_insertion_mode_in_body(myhtml_tree_t* tree, myhtml_token_node_t* token);
mybool_t myhtml_insertion_mode_in_template(myhtml_tree_t* tree, myhtml_token_node_t* token);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_rules_h */

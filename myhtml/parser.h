//
//  myhtml_parser.h
//  myhtml
//
//  Created by Alexander Borisov on 17.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_parser_h
#define myhtml_parser_h

#include "myosi.h"
#include "myhtml.h"
#include "tree.h"
#include "token.h"
#include "string.h"
#include "utils/mcobject.h"

void myhtml_parser_index(myhtml_tree_t* tree, myhtml_queue_node_index_t queue_idx, myhtml_token_node_t* token);
void myhtml_parser_stream(myhtml_tree_t* tree, myhtml_queue_node_index_t queue_idx, myhtml_token_node_t* token);
void myhtml_parser_worker(myhtml_tree_t* tree, myhtml_queue_node_index_t queue_idx, myhtml_token_node_t* token);

#endif /* myhtml_parser_h */

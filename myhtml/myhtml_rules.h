//
//  myhtml_rules.h
//  myhtml
//
//  Created by Alexander Borisov on 25.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_rules_h
#define myhtml_rules_h

#include "myosi.h"
#include "myhtml.h"
#include "myhtml_tree.h"


void myhtml_rules_init(myhtml_t* myhtml);

mybool_t myhtml_insertion_mode_in_body(myhtml_tree_t* tree, myhtml_token_node_t* token);

#endif /* myhtml_rules_h */
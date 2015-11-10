//
//  myhtml_parse.h
//  myhtml
//
//  Created by Alexander Borisov on 30.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_myhtml_parse_h
#define myhtml_myhtml_parse_h

#include "myosi.h"
#include "myhtml.h"
#include "mytags.h"
#include "myhtml_thread.h"
#include "myhtml_parse_doctype.h"
#include "myhtml_parse_script.h"

#define myhtml_parse_inc_html_offset(__offset__, __size__)   \
    __offset__++;                                            \
    if(__offset__ >= __size__)                               \
        return __offset__

void myhtml_parse_state_init(myhtml_t* myhtml);
void myhtml_parse_state_destroy(myhtml_t* myhtml);

void myhtml_parse_queue_create_text_node_if_need(myhtml_tree_t* tree, const char* html, size_t html_offset);

#endif

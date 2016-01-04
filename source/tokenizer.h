//
//  myhtml_parse.h
//  myhtml
//
//  Created by Alexander Borisov on 30.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_myhtml_tokenizer_h
#define myhtml_myhtml_tokenizer_h

#include "myosi.h"
#include "myhtml.h"
#include "mytags.h"
#include "thread.h"
#include "tokenizer_doctype.h"
#include "tokenizer_script.h"

#define myhtml_tokenizer_inc_html_offset(__offset__, __size__)   \
    __offset__++;                                            \
    if(__offset__ >= __size__)                               \
        return __offset__

void myhtml_tokenizer_begin(myhtml_tree_t* tree, const char* html, size_t html_length);
void myhtml_tokenizer_continue(myhtml_tree_t* tree, const char* html, size_t html_length);
void myhtml_tokenizer_end(myhtml_tree_t* tree, const char* html, size_t html_length);

myhtml_tree_node_t * myhtml_tokenizer_fragment_init(myhtml_tree_t* tree, mytags_ctx_index_t tag_idx, enum myhtml_namespace my_namespace);

void myhtml_tokenizer_wait(myhtml_tree_t* tree);
void myhtml_tokenizer_post(myhtml_tree_t* tree);

myhtml_status_t myhtml_tokenizer_state_init(myhtml_t* myhtml);
void myhtml_tokenizer_state_destroy(myhtml_t* myhtml);

mythread_queue_node_t * myhtml_tokenizer_queue_create_text_node_if_need(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset);

#endif

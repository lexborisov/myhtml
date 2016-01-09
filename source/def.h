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

#ifndef MyHTML_def_H
#define MyHTML_def_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// trash
#define myhtml_else_if_html_end_tag(__else__)                         \
__else__ if(html[html_offset] == '>') {                               \
    mh_queue_get(mh_queue_current(), tag_length) =                    \
    (html_offset - mh_queue_get(mh_queue_current(), tag_begin)) + 1;  \
                                                                      \
    mh_state_set(myhtml_tree) = MyHTML_TOKENIZER_STATE_DATA;          \
    myhtml_queue_node_add(myhtml, mh_queue_current(), myhtml_tree);   \
    mh_queue_current() = myhtml_queue_get_free_id(myhtml);            \
    break;                                                            \
}

#define myhtml_parser_skip_whitespace()                                                      \
if(myhtml_whithspace(html[html_offset], ==, ||)) {                                           \
    while (html_offset < html_size && (myhtml_whithspace(html[html_offset], ==, ||))) {      \
        html_offset++;                                                                       \
    }                                                                                        \
}

#define myhtml_parser_check_size_tag_name()                       \
if(mh_queue_get(qnode_idx, tagname_begin) == html_offset) {       \
    state = MyHTML_TOKENIZER_STATE_DATA;                          \
    break;                                                        \
}

#define myhtml_parser_queue_set_attr(__tree__, __qnode__)                                       \
    if(__qnode__->token->attr_first == NULL) {                                                  \
        __qnode__->token->attr_first = myhtml_tree_token_attr_current(__tree__);                \
        __qnode__->token->attr_last  = __qnode__->token->attr_first;                            \
                                                                                                \
        __tree__->queue_attr = __qnode__->token->attr_last;                                     \
        __tree__->queue_attr->next = NULL;                                                      \
        __tree__->queue_attr->prev = NULL;                                                      \
    }                                                                                           \
    else {                                                                                      \
        __qnode__->token->attr_last->next = myhtml_tree_token_attr_current(__tree__);           \
        __qnode__->token->attr_last->next->prev = __qnode__->token->attr_last;                  \
        __qnode__->token->attr_last = __qnode__->token->attr_last->next;                        \
                                                                                                \
        __qnode__->token->attr_last->next = NULL;                                               \
        __tree__->queue_attr = __qnode__->token->attr_last;                                     \
    }

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

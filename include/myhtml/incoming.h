/*
 Copyright 2016 Alexander Borisov
 
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

#ifndef MyHTML_INCOMING_H
#define MyHTML_INCOMING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/tree.h"
#include "myhtml/utils/mcobject_async.h"

struct myhtml_incoming_buffer {
    const char* data;
    size_t length;      // use of data
    size_t size;        // size of data
    size_t offset;      // begin global offset
    
    struct myhtml_incoming_buffer* prev;
    struct myhtml_incoming_buffer* next;
};


// incoming buffer
myhtml_incoming_buffer_t * myhtml_incomming_buffer_add(myhtml_incoming_buffer_t *current, mcobject_async_t *mcobject, size_t mcnode_id, const char *html, size_t html_size);
void myhtml_incomming_buffer_clean(myhtml_incoming_buffer_t *current);

myhtml_incoming_buffer_t * myhtml_incoming_buffer_find_by_position(myhtml_incoming_buffer_t *inc_buf, size_t position);

size_t myhtml_incoming_buffer_convert_one_escaped_to_code_point(myhtml_incoming_buffer_t **inc_buf, size_t *relative_pos);
size_t myhtml_incoming_buffer_escaped_case_cmp(myhtml_incoming_buffer_t **inc_buf, const char *to, size_t to_size, size_t *relative_pos);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyHTML_INCOMING_H */

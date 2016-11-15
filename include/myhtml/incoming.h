/*
 Copyright (C) 2016 Alexander Borisov
 
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

#ifndef MyHTML_INCOMING_H
#define MyHTML_INCOMING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <myhtml/myosi.h>
#include <myhtml/tree.h>
#include <myhtml/utils/mcobject.h>

struct myhtml_incoming_buffer {
    const char* data;
    size_t length;      // use of data
    size_t size;        // size of data
    size_t offset;      // begin global offset
    
    struct myhtml_incoming_buffer* prev;
    struct myhtml_incoming_buffer* next;
};


// incoming buffer
myhtml_incoming_buffer_t * myhtml_incomming_buffer_add(myhtml_incoming_buffer_t *current, mcobject_t *mcobject, const char *html, size_t html_size);
void myhtml_incomming_buffer_clean(myhtml_incoming_buffer_t *current);
myhtml_incoming_buffer_t * myhtml_incomming_buffer_split(myhtml_incoming_buffer_t *current, mcobject_t *mcobject, size_t global_pos);

myhtml_incoming_buffer_t * myhtml_incoming_buffer_find_by_position(myhtml_incoming_buffer_t *inc_buf, size_t begin);

const char * myhtml_incoming_buffer_data(myhtml_incoming_buffer_t *inc_buf);
size_t myhtml_incoming_buffer_length(myhtml_incoming_buffer_t *inc_buf);
size_t myhtml_incoming_buffer_size(myhtml_incoming_buffer_t *inc_buf);
size_t myhtml_incoming_buffer_offset(myhtml_incoming_buffer_t *inc_buf);
size_t myhtml_incoming_buffer_relative_begin(myhtml_incoming_buffer_t *inc_buf, size_t begin);
size_t myhtml_incoming_buffer_available_length(myhtml_incoming_buffer_t *inc_buf, size_t relative_begin, size_t length);

myhtml_incoming_buffer_t * myhtml_incoming_buffer_next(myhtml_incoming_buffer_t *inc_buf);
myhtml_incoming_buffer_t * myhtml_incoming_buffer_prev(myhtml_incoming_buffer_t *inc_buf);

size_t myhtml_incoming_buffer_convert_one_escaped_to_code_point(myhtml_incoming_buffer_t **inc_buf, size_t *relative_pos);
size_t myhtml_incoming_buffer_escaped_case_cmp(myhtml_incoming_buffer_t **inc_buf, const char *to, size_t to_size, size_t *relative_pos);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyHTML_INCOMING_H */

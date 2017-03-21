/*
 Copyright (C) 2016-2017 Alexander Borisov
 
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

#ifndef MyCORE_INCOMING_H
#define MyCORE_INCOMING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mycore/myosi.h"
#include "mycore/utils/mcobject.h"

struct mycore_incoming_buffer {
    const char* data;
    size_t length;      // use of data
    size_t size;        // size of data
    size_t offset;      // begin global offset
    
    struct mycore_incoming_buffer* prev;
    struct mycore_incoming_buffer* next;
};

mycore_incoming_buffer_t * mycore_incoming_buffer_add(mycore_incoming_buffer_t *current, mcobject_t *mcobject, const char *html, size_t html_size);
void mycore_incoming_buffer_clean(mycore_incoming_buffer_t *current);
mycore_incoming_buffer_t * mycore_incoming_buffer_split(mycore_incoming_buffer_t *current, mcobject_t *mcobject, size_t global_pos);

mycore_incoming_buffer_t * mycore_incoming_buffer_find_by_position(mycore_incoming_buffer_t *inc_buf, size_t begin);

const char * mycore_incoming_buffer_data(mycore_incoming_buffer_t *inc_buf);
size_t mycore_incoming_buffer_length(mycore_incoming_buffer_t *inc_buf);
size_t mycore_incoming_buffer_size(mycore_incoming_buffer_t *inc_buf);
size_t mycore_incoming_buffer_offset(mycore_incoming_buffer_t *inc_buf);
size_t mycore_incoming_buffer_relative_begin(mycore_incoming_buffer_t *inc_buf, size_t begin);
size_t mycore_incoming_buffer_available_length(mycore_incoming_buffer_t *inc_buf, size_t relative_begin, size_t length);

mycore_incoming_buffer_t * mycore_incoming_buffer_next(mycore_incoming_buffer_t *inc_buf);
mycore_incoming_buffer_t * mycore_incoming_buffer_prev(mycore_incoming_buffer_t *inc_buf);

size_t mycore_incoming_buffer_convert_one_escaped_to_code_point(mycore_incoming_buffer_t **inc_buf, size_t *relative_pos);
size_t mycore_incoming_buffer_escaped_case_cmp(mycore_incoming_buffer_t **inc_buf, const char *to, size_t to_size, size_t *relative_pos);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyCORE_INCOMING_H */

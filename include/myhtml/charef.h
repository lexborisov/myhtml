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

#ifndef MyHTML_CHAREF_H
#define MyHTML_CHAREF_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
#include <myhtml/myosi.h>

struct charef_entry {
    unsigned char ch;
    size_t next;
    size_t cur_pos;
    size_t codepoints[2];
    size_t codepoints_len;
}
typedef charef_entry_t;

struct charef_entry_result {
    const charef_entry_t *curr_entry;
    const charef_entry_t *last_entry;
    size_t last_offset;
    int is_done;
}
typedef charef_entry_result_t;

const charef_entry_t * myhtml_charef_find(const char *begin, size_t *offset, size_t size, size_t *data_size);
const charef_entry_t * myhtml_charef_find_by_pos(size_t pos, const char *begin, size_t *offset, size_t size, charef_entry_result_t *result);
const charef_entry_t * myhtml_charef_get_first_position(const char begin);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* charef_h */

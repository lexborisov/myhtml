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

#ifndef MyHTML_CHAREF_H
#define MyHTML_CHAREF_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
#include "myosi.h"

struct charef_entry {
    char ch;
    size_t next;
    size_t cur_pos;
    const char *data;
    size_t data_len;
}
typedef charef_entry_t;

const char * myhtml_charef_find(const char *begin, size_t *offset, size_t size, size_t *data_size);
const charef_entry_t * myhtml_charef_find_by_pos(size_t pos, const char *begin, size_t *offset, size_t size, int *is_done);
const charef_entry_t * myhtml_charef_get_first_position(const char begin);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* charef_h */

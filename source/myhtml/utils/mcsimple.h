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

#ifndef MyHTML_UTILS_MCSIMPLE_H
#define MyHTML_UTILS_MCSIMPLE_H
#pragma once

#include "myhtml/myosi.h"

struct mcsimple {
    size_t  struct_size;
    
    uint8_t **list;
    
    size_t list_pos_size;
    size_t list_pos_length;
    size_t list_pos_length_used;
    size_t list_size;
    size_t list_length;
}
typedef mcsimple_t;


mcsimple_t * mcsimple_create(void);
void mcsimple_init(mcsimple_t *mcsimple, size_t pos_size, size_t list_size, size_t struct_size);
void mcsimple_clean(mcsimple_t *mcsimple);
mcsimple_t * mcsimple_destroy(mcsimple_t *mcsimple, bool destroy_self);

uint8_t * mcsimple_init_list_entries(mcsimple_t *mcsimple, size_t pos);

void * mcsimple_malloc(mcsimple_t *mcsimple);
void * mcsimple_get_by_absolute_position(mcsimple_t *mcsimple, size_t pos);

#endif /* MyHTML_UTILS_MCSIMPLE_H */

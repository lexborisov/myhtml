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

#ifndef MyCORE_UTILS_MCSIMPLE_H
#define MyCORE_UTILS_MCSIMPLE_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mycore/myosi.h"

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


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyCORE_UTILS_MCSIMPLE_H */


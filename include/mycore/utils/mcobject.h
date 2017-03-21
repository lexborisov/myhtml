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

#ifndef MyCORE_UTILS_MCOBJECT_H
#define MyCORE_UTILS_MCOBJECT_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <mycore/myosi.h>

struct mcobject_chunk {
    unsigned char *begin;
    size_t length;
    size_t size;
    
    struct mcobject_chunk *next;
    struct mcobject_chunk *prev;
}
typedef mcobject_chunk_t;

struct mcobject {
    mcobject_chunk_t *chunk;
    
    void  **cache;
    size_t  cache_size;
    size_t  cache_length;
    
    size_t struct_size;
    size_t chunk_size;
}
typedef mcobject_t;


mcobject_t * mcobject_create(void);
mystatus_t mcobject_init(mcobject_t *mcobject, size_t chunk_size, size_t struct_size);
void mcobject_clean(mcobject_t *mcobject);
mcobject_t * mcobject_destroy(mcobject_t *mcobject, bool destroy_self);

void mcobject_chunk_malloc(mcobject_t* mcobject, mystatus_t* status);

void * mcobject_malloc(mcobject_t *mcobject, mystatus_t* status);
mystatus_t mcobject_free(mcobject_t *mcobject, void *entry);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyCORE_UTILS_MCOBJECT_H */


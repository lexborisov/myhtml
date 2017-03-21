/*
 Copyright (C) 2017 Alexander Borisov
 
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
 Foundation, Inc., 51 Franklin avl_treet, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#ifndef MyCORE_UTILS_MHASH_H
#define MyCORE_UTILS_MHASH_H

#include <string.h>

#include "mycore/myosi.h"
#include "mycore/utils/mchar_async.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mycore_utils_mhash_entry mycore_utils_mhash_entry_t;

struct mycore_utils_mhash_entry {
    char* key;
    size_t key_length;
    
    void *value;
    
    mycore_utils_mhash_entry_t* next;
};

struct mycore_utils_mhash {
    mchar_async_t* mchar_obj;
    size_t mchar_node;
    
    mycore_utils_mhash_entry_t** table;
    size_t table_size;
    size_t table_length;
    
    size_t table_max_depth;
}
typedef mycore_utils_mhash_t;

mycore_utils_mhash_t * mycore_utils_mhash_create(void);
mystatus_t mycore_utils_mhash_init(mycore_utils_mhash_t* mhash, size_t table_size, size_t depth);
void mycore_utils_mhash_clean(mycore_utils_mhash_t* mhash);
mycore_utils_mhash_t * mycore_utils_mhash_destroy(mycore_utils_mhash_t* mhash, bool self_destroy);
mycore_utils_mhash_entry_t * mycore_utils_mhash_create_entry(mycore_utils_mhash_t* mhash, const char* key, size_t key_size, void* value);

mycore_utils_mhash_entry_t * mycore_utils_mhash_add(mycore_utils_mhash_t* mhash, const char* key, size_t key_size, void* value);
mycore_utils_mhash_entry_t * mycore_utils_mhash_search(mycore_utils_mhash_t* mhash, const char* key, size_t key_size, void* value);
mycore_utils_mhash_entry_t * mycore_utils_mhash_add_with_choice(mycore_utils_mhash_t* mhash, const char* key, size_t key_size);

mycore_utils_mhash_entry_t * mycore_utils_mhash_entry_by_id(mycore_utils_mhash_t* mhash, size_t id);
size_t mycore_utils_mhash_get_table_size(mycore_utils_mhash_t* mhash);

mycore_utils_mhash_entry_t ** mycore_utils_mhash_rebuld(mycore_utils_mhash_t* mhash);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyCORE_UTILS_MHASH_H */

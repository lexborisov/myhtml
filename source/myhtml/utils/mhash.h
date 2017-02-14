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

#ifndef html_mhash_h
#define html_mhash_h

#include "myhtml/myosi.h"
#include "myhtml/utils/mchar_async.h"

typedef struct myhtml_utils_mhash_entry myhtml_utils_mhash_entry_t;

struct myhtml_utils_mhash_entry {
    char* key;
    size_t key_length;
    
    void *value;
    
    myhtml_utils_mhash_entry_t* next;
};

struct myhtml_utils_mhash {
    mchar_async_t* mchar_obj;
    size_t mchar_node;
    
    myhtml_utils_mhash_entry_t** table;
    size_t table_size;
    size_t table_length;
    
    size_t table_max_depth;
}
typedef myhtml_utils_mhash_t;

myhtml_utils_mhash_t * myhtml_utils_mhash_create(void);
myhtml_status_t myhtml_utils_mhash_init(myhtml_utils_mhash_t* mhash, size_t table_size, size_t depth);
void myhtml_utils_mhash_clean(myhtml_utils_mhash_t* mhash);
myhtml_utils_mhash_t * myhtml_utils_mhash_destroy(myhtml_utils_mhash_t* mhash, bool self_destroy);
myhtml_utils_mhash_entry_t * myhtml_utils_mhash_create_entry(myhtml_utils_mhash_t* mhash, const char* key, size_t key_size, void* value);

myhtml_utils_mhash_entry_t * myhtml_utils_mhash_add(myhtml_utils_mhash_t* mhash, const char* key, size_t key_size, void* value);
myhtml_utils_mhash_entry_t * myhtml_utils_mhash_search(myhtml_utils_mhash_t* mhash, const char* key, size_t key_size, void* value);
myhtml_utils_mhash_entry_t * myhtml_utils_mhash_add_with_choice(myhtml_utils_mhash_t* mhash, const char* key, size_t key_size);

myhtml_utils_mhash_entry_t * myhtml_utils_mhash_entry_by_id(myhtml_utils_mhash_t* mhash, size_t id);
size_t myhtml_utils_mhash_get_table_size(myhtml_utils_mhash_t* mhash);

myhtml_utils_mhash_entry_t ** myhtml_utils_mhash_rebuld(myhtml_utils_mhash_t* mhash);

#endif

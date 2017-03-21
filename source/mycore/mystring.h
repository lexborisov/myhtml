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

#ifndef MyCORE_MYSTRING_H
#define MyCORE_MYSTRING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "mycore/myosi.h"
#include "mycore/utils/mchar_async.h"
#include "mycore/utils.h"
    
#define mycore_string_get(str, attr) str->attr
#define mycore_string_set(str, attr) mycore_string_get(str, attr)
#define mycore_string_len(str) mycore_string_get(str, length)

#define MyCORE_STRING_PREPROCESSOR_CHECK_BUFFER_SIZE(str, unsigned_data) \
    if(str->length >= str->size) {                                       \
        mycore_string_realloc(str, (str->length + 5));                   \
                                                                         \
        unsigned_data = (unsigned char*)str->data;                       \
    }

#define MyCORE_STRING_REALLOC_IF_NEED(str, need_length, upto)               \
    if((str->length + need_length) >= str->size) {                          \
        mycore_string_realloc(str, (str->length + need_length + upto));     \
    }                                                                       \

#define MyCORE_STRING_APPEND_BYTE_WITHOUT_REALLOC(onechar, str)   \
    str->data[str->length] = onechar;                             \
    str->length++

#define MyCORE_STRING_APPEND_BYTE_WITHOUT_INCREMENT(onechar, str, upto)   \
    if(str->length >= str->size) {                                        \
        mycore_string_realloc(str, (str->length + upto));                 \
    }                                                                     \
    str->data[str->length] = onechar

#define MyCORE_STRING_APPEND_BYTE_WITHOUT_INCREMENT_REALLOC(onechar, str) \
    str->data[str->length] = onechar

#define MyCORE_STRING_APPEND_BYTE(onechar, str, upto)                \
    MyCORE_STRING_APPEND_BYTE_WITHOUT_INCREMENT(onechar, str, upto); \
    str->length++

struct mycore_string {
    char*  data;
    size_t size;
    size_t length;
    
    mchar_async_t *mchar;
    size_t node_idx;
};

struct mycore_string_raw {
    char*  data;
    size_t size;
    size_t length;
};

typedef size_t mycore_string_index_t;

char * mycore_string_init(mchar_async_t *mchar, size_t node_idx, mycore_string_t* str, size_t size);
char * mycore_string_realloc(mycore_string_t *str, size_t new_size);

void mycore_string_clean(mycore_string_t* str);
void mycore_string_clean_all(mycore_string_t* str);
mycore_string_t * mycore_string_destroy(mycore_string_t* str, bool destroy_obj);

void mycore_string_raw_clean(mycore_string_raw_t* str_raw);
void mycore_string_raw_clean_all(mycore_string_raw_t* str_raw);
mycore_string_raw_t * mycore_string_raw_destroy(mycore_string_raw_t* str_raw, bool destroy_obj);

/* basic api */
char * mycore_string_data_alloc(mchar_async_t *mchar, size_t node_id, size_t size);
char * mycore_string_data_realloc(mchar_async_t *mchar, size_t node_id, char *data,  size_t len_to_copy, size_t size);
void mycore_string_data_free(mchar_async_t *mchar, size_t node_id, char *data);

char * mycore_string_data(mycore_string_t *str);
size_t mycore_string_length(mycore_string_t *str);
size_t mycore_string_size(mycore_string_t *str);
char * mycore_string_data_set(mycore_string_t *str, char *data);
size_t mycore_string_size_set(mycore_string_t *str, size_t size);
size_t mycore_string_length_set(mycore_string_t *str, size_t length);

/* append */
void mycore_string_append(mycore_string_t* str, const char* data, size_t length);
void mycore_string_append_one(mycore_string_t* str, const char data);
void mycore_string_append_lowercase(mycore_string_t* str, const char* data, size_t length);
void mycore_string_append_with_replacement_null_characters(mycore_string_t* str, const char* buff, size_t length);

size_t mycore_string_raw_set_replacement_character(mycore_string_t* target, size_t position);

/* other functions */
void mycore_string_copy(mycore_string_t* dest, mycore_string_t* target);
size_t mycore_string_raw_copy(char* str1, const char* str2, size_t size);
void mycore_string_stay_only_whitespace(mycore_string_t* target);
size_t mycore_string_crop_whitespace_from_begin(mycore_string_t* target);
size_t mycore_string_whitespace_from_begin(mycore_string_t* target);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* mycore_mystring_h */

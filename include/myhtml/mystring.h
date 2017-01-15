/*
 Copyright (C) 2015-2016 Alexander Borisov
 
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

#ifndef MyHTML_MYSTRING_H
#define MyHTML_MYSTRING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <myhtml/myosi.h>
#include <myhtml/encoding.h>
#include <myhtml/utils/mchar_async.h>

#define myhtml_string_get(str, attr) str->attr
#define myhtml_string_set(str, attr) myhtml_string_get(str, attr)
#define myhtml_string_len(str) myhtml_string_get(str, length)

#define MyHTML_STRING_PREPROCESSOR_CHECK_BUFFER_SIZE(str, unsigned_data) \
    if(str->length >= str->size) {                                       \
        myhtml_string_realloc(str, (str->length + 5));                   \
                                                                         \
        unsigned_data = (unsigned char*)str->data;                       \
    }

#define MyHTML_STRING_REALLOC_IF_NEED(str, need_length, upto)               \
    if((str->length + need_length) >= str->size) {                          \
        myhtml_string_realloc(str, (str->length + need_length + upto));     \
    }                                                                       \

#define MyHTML_STRING_APPEND_BYTE_WITHOUT_REALLOC(onechar, str)   \
    str->data[str->length] = onechar;                             \
    str->length++

#define MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT(onechar, str, upto)   \
    if(str->length >= str->size) {                                        \
        myhtml_string_realloc(str, (str->length + upto));                 \
    }                                                                     \
    str->data[str->length] = onechar

#define MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT_REALLOC(onechar, str) \
    str->data[str->length] = onechar

#define MyHTML_STRING_APPEND_BYTE(onechar, str, upto)                \
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT(onechar, str, upto); \
    str->length++

#define myhtml_mystring_whitespace(onechar, action, logic)    \
    (onechar action ' '  logic                                \
     onechar action '\t' logic                                \
     onechar action '\n' logic                                \
     onechar action '\f' logic                                \
     onechar action '\r')

struct myhtml_string {
    char*  data;
    size_t size;
    size_t length;
    
    mchar_async_t *mchar;
    size_t node_idx;
};

struct myhtml_string_raw {
    char*  data;
    size_t size;
    size_t length;
};

typedef size_t myhtml_string_index_t;

char * myhtml_string_init(mchar_async_t *mchar, size_t node_idx, myhtml_string_t* str, size_t size);
char * myhtml_string_realloc(myhtml_string_t *str, size_t new_size);

void myhtml_string_clean(myhtml_string_t* str);
void myhtml_string_clean_all(myhtml_string_t* str);
myhtml_string_t * myhtml_string_destroy(myhtml_string_t* str, bool destroy_obj);

void myhtml_string_raw_clean(myhtml_string_raw_t* str_raw);
void myhtml_string_raw_clean_all(myhtml_string_raw_t* str_raw);
myhtml_string_raw_t * myhtml_string_raw_destroy(myhtml_string_raw_t* str_raw, bool destroy_obj);

/* basic api */
char * myhtml_string_data_alloc(mchar_async_t *mchar, size_t node_id, size_t size);
char * myhtml_string_data_realloc(mchar_async_t *mchar, size_t node_id, char *data,  size_t len_to_copy, size_t size);
void myhtml_string_data_free(mchar_async_t *mchar, size_t node_id, char *data);

char * myhtml_string_data(myhtml_string_t *str);
size_t myhtml_string_length(myhtml_string_t *str);
size_t myhtml_string_size(myhtml_string_t *str);
char * myhtml_string_data_set(myhtml_string_t *str, char *data);
size_t myhtml_string_size_set(myhtml_string_t *str, size_t size);
size_t myhtml_string_length_set(myhtml_string_t *str, size_t length);

/* append */
void myhtml_string_append(myhtml_string_t* str, const char* data, size_t length);
void myhtml_string_append_one(myhtml_string_t* str, const char data);
void myhtml_string_append_lowercase(myhtml_string_t* str, const char* data, size_t length);
void myhtml_string_append_with_convert_encoding(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding);

/* append with convert encoding */
void myhtml_string_append_chunk_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding);
void myhtml_string_append_one_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char data, myhtml_encoding_t encoding);

/* append with convert encoding lowercase */
void myhtml_string_append_lowercase_ascii_with_convert_encoding(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding);
void myhtml_string_append_chunk_lowercase_ascii_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding);

/* Oh, long long long name */

/* append with convert encoding with preprocessing */
size_t myhtml_string_append_with_convert_encoding_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding, bool emit_null_chars);
size_t myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding, bool emit_null_chars);

/* append with convert encoding lowercase with preprocessing */
size_t myhtml_string_append_lowercase_with_convert_encoding_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding, bool emit_null_chars);
size_t myhtml_string_append_lowercase_chunk_with_convert_encoding_with_preprocessing(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding, bool emit_null_chars);
    
void myhtml_string_append_with_replacement_null_characters_only(myhtml_string_t* str, const char* buff, size_t length);

/* append with preprocessing */
size_t myhtml_string_before_append_any_preprocessing(myhtml_string_t* str, const char* buff, size_t length, size_t last_position);
size_t myhtml_string_append_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, bool emit_null_chars);
size_t myhtml_string_append_lowercase_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, bool emit_null_chars);
size_t myhtml_string_raw_set_replacement_character(myhtml_string_t* target, size_t position);

/* other functions */
void myhtml_string_copy(myhtml_string_t* dest, myhtml_string_t* target);
size_t myhtml_string_raw_copy(char* str1, const char* str2, size_t size);
void myhtml_string_stay_only_whitespace(myhtml_string_t* target);
size_t myhtml_string_crop_whitespace_from_begin(myhtml_string_t* target);
size_t myhtml_string_whitespace_from_begin(myhtml_string_t* target);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_mystring_h */

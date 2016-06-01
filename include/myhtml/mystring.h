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
#include <myhtml/charef.h>
#include <myhtml/encoding.h>
#include <myhtml/utils/mchar_async.h>

#define myhtml_string_get(str, attr) str->attr
#define myhtml_string_set(str, attr) myhtml_string_get(str, attr)
#define myhtml_string_len(str) myhtml_string_get(str, length)

#define MyHTML_STRING_REALLOC_IF_NEED(str, count, upto)           \
    if((str->length + count) >= str->size) {                      \
        myhtml_string_release(str, (str->length + count + upto)); \
    }                                                             \

#define MyHTML_STRING_APPEND_BYTE_WITHOUT_REALLOC(onechar, str)   \
    str->data[str->length] = onechar;                             \
    str->length++

#define MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT(onechar, str, upto)   \
    if(str->length >= str->size) {                                        \
        myhtml_string_release(str, (str->size + upto));                   \
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

struct myhtml_string_char_ref_chunk {
    int state;
    size_t begin;
    long l_data;
    
    charef_entry_result_t charef_res;
    
    bool is_attributes;
    bool emit_null_char;
    
    myhtml_encoding_t encoding;
    myhtml_encoding_result_t res;
}
typedef myhtml_string_char_ref_chunk_t;

typedef size_t myhtml_string_index_t;

char * myhtml_string_init(mchar_async_t *mchar, size_t node_idx, myhtml_string_t* str, size_t size);
char * myhtml_string_realloc(mchar_async_t *mchar, size_t node_id, myhtml_string_t *str, size_t new_size);

void myhtml_string_clean(myhtml_string_t* str);
void myhtml_string_clean_all(myhtml_string_t* str);
myhtml_string_t * myhtml_string_destroy(myhtml_string_t* str, bool destroy_obj);

char * myhtml_string_data_alloc(mchar_async_t *mchar, size_t node_id, size_t size);
char * myhtml_string_data_realloc(mchar_async_t *mchar, size_t node_id, char *data,  size_t len_to_copy, size_t size);
void myhtml_string_data_free(mchar_async_t *mchar, size_t node_id, char *data);

char * myhtml_string_data(myhtml_string_t *str);
size_t myhtml_string_length(myhtml_string_t *str);
size_t myhtml_string_size(myhtml_string_t *str);
char * myhtml_string_data_set(myhtml_string_t *str, char *data);
size_t myhtml_string_size_set(myhtml_string_t *str, size_t size);
size_t myhtml_string_length_set(myhtml_string_t *str, size_t length);

void myhtml_string_append(myhtml_string_t* str, const char* data, size_t length);
void myhtml_string_append_one(myhtml_string_t* str, const char data);
void myhtml_string_append_lowercase(myhtml_string_t* str, const char* data, size_t length);
void myhtml_string_append_charef(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t length);
void myhtml_string_append_charef_end(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str);
void myhtml_string_append_with_convert_encoding(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding);

void myhtml_string_append_chunk_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res,
                                                      const char* buff, size_t length, myhtml_encoding_t encoding);

void myhtml_string_append_one_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res,
                                                    const char data, myhtml_encoding_t encoding);

void myhtml_string_append_lowercase_ascii_with_convert_encoding(myhtml_string_t* str, const char* buff, size_t length,
                                                                myhtml_encoding_t encoding);

void myhtml_string_append_chunk_lowercase_ascii_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res,
                                                                      const char* buff, size_t length, myhtml_encoding_t encoding);

void myhtml_string_append_with_convert_encoding_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length,
                                                                   myhtml_encoding_t encoding);
    
void myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(myhtml_string_t* str, myhtml_encoding_result_t* res,
                                                                         const char* buff, size_t length,
                                                                         myhtml_encoding_t encoding);
void myhtml_string_append_with_replacement_null_characters_only(myhtml_string_t* str, const char* buff, size_t length);

void myhtml_string_append_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, bool emit_null_chars);
void myhtml_string_append_lowercase_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length);
size_t myhtml_string_raw_set_replacement_character(myhtml_string_t* target, size_t position);

void myhtml_string_copy(myhtml_string_t* dest, myhtml_string_t* target);
size_t myhtml_string_raw_copy(char* str1, const char* str2, size_t size);
void myhtml_string_stay_only_whitespace(myhtml_string_t* target);
size_t myhtml_string_crop_whitespace_from_begin(myhtml_string_t* target);
size_t myhtml_string_whitespace_from_begin(myhtml_string_t* target);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_mystring_h */

/*
 Copyright 2015-2016 Alexander Borisov
 
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

#ifndef MyHTML_MYSTRING_H
#define MyHTML_MYSTRING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/charef.h"
#include "myhtml/encoding.h"
#include "myhtml/utils/mchar_async.h"

#define myhtml_string_get(__str__, __attr__) __str__->__attr__
#define myhtml_string_set(__str__, __attr__) myhtml_string_get(__str__, __attr__)
#define myhtml_string_len(__str__) myhtml_string_get(__str__, length)

#define MyHTML_STRING_REALLOC_IF_NEED(__str__, __count__, __upto__)         \
    if((__str__->length + __count__) >= __str__->size) {                    \
        myhtml_string_release(__str__, (str->size + __upto__));             \
    }                                                                       \

#define MyHTML_STRING_APPEND_BYTE_WITHOUT_REALLOC(__byte__, __str__)   \
    __str__->data[__str__->length] = __byte__;                         \
    __str__->length++

#define MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT(__byte__, __str__, __upto__)   \
    if(__str__->length >= __str__->size) {                                         \
        myhtml_string_release(__str__, (str->size + __upto__));                    \
    }                                                                              \
    __str__->data[__str__->length] = __byte__

#define MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT_REALLOC(__byte__, __str__) \
    __str__->data[__str__->length] = __byte__

#define MyHTML_STRING_APPEND_BYTE(__byte__, __str__, __upto__)                \
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT(__byte__, __str__, __upto__); \
    __str__->length++

#define myhtml_mystring_whithspace(__char__, __action__, __logic__)    \
    __char__ __action__ ' ' __logic__                     \
    __char__ __action__ '\t' __logic__                    \
    __char__ __action__ '\n' __logic__                    \
    __char__ __action__ '\f' __logic__                    \
    __char__ __action__ '\r'

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
    const charef_entry_t *entry;
    
    myhtml_encoding_t encoding;
    myhtml_encoding_result_t res;
}
typedef myhtml_string_char_ref_chunk_t;

typedef size_t myhtml_string_index_t;

void myhtml_string_init(myhtml_string_t* str, mchar_async_t *mchar, size_t node_idx, size_t size);
void myhtml_string_clean(myhtml_string_t* str);
void myhtml_string_clean_all(myhtml_string_t* str);
myhtml_string_t * myhtml_string_destroy(myhtml_string_t* str, mybool_t destroy_obj);

void myhtml_string_append(myhtml_string_t* str, const char* data, size_t length);
void myhtml_string_append_one(myhtml_string_t* str, const char data);
void myhtml_string_append_lowercase(myhtml_string_t* str, const char* data, size_t length);
void myhtml_string_append_charef(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t length);
void myhtml_string_append_charef_end(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str);
void myhtml_string_append_with_convert_encoding(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding);
void myhtml_string_append_chunk_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding);
void myhtml_string_append_one_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char data, myhtml_encoding_t encoding);

void myhtml_string_copy(myhtml_string_t* target, myhtml_string_t* dest);
size_t myhtml_string_raw_copy(char* str1, const char* str2, size_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_mystring_h */

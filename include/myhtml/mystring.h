/*
 Copyright 2015 Alexander Borisov
 
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

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/utils/mchar_async.h"

#define myhtml_string_get(__str__, __attr__) __str__->__attr__
#define myhtml_string_set(__str__, __attr__) myhtml_string_get(__str__, __attr__)
#define myhtml_string_len(__str__) myhtml_string_get(__str__, length)

struct myhtml_string {
    char*  data;
    size_t size;
    size_t length;
    
    mchar_async_t *mchar;
    size_t node_idx;
}
typedef myhtml_string_t;

typedef size_t myhtml_string_index_t;

void myhtml_string_init(myhtml_string_t* str, mchar_async_t *mchar, size_t node_idx, size_t size);
void myhtml_string_clean(myhtml_string_t* str);
myhtml_string_t * myhtml_string_destroy(myhtml_string_t* str, mybool_t destroy_obj);

void myhtml_string_append(myhtml_string_t* str, const char* buff, size_t length);
void myhtml_string_append_with_null(myhtml_string_t* str, const char* buff, size_t length);
void myhtml_string_append_one(myhtml_string_t* str, const char buff);
void myhtml_string_append_one_without_check(myhtml_string_t* str, const char buff);
void myhtml_string_append_lowercase_with_null(myhtml_string_t* str, const char* buff, size_t length);
void myhtml_string_copy(myhtml_string_t* target, myhtml_string_t* dest);

mybool_t myhtml_string_check(myhtml_string_t* str, size_t length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_mystring_h */

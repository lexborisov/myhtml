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

#ifndef MyHTML_API_ENCODING_H
#define MyHTML_API_ENCODING_H
#pragma once

#include "myhtml/api.h"

#ifdef __cplusplus
extern "C" {
#endif

enum myhtml_encoding_status {
    MyHTML_ENCODING_STATUS_OK       = 0x00,
    MyHTML_ENCODING_STATUS_ERROR    = 0x01,
    MyHTML_ENCODING_STATUS_CONTINUE = 0x02,
    MyHTML_ENCODING_STATUS_DONE     = 0x04
}
typedef myhtml_encoding_status_t;

struct myhtml_encoding_result {
    unsigned long first;
    unsigned long second;
    unsigned long third;
    unsigned long result;
    unsigned long result_aux;
    unsigned long flag;
}
typedef myhtml_encoding_result_t;

typedef myhtml_encoding_status_t
(*myhtml_encoding_custom_f)(unsigned const char byte, myhtml_encoding_result_t *res);

/***********************************************************************************
 *
 * MyHTML_ENCODING
 *
 ***********************************************************************************/

/**
 * Clean myhtml_encoding_result_t. Use this function before and after text decode
 *
 * @param[in] myhtml_encoding_result_t*
 */
void
myhtml_encoding_result_clean(myhtml_encoding_result_t *res);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* api_encoding_h */

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

#ifndef MyHTML_API_ENCODING_H
#define MyHTML_API_ENCODING_H
#pragma once

#include <myhtml/api.h>

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

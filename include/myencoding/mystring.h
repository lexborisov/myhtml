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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#ifndef MyENCODING_MYSTRING_H
#define MyENCODING_MYSTRING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <myencoding/myosi.h>
#include <myencoding/encoding.h>
#include <mycore/mystring.h>
#include <mycore/utils.h>

void myencoding_string_append(mycore_string_t* str, const char* buff, size_t length, myencoding_t encoding);

/* append with convert encoding */
void myencoding_string_append_chunk(mycore_string_t* str, myencoding_result_t* res, const char* buff, size_t length, myencoding_t encoding);
void myencoding_string_append_one(mycore_string_t* str, myencoding_result_t* res, const char data, myencoding_t encoding);

/* append with convert encoding lowercase */
void myencoding_string_append_lowercase_ascii(mycore_string_t* str, const char* buff, size_t length, myencoding_t encoding);
void myencoding_string_append_chunk_lowercase_ascii(mycore_string_t* str, myencoding_result_t* res, const char* buff, size_t length, myencoding_t encoding);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyENCODING_MYSTRING_H */

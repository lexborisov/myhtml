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

#ifndef MyHTML_MYSTRING_H
#define MyHTML_MYSTRING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
#include "myhtml/myosi.h"

#include "myencoding/encoding.h"
#include "myencoding/mystring.h"

#include "mycore/mystring.h"
#include "mycore/utils/mchar_async.h"

#define myhtml_string_get(str, attr) str->attr
#define myhtml_string_set(str, attr) myhtml_string_get(str, attr)
#define myhtml_string_len(str) myhtml_string_get(str, length)

/* append with convert encoding with preprocessing */
size_t myhtml_string_append_with_convert_encoding_with_preprocessing(mycore_string_t* str, const char* buff, size_t length, myencoding_t encoding, bool emit_null_chars);
size_t myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(mycore_string_t* str, myencoding_result_t* res, const char* buff, size_t length, myencoding_t encoding, bool emit_null_chars);

/* append with convert encoding lowercase with preprocessing */
size_t myhtml_string_append_lowercase_with_convert_encoding_with_preprocessing(mycore_string_t* str, const char* buff, size_t length, myencoding_t encoding, bool emit_null_chars);
size_t myhtml_string_append_lowercase_chunk_with_convert_encoding_with_preprocessing(mycore_string_t* str, myencoding_result_t* res, const char* buff, size_t length, myencoding_t encoding, bool emit_null_chars);

/* append with preprocessing */
size_t myhtml_string_before_append_any_preprocessing(mycore_string_t* str, const char* buff, size_t length, size_t last_position);
size_t myhtml_string_append_with_preprocessing(mycore_string_t* str, const char* buff, size_t length, bool emit_null_chars);
size_t myhtml_string_append_lowercase_with_preprocessing(mycore_string_t* str, const char* buff, size_t length, bool emit_null_chars);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_mystring_h */

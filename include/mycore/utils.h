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

#ifndef MyCORE_UTILS_H
#define MyCORE_UTILS_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <mycore/myosi.h>

#define mycore_utils_whithspace(onechar, action, logic)     \
    (onechar action ' '  logic                              \
     onechar action '\t' logic                              \
     onechar action '\n' logic                              \
     onechar action '\f' logic                              \
     onechar action '\r')

size_t mycore_power(size_t t, size_t k);
size_t mycore_strncasecmp(const char* str1, const char* str2, size_t size);
size_t mycore_strcasecmp(const char* str1, const char* str2);
size_t mycore_strncmp(const char* str1, const char* str2, size_t size);
size_t mycore_strcmp(const char* str1, const char* str2);
size_t mycore_strcmp_ws(const char* str1, const char* str2);
bool mycore_ustrcasecmp_without_checks_by_secondary(const unsigned char* ustr1, const unsigned char* ustr2);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* utils_h */

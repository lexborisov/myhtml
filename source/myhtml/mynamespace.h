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

#ifndef MyHTML_MYNAMESPACE_H
#define MyHTML_MYNAMESPACE_H
#pragma once

#define MyHTML_NAMESPACE_DETECT_NAME_STATIC_SIZE 19

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "myhtml/myosi.h"
#include "mycore/utils.h"

struct myhtml_namespace_detect_name_entry {
    const char* name;
    size_t name_length;
    
    myhtml_namespace_t ns;
    
    size_t next;
    size_t curr;
}
typedef myhtml_namespace_detect_name_entry_t;

struct myhtml_namespace_detect_url_entry {
    const char* url;
    size_t url_length;
    
    myhtml_namespace_t ns;
}
typedef myhtml_namespace_detect_url_entry_t;

const char * myhtml_namespace_name_by_id(myhtml_namespace_t ns, size_t *length);
bool myhtml_namespace_id_by_name(const char *name, size_t length, myhtml_namespace_t *ns);
const char * myhtml_namespace_url_by_id(myhtml_namespace_t ns, size_t *length);
myhtml_namespace_t myhtml_namespace_id_by_url(const char *url, size_t length);
const myhtml_namespace_detect_name_entry_t * myhtml_namespace_name_entry_by_name(const char* name, size_t length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyHTML_MYNAMESPACE_H */

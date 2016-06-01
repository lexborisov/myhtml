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

#include "myhtml/mynamespace.h"
#include "myhtml/mynamespace_resource.h"
#include "myhtml/utils/resources.h"

const myhtml_namespace_detect_name_entry_t * myhtml_namespace_name_entry_by_name(const char* name, size_t length)
{
    size_t idx = ((myhtml_string_chars_lowercase_map[ (const unsigned char)name[0] ] *
                   myhtml_string_chars_lowercase_map[ (const unsigned char)name[(length - 1)] ] *
                   length)
                  % MyHTML_NAMESPACE_DETECT_NAME_STATIC_SIZE) + 1;
    
    while (myhtml_namespace_detect_name_entry_static_list_index[idx].name)
    {
        if(myhtml_namespace_detect_name_entry_static_list_index[idx].name_length == length) {
            if(myhtml_strncasecmp(myhtml_namespace_detect_name_entry_static_list_index[idx].name, name, length) == 0)
                return &myhtml_namespace_detect_name_entry_static_list_index[idx];
            
            if(myhtml_namespace_detect_name_entry_static_list_index[idx].next)
                idx = myhtml_namespace_detect_name_entry_static_list_index[idx].next;
            else
                return NULL;
        }
        else if(myhtml_namespace_detect_name_entry_static_list_index[idx].name_length > length) {
            return NULL;
        }
        else {
            idx = myhtml_namespace_detect_name_entry_static_list_index[idx].next;
        }
    }
    
    return NULL;
}

bool myhtml_namespace_by_name(const char *name, size_t length, myhtml_namespace_t *ns)
{
    const myhtml_namespace_detect_name_entry_t *entry = myhtml_namespace_name_entry_by_name(name, length);
    
    if(entry) {
        if(ns)
            *ns = entry->ns;
        
        return true;
    }
    
    return false;
}




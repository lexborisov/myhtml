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

#include "myhtml/mynamespace.h"
#include "myhtml/mynamespace_resource.h"
#include "mycore/utils/resources.h"

const myhtml_namespace_detect_name_entry_t * myhtml_namespace_name_entry_by_name(const char* name, size_t length)
{
    size_t idx = ((mycore_string_chars_lowercase_map[ (const unsigned char)name[0] ] *
                   mycore_string_chars_lowercase_map[ (const unsigned char)name[(length - 1)] ] *
                   length)
                  % MyHTML_NAMESPACE_DETECT_NAME_STATIC_SIZE) + 1;
    
    while (myhtml_namespace_detect_name_entry_static_list_index[idx].name)
    {
        if(myhtml_namespace_detect_name_entry_static_list_index[idx].name_length == length) {
            if(mycore_strncasecmp(myhtml_namespace_detect_name_entry_static_list_index[idx].name, name, length) == 0)
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

bool myhtml_namespace_id_by_name(const char *name, size_t length, myhtml_namespace_t *ns)
{
    const myhtml_namespace_detect_name_entry_t *entry = myhtml_namespace_name_entry_by_name(name, length);
    
    if(entry) {
        if(ns)
            *ns = entry->ns;
        
        return true;
    }
    
    return false;
}

const char * myhtml_namespace_name_by_id(myhtml_namespace_t ns, size_t *length)
{
    if(ns > MyHTML_NAMESPACE_LAST_ENTRY) {
        if(length)
            *length = 0;
        
        return NULL;
    }
    
    const char *name_ns = myhtml_namespace_resource_names_map[ns];
    
    if(length)
        *length = strlen(name_ns);
    
    return name_ns;
}

const char * myhtml_namespace_url_by_id(myhtml_namespace_t ns, size_t *length)
{
    if(ns > MyHTML_NAMESPACE_LAST_ENTRY) {
        if(length)
            *length = 0;
        
        return NULL;
    }
    
    const myhtml_namespace_detect_url_entry_t *url_entry = &myhtml_namespace_detect_url_entry_static_list_index[ns];
    
    if(length) {
        *length = url_entry->url_length;
    }
    
    return url_entry->url;
}

myhtml_namespace_t myhtml_namespace_id_by_url(const char *url, size_t length)
{
    size_t i = 0;
    while(myhtml_namespace_detect_url_entry_static_list_index[i].ns)
    {
        if(myhtml_namespace_detect_url_entry_static_list_index[i].url_length == length) {
            size_t k = length;
            
            const unsigned char *u_url = (const unsigned char*)myhtml_namespace_detect_url_entry_static_list_index[i].url;
            const unsigned char *u_target_url = (const unsigned char*)url;
            
            while(k) {
                k--;
                
                if(u_url[k] != u_target_url[k]) {
                    i++;
                    continue;
                }
            }
            
            return myhtml_namespace_detect_url_entry_static_list_index[i].ns;
        }
        
        i++;
    }
    
    return MyHTML_NAMESPACE_UNDEF;
}



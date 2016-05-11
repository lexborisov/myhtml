/*
 Copyright (C) 2015-2016 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#include "myhtml/utils/resources.h"
#include "tag.h"

%CONST%


%STATIC_LIST%

const myhtml_tag_context_t * myhtml_tag_static_search(const char* name, size_t length)
{
    size_t idx = ((myhtml_string_chars_lowercase_map[ (const unsigned char)name[0] ] *
                   myhtml_string_chars_lowercase_map[ (const unsigned char)name[(length - 1)] ] *
                   length)
                  % MyHTML_BASE_STATIC_SIZE) + 1;

    while (myhtml_tag_static_list_index[idx].ctx)
    {
        if(myhtml_tag_static_list_index[idx].ctx->name_length == length) {
            if(myhtml_strncasecmp(myhtml_tag_static_list_index[idx].ctx->name, name, length) == 0)
                return myhtml_tag_static_list_index[idx].ctx;

            if(myhtml_tag_static_list_index[idx].next)
                idx = myhtml_tag_static_list_index[idx].next;
            else
                return NULL;
        }
        else if(myhtml_tag_static_list_index[idx].ctx->name_length > length) {
            return NULL;
        }
        else {
            idx = myhtml_tag_static_list_index[idx].next;
        }
    }

    return NULL;
}

const myhtml_tag_context_t * myhtml_tag_static_get_by_id(size_t idx)
{
    return &myhtml_tag_base_list[idx];
}




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




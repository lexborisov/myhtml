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

#include "myencoding/mystring.h"
#include "mycore/utils/resources.h"

void myencoding_string_append(mycore_string_t* str, const char* buff, size_t length, myencoding_t encoding)
{
    myencoding_result_t res;
    myencoding_result_clean(&res);
    
    myencoding_string_append_chunk(str, &res, buff, length, encoding);
}

void myencoding_string_append_chunk(mycore_string_t* str, myencoding_result_t* res, const char* buff, size_t length, myencoding_t encoding)
{
    unsigned const char* u_buff = (unsigned const char*)buff;
    const myencoding_custom_f func = myencoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyENCODING_STATUS_OK) {
            MyCORE_STRING_REALLOC_IF_NEED(str, 5, 0);
            str->length += myencoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
        }
    }
    
    MyCORE_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
}

void myencoding_string_append_one(mycore_string_t* str, myencoding_result_t* res, const char data, myencoding_t encoding)
{
    const myencoding_custom_f func = myencoding_get_function_by_id(encoding);
    
    if(func((unsigned const char)data, res) == MyENCODING_STATUS_OK) {
        MyCORE_STRING_REALLOC_IF_NEED(str, 5, 0);
        str->length += myencoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
        
        MyCORE_STRING_APPEND_BYTE_WITHOUT_INCREMENT_REALLOC('\0', str);
    }
}

void myencoding_string_append_lowercase_ascii(mycore_string_t* str, const char* buff, size_t length, myencoding_t encoding)
{
    myencoding_result_t res;
    myencoding_result_clean(&res);
    
    myencoding_string_append_chunk_lowercase_ascii(str, &res, buff, length, encoding);
}

void myencoding_string_append_chunk_lowercase_ascii(mycore_string_t* str, myencoding_result_t* res, const char* buff, size_t length, myencoding_t encoding)
{
    unsigned const char* u_buff = (unsigned const char*)buff;
    const myencoding_custom_f func = myencoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyENCODING_STATUS_OK) {
            MyCORE_STRING_REALLOC_IF_NEED(str, 5, 1);
            
            size_t insert_len = myencoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
            
            if(insert_len == 1) {
                str->data[str->length] = (const char)mycore_string_chars_lowercase_map[ u_buff[i] ];
            }
            
            str->length += insert_len;
        }
    }
    
    MyCORE_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
}

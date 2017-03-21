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

#include "myhtml/mystring.h"
#include "mycore/utils/resources.h"

size_t myhtml_string_append_with_preprocessing(mycore_string_t* str, const char* buff, size_t length, bool emit_null_chars)
{
    MyCORE_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned char *data = (unsigned char*)str->data;
    const unsigned char *u_buff = (const unsigned char*)buff;
    
    /* 0x0D == \r */
    /* 0x0A == \n */
    
    for (size_t i = 0; i < length; i++)
    {
        if(u_buff[i] == 0x0D) {
            data[str->length] = 0x0A;
            
            if((i + 1) < length) {
                if(u_buff[(i + 1)] == 0x0A)
                    i++;
            }
            else {
                str->length++;
                
                MyCORE_STRING_REALLOC_IF_NEED(str, 0, 2);
                str->data[str->length] = '\0';
                
                return str->length;
            }
        }
        else if(u_buff[i] == 0x00 && emit_null_chars == false)
        {
            mycore_string_realloc(str, (str->size + 5));
            data = (unsigned char*)str->data;
            
            // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
            data[str->length] = 0xEF; str->length++;
            data[str->length] = 0xBF; str->length++;
            data[str->length] = 0xBD;
        }
        else
            data[str->length] = u_buff[i];
        
        str->length++;
    }
    
    str->data[str->length] = '\0';
    
    return 0;
}

size_t myhtml_string_append_lowercase_with_preprocessing(mycore_string_t* str, const char* buff, size_t length, bool emit_null_chars)
{
    MyCORE_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned char *data = (unsigned char*)str->data;
    const unsigned char *u_buff = (const unsigned char*)buff;
    
    for (size_t i = 0; i < length; i++)
    {
        if(u_buff[i] == 0x0D) {
            data[str->length] = 0x0A;
            
            if((i + 1) < length) {
                if(u_buff[(i + 1)] == 0x0A)
                    i++;
            }
            else {
                str->length++;
                
                MyCORE_STRING_REALLOC_IF_NEED(str, 0, 2);
                str->data[str->length] = '\0';
                
                return str->length;
            }
        }
        else if(u_buff[i] == 0x00 && emit_null_chars == false)
        {
            mycore_string_realloc(str, (str->size + 5));
            data = (unsigned char*)str->data;
            
            // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
            data[str->length] = 0xEF; str->length++;
            data[str->length] = 0xBF; str->length++;
            data[str->length] = 0xBD;
        }
        else {
            data[str->length] = mycore_string_chars_lowercase_map[ u_buff[i] ];
        }
        
        str->length++;
    }
    
    str->data[str->length] = '\0';
    
    return 0;
}

size_t myhtml_string_before_append_any_preprocessing(mycore_string_t* str, const char* buff, size_t length, size_t last_position)
{
    if(last_position == 0 || length == 0)
        return 0;
    
    if(*buff == '\n')
        return 1;
    
    return 0;
}

/////////////////////////////////////////////////////////
//// Append With Convert Encoding and Preprocessing API
////
/////////////////////////////////////////////////////////
size_t myhtml_string_append_with_convert_encoding_with_preprocessing(mycore_string_t* str, const char* buff, size_t length, myencoding_t encoding, bool emit_null_chars)
{
    myencoding_result_t res;
    myencoding_result_clean(&res);
    
    return myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &res, buff, length, encoding, emit_null_chars);
}

size_t myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(mycore_string_t* str, myencoding_result_t* res, const char* buff, size_t length, myencoding_t encoding, bool emit_null_chars)
{
    MyCORE_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned const char* u_buff = (unsigned const char*)buff;
    const myencoding_custom_f func = myencoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyENCODING_STATUS_OK) {
            MyCORE_STRING_REALLOC_IF_NEED(str, 5, 1);
            
            size_t len = myencoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
            
            if(len == 1) {
                if(str->data[str->length] == '\r') {
                    str->data[str->length] = '\n';
                    
                    if((i + 1) < length) {
                        if(buff[(i + 1)] == '\n')
                            i++;
                    }
                    else {
                        str->length++;
                        
                        MyCORE_STRING_REALLOC_IF_NEED(str, 0, 2);
                        str->data[str->length] = '\0';
                        
                        return str->length;
                    }
                }
                else if(str->data[str->length] == 0x00 && emit_null_chars == false)
                {
                    mycore_string_realloc(str, (str->size + 5));
                    
                    // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
                    str->data[str->length] = (char)0xEF; str->length++;
                    str->data[str->length] = (char)0xBF; str->length++;
                    str->data[str->length] = (char)0xBD;
                }
            }
            
            str->length += len;
        }
    }
    
    MyCORE_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
    
    return 0;
}

size_t myhtml_string_append_lowercase_with_convert_encoding_with_preprocessing(mycore_string_t* str, const char* buff, size_t length, myencoding_t encoding, bool emit_null_chars)
{
    myencoding_result_t res;
    myencoding_result_clean(&res);
    
    return myhtml_string_append_lowercase_chunk_with_convert_encoding_with_preprocessing(str, &res, buff, length, encoding, emit_null_chars);
}

size_t myhtml_string_append_lowercase_chunk_with_convert_encoding_with_preprocessing(mycore_string_t* str, myencoding_result_t* res, const char* buff, size_t length, myencoding_t encoding, bool emit_null_chars)
{
    MyCORE_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned const char* u_buff = (unsigned const char*)buff;
    const myencoding_custom_f func = myencoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyENCODING_STATUS_OK) {
            MyCORE_STRING_REALLOC_IF_NEED(str, 5, 1);
            
            size_t len = myencoding_codepoint_to_lowercase_ascii_utf_8(res->result, &str->data[str->length]);
            
            if(len == 1) {
                if(str->data[str->length] == '\r') {
                    str->data[str->length] = '\n';
                    
                    if((i + 1) < length) {
                        if(buff[(i + 1)] == '\n')
                            i++;
                    }
                    else {
                        str->length++;
                        
                        MyCORE_STRING_REALLOC_IF_NEED(str, 0, 2);
                        str->data[str->length] = '\0';
                        
                        return str->length;
                    }
                }
                else if(str->data[str->length] == 0x00 && emit_null_chars == false)
                {
                    mycore_string_realloc(str, (str->size + 5));
                    
                    // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
                    str->data[str->length] = (char)0xEF; str->length++;
                    str->data[str->length] = (char)0xBF; str->length++;
                    str->data[str->length] = (char)0xBD;
                }
            }
            
            str->length += len;
        }
    }
    
    MyCORE_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
    
    return 0;
}

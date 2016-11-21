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

#include "myhtml/mystring.h"
#include "myhtml/utils/resources.h"

/////////////////////////////////////////////////////////
//// Init, Clean, Destroy
////
/////////////////////////////////////////////////////////
char * myhtml_string_init(mchar_async_t *mchar, size_t node_idx, myhtml_string_t* str, size_t size)
{
    str->data     = mchar_async_malloc(mchar, node_idx, size);
    str->size     = size;
    str->node_idx = node_idx;
    str->mchar    = mchar;
    
    myhtml_string_clean(str);
    
    return str->data;
}

void myhtml_string_clean(myhtml_string_t* str)
{
    str->length = 0;
}

void myhtml_string_clean_all(myhtml_string_t* str)
{
    memset(str, 0, sizeof(myhtml_string_t));
}

myhtml_string_t * myhtml_string_destroy(myhtml_string_t* str, bool destroy_obj)
{
    if(str == NULL)
        return NULL;
    
    if(str->data && str->mchar)
        mchar_async_free(str->mchar, str->node_idx, str->data);
    
    if(destroy_obj && str) {
        myhtml_free(str);
        return NULL;
    }
    
    return str;
}

void myhtml_string_raw_clean(myhtml_string_raw_t* str_raw)
{
    str_raw->length = 0;
}

void myhtml_string_raw_clean_all(myhtml_string_raw_t* str_raw)
{
    memset(str_raw, 0, sizeof(myhtml_string_raw_t));
}

myhtml_string_raw_t * myhtml_string_raw_destroy(myhtml_string_raw_t* str_raw, bool destroy_obj)
{
    if(str_raw == NULL)
        return NULL;
    
    if(str_raw->data) {
        myhtml_free(str_raw->data);
        str_raw->data = NULL;
    }
    
    if(destroy_obj && str_raw) {
        myhtml_free(str_raw);
        return NULL;
    }
    
    return str_raw;
}

char * myhtml_string_realloc(myhtml_string_t *str, size_t new_size)
{
    if(str == NULL)
        return NULL;
    
    char *tmp = mchar_async_realloc(str->mchar, str->node_idx, str->data, str->length, new_size);
    
    if(tmp) {
        str->size = new_size;
        str->data = tmp;
    }
    else
        return NULL;
    
    return tmp;
}

/////////////////////////////////////////////////////////
//// Basic API
////
/////////////////////////////////////////////////////////
char * myhtml_string_data_alloc(mchar_async_t *mchar, size_t node_id, size_t size)
{
    return mchar_async_malloc(mchar, node_id, size);
}

char * myhtml_string_data_realloc(mchar_async_t *mchar, size_t node_id, char *data,  size_t len_to_copy, size_t size)
{
    return mchar_async_realloc(mchar, node_id, data, len_to_copy, size);
}

void myhtml_string_data_free(mchar_async_t *mchar, size_t node_id, char *data)
{
    mchar_async_free(mchar, node_id, data);
}

char * myhtml_string_data(myhtml_string_t *str)
{
    if(str == NULL)
        return NULL;
    
    return str->data;
}

size_t myhtml_string_length(myhtml_string_t *str)
{
    if(str == NULL)
        return 0;
    
    return str->length;
}

size_t myhtml_string_size(myhtml_string_t *str)
{
    if(str == NULL)
        return 0;
    
    return str->size;
}

char * myhtml_string_data_set(myhtml_string_t *str, char *data)
{
    if(str == NULL)
        return NULL;
    
    str->data = data;
    return str->data;
}

size_t myhtml_string_size_set(myhtml_string_t *str, size_t size)
{
    if(str == NULL)
        return 0;
    
    str->size = size;
    return str->size;
}

size_t myhtml_string_length_set(myhtml_string_t *str, size_t length)
{
    if(str == NULL)
        return 0;
    
    str->length = length;
    return str->length;
}

/////////////////////////////////////////////////////////
//// Append API
////
/////////////////////////////////////////////////////////
void myhtml_string_append(myhtml_string_t* str, const char* buff, size_t length)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    memcpy(&str->data[str->length], buff, (sizeof(char) * length));
    
    str->length += length;
    str->data[str->length] = '\0';
}

void myhtml_string_append_one(myhtml_string_t* str, const char data)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, 2, 1);
    MyHTML_STRING_APPEND_BYTE_WITHOUT_REALLOC(data, str);
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT_REALLOC('\0', str);
}

void myhtml_string_append_lowercase(myhtml_string_t* str, const char* data, size_t length)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned char *ref = (unsigned char*)&str->data[str->length];
    const unsigned char *buf = (const unsigned char*)data;
    
    size_t i;
    for(i = 0; i < length; i++) {
        ref[i] = myhtml_string_chars_lowercase_map[ buf[i] ];
    }
    
    ref[i] = '\0';
    str->length += length;
}

size_t myhtml_string_append_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, bool emit_null_chars)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
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
                return str->length;
            }
        }
        else if(u_buff[i] == 0x00 && emit_null_chars == false)
        {
            myhtml_string_realloc(str, (str->size + 5));
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

size_t myhtml_string_append_lowercase_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, bool emit_null_chars)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
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
                return str->length;
            }
        }
        else if(u_buff[i] == 0x00 && emit_null_chars == false)
        {
            myhtml_string_realloc(str, (str->size + 5));
            data = (unsigned char*)str->data;
            
            // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
            data[str->length] = 0xEF; str->length++;
            data[str->length] = 0xBF; str->length++;
            data[str->length] = 0xBD;
        }
        else {
            data[str->length] = myhtml_string_chars_lowercase_map[ u_buff[i] ];
        }
        
        str->length++;
    }
    
    str->data[str->length] = '\0';
    
    return 0;
}

size_t myhtml_string_before_append_any_preprocessing(myhtml_string_t* str, const char* buff, size_t length, size_t last_position)
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
size_t myhtml_string_append_with_convert_encoding_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding, bool emit_null_chars)
{
    myhtml_encoding_result_t res;
    myhtml_encoding_result_clean(&res);
    
    return myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &res, buff, length, encoding, emit_null_chars);
}

size_t myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding, bool emit_null_chars)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned const char* u_buff = (unsigned const char*)buff;
    const myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyHTML_ENCODING_STATUS_OK) {
            MyHTML_STRING_REALLOC_IF_NEED(str, 5, 1);
            
            size_t len = myhtml_encoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
            
            if(len == 1) {
                if(str->data[str->length] == '\r') {
                    str->data[str->length] = '\n';
                    
                    if((i + 1) < length) {
                        if(buff[(i + 1)] == '\n')
                            i++;
                    }
                    else {
                        str->length++;
                        return str->length;
                    }
                }
                else if(str->data[str->length] == 0x00 && emit_null_chars == false)
                {
                    myhtml_string_realloc(str, (str->size + 5));
                    
                    // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
                    str->data[str->length] = (char)0xEF; str->length++;
                    str->data[str->length] = (char)0xBF; str->length++;
                    str->data[str->length] = (char)0xBD;
                }
            }
            
            str->length += len;
        }
    }
    
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
    
    return 0;
}

size_t myhtml_string_append_lowercase_with_convert_encoding_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding, bool emit_null_chars)
{
    myhtml_encoding_result_t res;
    myhtml_encoding_result_clean(&res);
    
    return myhtml_string_append_lowercase_chunk_with_convert_encoding_with_preprocessing(str, &res, buff, length, encoding, emit_null_chars);
}

size_t myhtml_string_append_lowercase_chunk_with_convert_encoding_with_preprocessing(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding, bool emit_null_chars)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned const char* u_buff = (unsigned const char*)buff;
    const myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyHTML_ENCODING_STATUS_OK) {
            MyHTML_STRING_REALLOC_IF_NEED(str, 5, 1);
            
            size_t len = myhtml_encoding_codepoint_to_lowercase_ascii_utf_8(res->result, &str->data[str->length]);
            
            if(len == 1) {
                if(str->data[str->length] == '\r') {
                    str->data[str->length] = '\n';
                    
                    if((i + 1) < length) {
                        if(buff[(i + 1)] == '\n')
                            i++;
                    }
                    else {
                        str->length++;
                        return str->length;
                    }
                }
                else if(str->data[str->length] == 0x00 && emit_null_chars == false)
                {
                    myhtml_string_realloc(str, (str->size + 5));
                    
                    // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
                    str->data[str->length] = (char)0xEF; str->length++;
                    str->data[str->length] = (char)0xBF; str->length++;
                    str->data[str->length] = (char)0xBD;
                }
            }
            
            str->length += len;
        }
    }
    
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
    
    return 0;
}

/////////////////////////////////////////////////////////
//// Append With Convert Encoding without Preprocessing API
////
/////////////////////////////////////////////////////////
void myhtml_string_append_with_convert_encoding(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding)
{
    myhtml_encoding_result_t res;
    myhtml_encoding_result_clean(&res);
    
    myhtml_string_append_chunk_with_convert_encoding(str, &res, buff, length, encoding);
}

void myhtml_string_append_chunk_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding)
{
    unsigned const char* u_buff = (unsigned const char*)buff;
    const myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyHTML_ENCODING_STATUS_OK) {
            MyHTML_STRING_REALLOC_IF_NEED(str, 5, 0);
            str->length += myhtml_encoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
        }
    }
    
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
}

void myhtml_string_append_one_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char data, myhtml_encoding_t encoding)
{
    const myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(encoding);
    
    if(func((unsigned const char)data, res) == MyHTML_ENCODING_STATUS_OK) {
        MyHTML_STRING_REALLOC_IF_NEED(str, 5, 0);
        str->length += myhtml_encoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
        
        MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT_REALLOC('\0', str);
    }
}


void myhtml_string_append_lowercase_ascii_with_convert_encoding(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding)
{
    myhtml_encoding_result_t res;
    myhtml_encoding_result_clean(&res);
    
    myhtml_string_append_chunk_lowercase_ascii_with_convert_encoding(str, &res, buff, length, encoding);
}

void myhtml_string_append_chunk_lowercase_ascii_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding)
{
    unsigned const char* u_buff = (unsigned const char*)buff;
    const myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyHTML_ENCODING_STATUS_OK) {
            MyHTML_STRING_REALLOC_IF_NEED(str, 5, 1);
            
            size_t insert_len = myhtml_encoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
            
            if(insert_len == 1) {
                str->data[str->length] = (const char)myhtml_string_chars_lowercase_map[ u_buff[i] ];
            }
            
            str->length += insert_len;
        }
    }
    
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
}

void myhtml_string_copy(myhtml_string_t* dest, myhtml_string_t* target)
{
    myhtml_string_append(dest, target->data, target->length);
}

size_t myhtml_string_raw_copy(char* str1, const char* str2, size_t size)
{
    str1[size] = '\0';
    
    while(size) {
        size--;
        str1[size] = str2[size];
    }
    
    return size;
}

size_t myhtml_string_raw_set_replacement_character(myhtml_string_t* target, size_t position)
{
    // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
    target->data[(position)]     = (char)0xEF;
    target->data[(position + 1)] = (char)0xBF;
    target->data[(position + 2)] = (char)0xBD;
    
    return 3;
}

void myhtml_string_append_with_replacement_null_characters_only(myhtml_string_t* str, const char* buff, size_t length)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned char *data = (unsigned char*)str->data;
    const unsigned char *u_buff = (const unsigned char*)buff;
    
    for (size_t i = 0; i < length; i++)
    {
        if(u_buff[i] == 0x00) {
            myhtml_string_realloc(str, (str->size + 5));
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
}

void myhtml_string_stay_only_whitespace(myhtml_string_t* target)
{
    char *data = target->data;
    size_t pos = 0;
    
    for(size_t i = 0; i < target->length; i++)
    {
        if(myhtml_mystring_whitespace(data[i], ==, ||)) {
            data[pos] = data[i];
            pos++;
        }
    }
    
    target->length = pos;
}

size_t myhtml_string_crop_whitespace_from_begin(myhtml_string_t* target)
{
    char *data = target->data;
    size_t i;
    
    for(i = 0; i < target->length; i++) {
        if(myhtml_mystring_whitespace(data[i], !=, &&))
            break;
    }
    
    if(i)
        target->data = mchar_async_crop_first_chars_without_cache(target->data, i);
    
    target->length -= i;
    
    return i;
}

size_t myhtml_string_whitespace_from_begin(myhtml_string_t* target)
{
    char *data = target->data;
    size_t i;
    
    for(i = 0; i < target->length; i++) {
        if(myhtml_mystring_whitespace(data[i], !=, &&))
            break;
    }
    
    return i;
}



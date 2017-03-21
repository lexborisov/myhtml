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

#include "mycore/mystring.h"
#include "mycore/utils/resources.h"

/////////////////////////////////////////////////////////
//// Init, Clean, Destroy
////
/////////////////////////////////////////////////////////
char * mycore_string_init(mchar_async_t *mchar, size_t node_idx, mycore_string_t* str, size_t size)
{
    str->data     = mchar_async_malloc(mchar, node_idx, size);
    str->size     = size;
    str->node_idx = node_idx;
    str->mchar    = mchar;
    
    mycore_string_clean(str);
    
    return str->data;
}

void mycore_string_clean(mycore_string_t* str)
{
    str->length = 0;
}

void mycore_string_clean_all(mycore_string_t* str)
{
    memset(str, 0, sizeof(mycore_string_t));
}

mycore_string_t * mycore_string_destroy(mycore_string_t* str, bool destroy_obj)
{
    if(str == NULL)
        return NULL;
    
    if(str->data && str->mchar)
        mchar_async_free(str->mchar, str->node_idx, str->data);
    
    if(destroy_obj && str) {
        mycore_free(str);
        return NULL;
    }
    
    return str;
}

void mycore_string_raw_clean(mycore_string_raw_t* str_raw)
{
    str_raw->length = 0;
}

void mycore_string_raw_clean_all(mycore_string_raw_t* str_raw)
{
    memset(str_raw, 0, sizeof(mycore_string_raw_t));
}

mycore_string_raw_t * mycore_string_raw_destroy(mycore_string_raw_t* str_raw, bool destroy_obj)
{
    if(str_raw == NULL)
        return NULL;
    
    if(str_raw->data) {
        mycore_free(str_raw->data);
        str_raw->data = NULL;
    }
    
    if(destroy_obj && str_raw) {
        mycore_free(str_raw);
        return NULL;
    }
    
    return str_raw;
}

char * mycore_string_realloc(mycore_string_t *str, size_t new_size)
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
char * mycore_string_data_alloc(mchar_async_t *mchar, size_t node_id, size_t size)
{
    return mchar_async_malloc(mchar, node_id, size);
}

char * mycore_string_data_realloc(mchar_async_t *mchar, size_t node_id, char *data,  size_t len_to_copy, size_t size)
{
    return mchar_async_realloc(mchar, node_id, data, len_to_copy, size);
}

void mycore_string_data_free(mchar_async_t *mchar, size_t node_id, char *data)
{
    mchar_async_free(mchar, node_id, data);
}

char * mycore_string_data(mycore_string_t *str)
{
    if(str == NULL)
        return NULL;
    
    return str->data;
}

size_t mycore_string_length(mycore_string_t *str)
{
    if(str == NULL)
        return 0;
    
    return str->length;
}

size_t mycore_string_size(mycore_string_t *str)
{
    if(str == NULL)
        return 0;
    
    return str->size;
}

char * mycore_string_data_set(mycore_string_t *str, char *data)
{
    if(str == NULL)
        return NULL;
    
    str->data = data;
    return str->data;
}

size_t mycore_string_size_set(mycore_string_t *str, size_t size)
{
    if(str == NULL)
        return 0;
    
    str->size = size;
    return str->size;
}

size_t mycore_string_length_set(mycore_string_t *str, size_t length)
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
void mycore_string_append(mycore_string_t* str, const char* buff, size_t length)
{
    MyCORE_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    memcpy(&str->data[str->length], buff, (sizeof(char) * length));
    
    str->length += length;
    str->data[str->length] = '\0';
}

void mycore_string_append_one(mycore_string_t* str, const char data)
{
    MyCORE_STRING_REALLOC_IF_NEED(str, 2, 1);
    MyCORE_STRING_APPEND_BYTE_WITHOUT_REALLOC(data, str);
    MyCORE_STRING_APPEND_BYTE_WITHOUT_INCREMENT_REALLOC('\0', str);
}

void mycore_string_append_lowercase(mycore_string_t* str, const char* data, size_t length)
{
    MyCORE_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned char *ref = (unsigned char*)&str->data[str->length];
    const unsigned char *buf = (const unsigned char*)data;
    
    size_t i;
    for(i = 0; i < length; i++) {
        ref[i] = mycore_string_chars_lowercase_map[ buf[i] ];
    }
    
    ref[i] = '\0';
    str->length += length;
}

void mycore_string_copy(mycore_string_t* dest, mycore_string_t* target)
{
    mycore_string_append(dest, target->data, target->length);
}

size_t mycore_string_raw_copy(char* str1, const char* str2, size_t size)
{
    str1[size] = '\0';
    
    while(size) {
        size--;
        str1[size] = str2[size];
    }
    
    return size;
}

size_t mycore_string_raw_set_replacement_character(mycore_string_t* target, size_t position)
{
    // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
    target->data[(position)]     = (char)0xEF;
    target->data[(position + 1)] = (char)0xBF;
    target->data[(position + 2)] = (char)0xBD;
    
    return 3;
}

void mycore_string_append_with_replacement_null_characters(mycore_string_t* str, const char* buff, size_t length)
{
    MyCORE_STRING_REALLOC_IF_NEED(str, (length + 1), 0);
    
    unsigned char *data = (unsigned char*)str->data;
    const unsigned char *u_buff = (const unsigned char*)buff;
    
    for (size_t i = 0; i < length; i++)
    {
        if(u_buff[i] == 0x00) {
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
}

void mycore_string_stay_only_whitespace(mycore_string_t* target)
{
    char *data = target->data;
    size_t pos = 0;
    
    for(size_t i = 0; i < target->length; i++)
    {
        if(mycore_utils_whithspace(data[i], ==, ||)) {
            data[pos] = data[i];
            pos++;
        }
    }
    
    target->length = pos;
}

size_t mycore_string_crop_whitespace_from_begin(mycore_string_t* target)
{
    char *data = target->data;
    size_t i;
    
    for(i = 0; i < target->length; i++) {
        if(mycore_utils_whithspace(data[i], !=, &&))
            break;
    }
    
    if(i)
        target->data = mchar_async_crop_first_chars_without_cache(target->data, i);
    
    target->length -= i;
    
    return i;
}

size_t mycore_string_whitespace_from_begin(mycore_string_t* target)
{
    char *data = target->data;
    size_t i;
    
    for(i = 0; i < target->length; i++) {
        if(mycore_utils_whithspace(data[i], !=, &&))
            break;
    }
    
    return i;
}



/*
 Copyright 2015-2016 Alexander Borisov
 
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
#include "myhtml/mystring.h"


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
    
    if(str->data && str->mchar && str->node_idx)
        mchar_async_free(str->mchar, str->node_idx, str->data);
    
    if(destroy_obj && str)
        free(str);
    
    return NULL;
}

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

char * myhtml_string_realloc(mchar_async_t *mchar, size_t node_id, myhtml_string_t *str, size_t new_size)
{
    if(str == NULL)
        return 0;
    
    char *tmp;
    
    if(str->data) {
        tmp = mchar_async_realloc(str->mchar, str->node_idx, str->data, str->length, new_size);
    }
    else {
        tmp = mchar_async_malloc(mchar, node_id, new_size);
        
        str->mchar    = mchar;
        str->node_idx = node_id;
    }
    
    if(tmp) {
        str->size = new_size;
        str->data = tmp;
    }
    else
        return NULL;
    
    return str->data;
}

bool myhtml_string_release(myhtml_string_t* str, size_t size)
{
    char* tmp = mchar_async_realloc(str->mchar, str->node_idx, str->data, str->length, size);
    
    if(tmp) {
        str->size = size;
        str->data = tmp;
    }
    else
        return false;
    
    return true;
}

void _myhtml_string_charef_append(myhtml_string_t* str, const char sm)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, 2, 32);
    
    str->data[str->length] = sm;
    str->length++;
    
    str->data[str->length] = '\0';
}

size_t _myhtml_string_append_char_references_state_0(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    size_t tmp_offset = offset;
    
    while(offset < size)
    {
        if(buff[offset] == '&')
        {
            if(chunk->encoding == MyHTML_ENCODING_UTF_8) {
                myhtml_string_append_with_preprocessing(str, &buff[tmp_offset], (offset - tmp_offset));
            }
            else {
                myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &chunk->res, &buff[tmp_offset], (offset - tmp_offset), chunk->encoding);
                myhtml_encoding_result_clean(&chunk->res);
            }
            
            chunk->begin = str->length;
            chunk->state = 1;
            
            _myhtml_string_charef_append(str, buff[offset]);
            
            offset++;
            return offset;
        }
        
        offset++;
    }
    
    if(chunk->encoding == MyHTML_ENCODING_UTF_8)
        myhtml_string_append_with_preprocessing(str, &buff[tmp_offset], (offset - tmp_offset));
    else
        myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &chunk->res, &buff[tmp_offset], (offset - tmp_offset), chunk->encoding);
    
    return offset;
}

size_t _myhtml_string_append_char_references_state_1(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    if(buff[offset] == '#')
    {
        _myhtml_string_charef_append(str, buff[offset]);
        offset++;
        
        chunk->l_data = 0;
        
        if(offset >= size) {
            chunk->state = 3;
            return offset;
        }
        
        if(buff[offset] == 'x' || buff[offset] == 'X') {
            _myhtml_string_charef_append(str, buff[offset]);
            offset++;
            
            chunk->state = 5;
        }
        else
            chunk->state = 4;
    }
    else {
        chunk->charef_res.last_entry = NULL;
        chunk->charef_res.curr_entry = myhtml_charef_get_first_position(buff[offset]);
        
        if(chunk->charef_res.curr_entry->ch == '\0')
            chunk->state = 0;
        else {
            chunk->state = 2;
            
            _myhtml_string_charef_append(str, buff[offset]);
            offset++;
        }
    }
    
    return offset;
}

size_t _myhtml_string_append_char_references_state_2(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    size_t tmp_offset = offset;
    
    const charef_entry_t *current_entry = myhtml_charef_find_by_pos(chunk->charef_res.curr_entry->next, buff, &offset, size, &chunk->charef_res);
    
    if(chunk->charef_res.is_done) {
        chunk->state = 0;
        
        if(buff[offset] == ';')
            offset++;
        else {
            if(chunk->is_attributes &&
               (buff[offset] == '=' || myhtml_string_alphanumeric_character[ (unsigned char)buff[offset] ] != 0xff))
            {
                if(chunk->encoding == MyHTML_ENCODING_UTF_8)
                    myhtml_string_append_with_preprocessing(str, &buff[tmp_offset], (offset - tmp_offset));
                else
                    myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &chunk->res, &buff[tmp_offset], (offset - tmp_offset), chunk->encoding);
                
                return offset;
            }
        }
        
        if(current_entry->codepoints_len)
        {
            for (size_t i = 0; i < current_entry->codepoints_len; i++) {
                MyHTML_STRING_REALLOC_IF_NEED(str, 4, 32);
                
                chunk->begin += myhtml_encoding_codepoint_to_ascii_utf_8(current_entry->codepoints[i], &str->data[chunk->begin]);
            }
            
            str->length = chunk->begin;
        }
        else {
            if(chunk->encoding == MyHTML_ENCODING_UTF_8)
                myhtml_string_append_with_preprocessing(str, &buff[tmp_offset], (offset - tmp_offset));
            else
                myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &chunk->res, &buff[tmp_offset], (offset - tmp_offset), chunk->encoding);
        }
        
        chunk->charef_res.last_entry = NULL;
    }
    else {
        if(chunk->encoding == MyHTML_ENCODING_UTF_8)
            myhtml_string_append_with_preprocessing(str, &buff[tmp_offset], (offset - tmp_offset));
        else
            myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &chunk->res, &buff[tmp_offset], (offset - tmp_offset), chunk->encoding);
    }
    
    return offset;
}

size_t _myhtml_string_append_char_references_state_3(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    if(buff[offset] == 'x' || buff[offset] == 'X') {
        _myhtml_string_charef_append(str, buff[offset]);
        offset++;
        
        chunk->state = 5;
    }
    else
        chunk->state = 4;
    
    return offset;
}

void _myhtml_string_append_char_references_state_end(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str)
{
    str->length = chunk->begin;
    
    /* 4 is max utf8 byte + \0 */
    MyHTML_STRING_REALLOC_IF_NEED(str, 5, 12);
    
    if(chunk->l_data <= 0x9F)
        chunk->l_data = replacement_character[chunk->l_data];
    else if(chunk->l_data >= 0xD800 && chunk->l_data <= 0xDFFF)
        chunk->l_data = replacement_character[0];
    else if(chunk->l_data > 0x10FFFF)
        chunk->l_data = replacement_character[0];
    
//    if(chunk->l_data == 0x10FFFE)
//        chunk->l_data = replacement_character[0];
    
    str->length += myhtml_encoding_codepoint_to_ascii_utf_8(chunk->l_data, &str->data[chunk->begin]);
    str->data[str->length] = '\0';
}

size_t _myhtml_string_append_char_references_state_4(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const unsigned char *buff, size_t offset, size_t size)
{
    size_t tmp_offset = offset;
    
    while(offset < size)
    {
        if(myhtml_string_chars_num_map[ buff[offset] ] == 0xff)
        {
            chunk->state = 0;
            
            myhtml_string_append_with_preprocessing(str, (const char*)&buff[tmp_offset], (offset - tmp_offset));
            
            if((str->length - (chunk->begin + 2)) == 0) {
                return offset;
            }
            
            if(buff[offset] == ';')
                offset++;
            
            _myhtml_string_append_char_references_state_end(chunk, str);
            return offset;
        }
        
        if(chunk->l_data <= 0x10FFFF) {
            chunk->l_data = myhtml_string_chars_num_map[ buff[offset] ] + chunk->l_data * 10;
        }
        
        offset++;
    }
    
    myhtml_string_append_with_preprocessing(str, (const char*)&buff[tmp_offset], (offset - tmp_offset));
    
    return offset;
}

size_t _myhtml_string_append_char_references_state_5(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    unsigned const char *u_buff = (unsigned const char*)buff;
    size_t current_len = (str->length - (chunk->begin + 3));
    
    size_t tmp_offset = offset, tmp_set_to = offset;
    
    while(offset < size)
    {
        if(myhtml_string_chars_hex_map[ u_buff[offset] ] == 0xff)
        {
            if(((offset - tmp_offset) + current_len) > 6) {
                chunk->l_data = 0xFFFFFF;
            }
            
            chunk->state = 0;
            myhtml_string_append_with_preprocessing(str, &buff[tmp_offset], (tmp_set_to - tmp_offset));
            
            if((str->length - (chunk->begin + 3)) == 0) {
                return offset;
            }
            
            if(buff[offset] == ';')
                offset++;
            
            _myhtml_string_append_char_references_state_end(chunk, str);
            return offset;
        }
        
        if(((offset - tmp_offset) + current_len) < 7) {
            chunk->l_data <<= 4;
            chunk->l_data |= myhtml_string_chars_hex_map[ u_buff[offset] ];
            
            tmp_set_to++;
        }
        
        offset++;
    }
    
    if(((offset - tmp_offset) + current_len) > 6) {
        chunk->l_data = 0xFFFFFF;
    }
    
    myhtml_string_append_with_preprocessing(str, &buff[tmp_offset], (offset - tmp_offset));
    
    return offset;
}

void myhtml_string_append_charef(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t size)
{
    size_t offset = 0;
    
    while (offset < size)
    {
        switch (chunk->state) {
            case 1:
                offset = _myhtml_string_append_char_references_state_1(chunk, str, buff, offset, size);
                break;
            case 2:
                offset = _myhtml_string_append_char_references_state_2(chunk, str, buff, offset, size);
                break;
            case 3:
                offset = _myhtml_string_append_char_references_state_3(chunk, str, buff, offset, size);
                break;
            case 4:
                offset = _myhtml_string_append_char_references_state_4(chunk, str, (const unsigned char*)buff, offset, size);
                break;
            case 5:
                offset = _myhtml_string_append_char_references_state_5(chunk, str, buff, offset, size);
                break;
            default:
                offset = _myhtml_string_append_char_references_state_0(chunk, str, buff, offset, size);
                break;
        }
    }
}

void myhtml_string_append_charef_end(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str)
{
    if(chunk->state == 2 && chunk->charef_res.last_entry)
    {
        const charef_entry_t *entry = chunk->charef_res.last_entry;
        
        for (size_t i = 0; i < entry->codepoints_len; i++) {
            MyHTML_STRING_REALLOC_IF_NEED(str, 4, 32);
            
            chunk->begin += myhtml_encoding_codepoint_to_ascii_utf_8(entry->codepoints[i], &str->data[chunk->begin]);
        }
        
        str->length = chunk->begin;
    }
    else if(chunk->state == 4) {
        if((str->length - (chunk->begin + 2)))
            _myhtml_string_append_char_references_state_end(chunk, str);
    }
    else if(chunk->state == 5) {
        if((str->length - (chunk->begin + 3)))
            _myhtml_string_append_char_references_state_end(chunk, str);
    }
    
    if(str->length) {
        if(str->data[ (str->length - 1) ] == '\r') {
            str->data[ (str->length - 1) ] = '\n';
        }
    }
}

void myhtml_string_append(myhtml_string_t* str, const char* buff, size_t length)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), (length + 32));
    
    memcpy(&str->data[str->length], buff, (sizeof(char) * length));
    
    str->length += length;
    str->data[str->length] = '\0';
}

void myhtml_string_append_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 32);
    
    unsigned char *data = (unsigned char*)str->data;
    const unsigned char *u_buff = (unsigned char*)buff;
    
    for (size_t i = 0; i < length; i++)
    {
        if(u_buff[i] == 0x0D)
        {
            data[str->length] = 0x0A;
            
            if((i+1) < length && u_buff[(i + 1)] == 0x0A) {
                i++;
            }
        }
        else if(u_buff[i] == 0x00)
        {
            MyHTML_STRING_REALLOC_IF_NEED(str, 4, 32);
            
            if((unsigned char *)str->data != data)
                data = (unsigned char*)str->data;
            
            // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
            data[str->length] = 0xEF; str->length++;
            data[str->length] = 0xBF; str->length++;
            data[str->length] = 0xBD;
        }
        else {
            data[str->length] = u_buff[i];
        }
        
        str->length++;
    }
    
    str->data[str->length] = '\0';
}

void myhtml_string_append_lowercase_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 32);
    
    unsigned char *data = (unsigned char*)str->data;
    const unsigned char *u_buff = (unsigned char*)buff;
    
    for (size_t i = 0; i < length; i++)
    {
        if(u_buff[i] == 0x0D)
        {
            data[str->length] = 0x0A;
            
            if((i+1) < length && u_buff[(i + 1)] == 0x0A) {
                i++;
            }
        }
        else if(u_buff[i] == 0x00)
        {
            MyHTML_STRING_REALLOC_IF_NEED(str, 4, 32);
            
            if((unsigned char *)str->data != data)
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
}

void myhtml_string_append_with_convert_encoding_with_preprocessing(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding)
{
    myhtml_encoding_result_t res;
    myhtml_encoding_result_clean(&res);
    
    myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &res, buff, length, encoding);
}

void myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding)
{
    unsigned const char* u_buff = (unsigned const char*)buff;
    myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyHTML_ENCODING_STATUS_OK) {
            MyHTML_STRING_REALLOC_IF_NEED(str, 4, 32);
            
            size_t len = myhtml_encoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
            
            if(len == 1) {
                // change \r\n to \n
                if(str->length > 0) {
                    if(str->data[(str->length - 1)] == '\r') {
                        str->data[(str->length - 1)] = '\n';
                        
                        if(str->data[str->length] == '\n')
                            str->length--;
                        
                        str->length += len;
                        continue;
                    }
                }
                
                if(str->data[str->length] == 0x00)
                {
                    MyHTML_STRING_REALLOC_IF_NEED(str, 4, 32);
                    
                    // Unicode Character 'REPLACEMENT CHARACTER' (U+FFFD)
                    str->data[str->length] = 0xEF; str->length++;
                    str->data[str->length] = 0xBF; str->length++;
                    str->data[str->length] = 0xBD;
                }
            }
            
            str->length += len;
        }
    }
    
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
}

void myhtml_string_append_with_convert_encoding(myhtml_string_t* str, const char* buff, size_t length, myhtml_encoding_t encoding)
{
    myhtml_encoding_result_t res;
    myhtml_encoding_result_clean(&res);
    
    myhtml_string_append_chunk_with_convert_encoding(str, &res, buff, length, encoding);
}

void myhtml_string_append_chunk_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char* buff, size_t length, myhtml_encoding_t encoding)
{
    unsigned const char* u_buff = (unsigned const char*)buff;
    myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyHTML_ENCODING_STATUS_OK) {
            MyHTML_STRING_REALLOC_IF_NEED(str, 4, 32);
            str->length += myhtml_encoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
        }
    }
    
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT('\0', str, 1);
}

void myhtml_string_append_one(myhtml_string_t* str, const char data)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, 2, 32);
    MyHTML_STRING_APPEND_BYTE_WITHOUT_REALLOC(data, str);
    MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT_REALLOC('\0', str);
}

void myhtml_string_append_one_with_convert_encoding(myhtml_string_t* str, myhtml_encoding_result_t* res, const char data, myhtml_encoding_t encoding)
{
    myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(encoding);
    
    if(func((unsigned const char)data, res) == MyHTML_ENCODING_STATUS_OK) {
        MyHTML_STRING_REALLOC_IF_NEED(str, 5, 32);
        str->length += myhtml_encoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
        
        MyHTML_STRING_APPEND_BYTE_WITHOUT_INCREMENT_REALLOC('\0', str);
    }
}

void myhtml_string_append_lowercase(myhtml_string_t* str, const char* data, size_t length)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), 32);
    
    unsigned char *ref = (unsigned char*)&str->data[str->length];
    unsigned char *buf = (unsigned char*)data;
    
    size_t i;
    for(i = 0; i < length; i++) {
        ref[i] = myhtml_string_chars_lowercase_map[ buf[i] ];
    }
    
    ref[i] = '\0';
    str->length += length;
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
    myhtml_encoding_custom_f func = myhtml_encoding_get_function_by_id(encoding);
    
    for (size_t i = 0; i < length; i++)
    {
        if(func(u_buff[i], res) == MyHTML_ENCODING_STATUS_OK) {
            MyHTML_STRING_REALLOC_IF_NEED(str, 4, 32);
            
            size_t insert_len = myhtml_encoding_codepoint_to_ascii_utf_8(res->result, &str->data[str->length]);
            
            if(insert_len == 1) {
                str->data[str->length] = myhtml_string_chars_lowercase_map[ u_buff[i] ];
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
    target->data[(position)]     = 0xEF;
    target->data[(position + 1)] = 0xBF;
    target->data[(position + 2)] = 0xBD;
    
    return 3;
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



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

#include "mystring.h"

static const unsigned char myhtml_string_chars_num_map[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff
};

static const unsigned char myhtml_string_chars_hex_map[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff
};

void myhtml_string_init(myhtml_string_t* str, mchar_async_t *mchar, size_t node_idx, size_t size)
{
    str->data     = mchar_async_malloc(mchar, node_idx, size);
    str->size     = size;
    str->node_idx = node_idx;
    str->mchar    = mchar;
    
    myhtml_string_clean(str);
}

void myhtml_string_clean(myhtml_string_t* str)
{
    str->length = 0;
}

myhtml_string_t * myhtml_string_destroy(myhtml_string_t* str, mybool_t destroy_obj)
{
    if(str == NULL)
        return NULL;
    
    if(str->data && str->mchar && str->node_idx)
        mchar_async_free(str->mchar, str->node_idx, str->data);
    
    if(destroy_obj && str)
        free(str);
    
    return NULL;
}

mybool_t myhtml_string_release(myhtml_string_t* str, size_t size)
{
    char* tmp = mchar_async_realloc(str->mchar, str->node_idx, str->data, str->length, size);
    
    if(tmp) {
        str->size = size;
        str->data = tmp;
    }
    else
        return myfalse;
    
    return mytrue;
}

mybool_t myhtml_string_check(myhtml_string_t* str, size_t length)
{
    length = str->length + length;
    
    if(length > str->size)
    {
        char* tmp = mchar_async_realloc(str->mchar, str->node_idx, str->data, str->length, length);
        
        if(tmp) {
            str->size = length;
            str->data = tmp;
        }
        else
            return myfalse;
    }
    
    str->length = length;
    
    return mytrue;
}

void myhtml_string_append(myhtml_string_t* str, const char* buff, size_t length)
{
    size_t begin = str->length;
    myhtml_string_check(str, length + 1);
    str->length--;
    
    memcpy(&str->data[begin], buff, (sizeof(char) * length));
    str->data[str->length] = '\0';
}

size_t myhtml_string_convert_dec_to_ansi_utf8(size_t value, char *utf8)
{
    /* 0x80 -- 10xxxxxx */
    /* 0xC0 -- 110xxxxx */
    /* 0xE0 -- 1110xxxx */
    /* 0xF0 -- 11110xxx */
    /* 0xF8 -- 111110xx */
    /* 0xFC -- 1111110x */
    
    if (value <= 0x0000007F) {
        /* 0xxxxxxx */
        utf8[0] = value;
        return 1;
    }
    else if (value > 0x0000007F && value <= 0x000007FF) {
        /* 110xxxxx 10xxxxxx */
        utf8[0] = 0xC0 | (value >> 6  );
        utf8[1] = 0x80 | (value & 0x3F);
        
        return 2;
    }
    else if (value > 0x000007FF && value <= 0x0000FFFF) {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        utf8[0] = 0xE0 | ((value >> 12));
        utf8[1] = 0x80 | ((value >> 6 ) & 0x3F);
        utf8[2] = 0x80 | ( value & 0x3F);
        
        return 3;
    }
    else if (value > 0x0000FFFF && value <= 0x001FFFFF) {
        /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8[0] = 0xF0 | ( value >> 18);
        utf8[1] = 0x80 | ((value >> 12) & 0x3F);
        utf8[2] = 0x80 | ((value >> 6 ) & 0x3F);
        utf8[3] = 0x80 | ( value & 0x3F);
        
        return 4;
    }
    /* not uses in unicode */
    //    else if (value > 0x001FFFFF && value <= 0x03FFFFFF) {
    //        /* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
    //        utf8[0] = 0xF8 | ( value >> 24);
    //        utf8[1] = 0x80 | ((value >> 18) & 0x3F);
    //        utf8[2] = 0x80 | ((value >> 12) & 0x3F);
    //        utf8[3] = 0x80 | ((value >> 6 ) & 0x3F);
    //        utf8[4] = 0x80 | ( value & 0x3F);
    //
    //        return 5;
    //    }
    //    else if (value > 0x03FFFFFF && value <= 0x7FFFFFFF) {
    //        /* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
    //        utf8[0] = 0xFC | ( value >> 30);
    //        utf8[1] = 0x80 | ((value >> 24) & 0x3F);
    //        utf8[2] = 0x80 | ((value >> 18) & 0x3F);
    //        utf8[3] = 0x80 | ((value >> 12) & 0x3F);
    //        utf8[4] = 0x80 | ((value >> 6 ) & 0x3F);
    //        utf8[5] = 0x80 | ( value & 0x3F);
    //
    //        return 6;
    //    }
    
    return 0;
}

void _myhtml_string_charef_append(myhtml_string_t* str, const char sm)
{
    if((str->length + 1) >= str->size) {
        str->size += 128;
        myhtml_string_release(str, str->size);
    }
    
    str->data[str->length] = sm;
    str->length++;
    str->data[str->length] = '\0';
}

size_t _myhtml_string_append_char_references_state_0(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    while(offset < size)
    {
        if(buff[offset] == '&')
        {
            chunk->begin = str->length;
            chunk->state = 1;
            
            _myhtml_string_charef_append(str, buff[offset]);
            
            offset++;
            break;
        }
        
        _myhtml_string_charef_append(str, buff[offset]);
        offset++;
    }
    
    return offset;
}

size_t _myhtml_string_append_char_references_state_1(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    _myhtml_string_charef_append(str, buff[offset]);
    
    if(myhtml_mystring_whithspace(buff[offset], ==, ||) || buff[offset] == '<' || buff[offset] == '&') {
        chunk->state = 0;
        offset++;
    }
    else if(buff[offset] == '#')
    {
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
        chunk->entry = myhtml_charef_get_first_position(buff[offset]);
        
        if(chunk->entry->ch == '\0')
            chunk->state = 0;
        else {
            chunk->state = 2;
        }
        
        offset++;
    }
    
    return offset;
}

size_t _myhtml_string_append_char_references_state_2(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    int is_done = 0;
    size_t tmp_offset = offset;
    
    chunk->entry = myhtml_charef_find_by_pos(chunk->entry->next, buff, &offset, size, &is_done);
    
    if(is_done) {
        chunk->state = 0;
        
        if(chunk->entry->data)
        {
            size_t current_len = str->length - chunk->begin;
            
            if(current_len < chunk->entry->data_len)
            {
                size_t to_size = str->length + chunk->entry->data_len;
                
                if(to_size >= str->size) {
                    str->size = to_size + 128;
                    myhtml_string_release(str, str->size);
                }
            }
            
            memcpy(&str->data[chunk->begin], chunk->entry->data, chunk->entry->data_len);
            str->length = chunk->begin + chunk->entry->data_len;
        }
        else {
            size_t len = (offset - tmp_offset);
            if(len) {
                memcpy(&str->data[str->length], &buff[tmp_offset], len);
                str->length += len;
            }
        }
    }
    else {
        size_t len = (offset - tmp_offset);
        if(len) {
            memcpy(&str->data[str->length], &buff[tmp_offset], len);
            str->length += len;
        }
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
    
    /* 4 is max utf8 byte + null */
    if((str->length + 5) > str->size) {
        str->size += 128;
        myhtml_string_release(str, str->size);
    }
    
    str->length += myhtml_string_convert_dec_to_ansi_utf8(chunk->l_data, &str->data[str->length]);
    str->data[str->length] = '\0';
}

size_t _myhtml_string_append_char_references_state_4(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const unsigned char *buff, size_t offset, size_t size)
{
    while(offset < size)
    {
        if(myhtml_string_chars_num_map[ buff[offset] ] == 0xff)
        {
            chunk->state = 0;
            
            if((str->length - (chunk->begin + 2)) == 0) {
                _myhtml_string_charef_append(str, buff[offset]);
                offset++;
                
                return offset;
            }
            
            if(buff[offset] == ';')
                offset++;
            
            _myhtml_string_append_char_references_state_end(chunk, str);
            break;
        }
        
        chunk->l_data = myhtml_string_chars_num_map[ buff[offset] ] + chunk->l_data * 10;
        
        _myhtml_string_charef_append(str, buff[offset]);
        offset++;
    }
    
    return offset;
}

size_t _myhtml_string_append_char_references_state_5(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    unsigned const char *u_buff = (unsigned const char*)buff;
    size_t start_pos = chunk->begin + 3;
    
    while(offset < size)
    {
        if(myhtml_string_chars_hex_map[ u_buff[offset] ] == 0xff)
        {
            chunk->state = 0;
            
            if((str->length - (chunk->begin + 2)) == 0) {
                _myhtml_string_charef_append(str, buff[offset]);
                offset++;
                
                return offset;
            }
            
            if(buff[offset] == ';')
                offset++;
            
            _myhtml_string_append_char_references_state_end(chunk, str);
            break;
        }
        
        if((str->length - start_pos) < 5) {
            chunk->l_data <<= 4;
            chunk->l_data |= myhtml_string_chars_hex_map[ u_buff[offset] ];
            
            _myhtml_string_charef_append(str, buff[offset]);
        }
        
        offset++;
    }
    
    return offset;
}

void myhtml_string_append_char_references(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t size)
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

void myhtml_string_append_char_references_end(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str)
{
    if(chunk->state == 4 || chunk->state == 5) {
        _myhtml_string_append_char_references_state_end(chunk, str);
    }
}

void myhtml_string_append_with_null(myhtml_string_t* str, const char* buff, size_t length)
{
    size_t begin = str->length;
    myhtml_string_check(str, (length + 1));
    
    char* cache = &str->data[begin];
    
    memcpy(cache, buff, (sizeof(char) * length));
    cache[length] = '\0';
}

void myhtml_string_append_one_without_check(myhtml_string_t* str, const char buff)
{
    str->data[str->length] = buff;
    str->length++;
}

void myhtml_string_append_one(myhtml_string_t* str, const char buff)
{
    myhtml_string_check(str, 1);
    
    str->data[str->length] = buff;
    str->length++;
}

void myhtml_string_append_lowercase_with_null(myhtml_string_t* str, const char* buff, size_t length)
{
    size_t begin = str->length;
    
    myhtml_string_check(str, (length + 1));
    
    char* cache = &str->data[begin];
    
    size_t i;
    for(i = 0; i < length; i++) {
        cache[i] = buff[i] > 0x40 && buff[i] < 0x5b ? (buff[i]|0x60) : buff[i];
    }
    cache[i] = '\0';
}

void myhtml_string_copy(myhtml_string_t* target, myhtml_string_t* dest)
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


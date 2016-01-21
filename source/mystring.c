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
    }
    else if(buff[offset] == '#') {
        while(offset < size)
        {
            _myhtml_string_charef_append(str, buff[offset]);
            offset++;
        }
    }
    else {
        chunk->entry = myhtml_charef_get_first_position(buff[offset]);
        
        if(chunk->entry->ch == '\0')
            chunk->state = 0;
        else {
            chunk->state = 2;
            offset++;
        }
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
            default:
                offset = _myhtml_string_append_char_references_state_0(chunk, str, buff, offset, size);
                break;
        }
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


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

static const unsigned char myhtml_string_chars_lowercase_map[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
    0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
    0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
    0x3f, 0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
    0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d,
    0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
    0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1,
    0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
    0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc,
    0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce,
    0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0,
    0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2,
    0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
    0xfc, 0xfd, 0xfe, 0xff
};

static const size_t replacement_character[] = {
    65533, 1, 2, 3, 4, 5, 6, 7, 8,
    9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26,
    27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 62,
    63, 64, 65, 66, 67, 68, 69, 70, 71,
    72, 73, 74, 75, 76, 77, 78, 79, 80,
    81, 82, 83, 84, 85, 86, 87, 88, 89,
    90, 91, 92, 93, 94, 95, 96, 97, 98,
    99, 100, 101, 102, 103, 104, 105, 106, 107,
    108, 109, 110, 111, 112, 113, 114, 115, 116,
    117, 118, 119, 120, 121, 122, 123, 124, 125,
    126, 127, 8364, 129, 8218, 402, 8222, 8230, 8224,
    8225, 710, 8240, 352, 8249, 338, 141, 381, 143,
    144, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 732,
    8482, 353, 8250, 339, 157, 382, 376
};

static const size_t myhtml_string_alphanumeric_character[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0a,
    0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0a,
    0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0c, 0x0d,
    0x0e, 0x0f, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0a,
    0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x0a, 0x0b, 0x0c, 0x0d, 0xff, 0xff, 0xff,
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

void _myhtml_string_charef_append(myhtml_string_t* str, const char sm)
{
    MyHTML_STRING_REALLOC_IF_NEED(str, (str->length + 2), 32);
    
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
                MyHTML_STRING_REALLOC_IF_NEED(str, (chunk->begin + 4), 32);
                
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
    MyHTML_STRING_REALLOC_IF_NEED(str, (chunk->begin + 5), 12);
    
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
        
        chunk->l_data = myhtml_string_chars_num_map[ buff[offset] ] + chunk->l_data * 10;
        offset++;
    }
    
    myhtml_string_append_with_preprocessing(str, (const char*)&buff[tmp_offset], (offset - tmp_offset));
    
    return offset;
}

size_t _myhtml_string_append_char_references_state_5(myhtml_string_char_ref_chunk_t *chunk, myhtml_string_t* str, const char* buff, size_t offset, size_t size)
{
    unsigned const char *u_buff = (unsigned const char*)buff;
    size_t start_pos = chunk->begin + 3;
    
    size_t tmp_offset = offset, tmp_set_to = offset;
    
    while(offset < size)
    {
        if(myhtml_string_chars_hex_map[ u_buff[offset] ] == 0xff)
        {
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
        
        if((str->length - start_pos) < 6) {
            chunk->l_data <<= 4;
            chunk->l_data |= myhtml_string_chars_hex_map[ u_buff[offset] ];
            
            tmp_set_to++;
        }
        
        offset++;
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
    if(chunk->state == 4 || chunk->state == 5) {
        _myhtml_string_append_char_references_state_end(chunk, str);
    }
    else if(chunk->state == 2 && chunk->charef_res.last_entry)
    {
        const charef_entry_t *entry = chunk->charef_res.last_entry;
        
        for (size_t i = 0; i < entry->codepoints_len; i++) {
            MyHTML_STRING_REALLOC_IF_NEED(str, (chunk->begin + 4), 32);
            
            chunk->begin += myhtml_encoding_codepoint_to_ascii_utf_8(entry->codepoints[i], &str->data[chunk->begin]);
        }
        
        str->length = chunk->begin;
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
            MyHTML_STRING_REALLOC_IF_NEED(str, (length + 4), 32);
            
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
            MyHTML_STRING_REALLOC_IF_NEED(str, (length + 4), 32);
            
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
                    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 4), 32);
                    
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
    MyHTML_STRING_REALLOC_IF_NEED(str, (length + 1), (length + 32));
    
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




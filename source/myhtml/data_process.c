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

#include "myhtml/data_process.h"
#include "mycore/utils/resources.h"

#define MyHTML_DATA_PROCESS_APPEND_WITH_PREPROCESSING() \
tmp_offset += myhtml_string_before_append_any_preprocessing(str, &data[tmp_offset], (offset - tmp_offset), \
                                                            proc_entry->tmp_str_pos_proc); \
if(offset != tmp_offset) { \
    if(proc_entry->encoding == MyENCODING_UTF_8) \
        proc_entry->tmp_str_pos_proc = myhtml_string_append_with_preprocessing(str, &data[tmp_offset], (offset - tmp_offset), \
                                                                               proc_entry->emit_null_char); \
    else { \
        proc_entry->tmp_str_pos_proc = \
        myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &proc_entry->res, \
                                                                            &data[tmp_offset], (offset - tmp_offset), \
                                                                            proc_entry->encoding, proc_entry->emit_null_char); \
    } \
}

void myhtml_data_process_entry_clean(myhtml_data_process_entry_t* proc_entry)
{
    memset(proc_entry, 0, sizeof(myhtml_data_process_entry_t));
    proc_entry->state = myhtml_data_process_state_data;
}

void myhtml_data_process_string_append_char(mycore_string_t* str, const char sm)
{
    MyCORE_STRING_REALLOC_IF_NEED(str, 2, 0);
    
    str->data[str->length] = sm;
    str->length++;
    
    str->data[str->length] = '\0';
}

size_t myhtml_data_process_state_data(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size)
{
    size_t tmp_offset = offset;
    
    while(offset < size)
    {
        if(data[offset] == '&')
        {
            tmp_offset += myhtml_string_before_append_any_preprocessing(str, &data[tmp_offset], (offset - tmp_offset),
                                                                        proc_entry->tmp_str_pos_proc);
            if(offset != tmp_offset) {
                if(proc_entry->encoding == MyENCODING_UTF_8)
                    proc_entry->tmp_str_pos_proc = myhtml_string_append_with_preprocessing(str, &data[tmp_offset],
                                                                                           (offset - tmp_offset),
                                                                                           proc_entry->emit_null_char);
                else {
                    proc_entry->tmp_str_pos_proc =
                    myhtml_string_append_chunk_with_convert_encoding_with_preprocessing(str, &proc_entry->res,
                                                                                        &data[tmp_offset], (offset - tmp_offset),
                                                                                        proc_entry->encoding, proc_entry->emit_null_char);
                    myencoding_result_clean(&proc_entry->res);
                }
            }
            
            proc_entry->tmp_str_pos = str->length;
            proc_entry->state = myhtml_data_process_state_ampersand;
            
            myhtml_data_process_string_append_char(str, data[offset]);
            
            offset++;
            return offset;
        }
        
        offset++;
    }
    
    MyHTML_DATA_PROCESS_APPEND_WITH_PREPROCESSING()
    
    return offset;
}

size_t myhtml_data_process_state_ampersand(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size)
{
    if(data[offset] == '#')
    {
        myhtml_data_process_string_append_char(str, data[offset]);
        offset++;
        
        proc_entry->tmp_num = 0;
        
        if(offset >= size) {
            proc_entry->state = myhtml_data_process_state_ampersand_hash;
            return offset;
        }
        
        if(data[offset] == 'x' || data[offset] == 'X') {
            myhtml_data_process_string_append_char(str, data[offset]);
            offset++;
            
            proc_entry->state = myhtml_data_process_state_ampersand_hash_x_data;
        }
        else
            proc_entry->state = myhtml_data_process_state_ampersand_hash_data;
    }
    else {
        proc_entry->charef_res.last_entry = NULL;
        proc_entry->charef_res.curr_entry = myhtml_charef_get_first_position(data[offset]);
        
        if(proc_entry->charef_res.curr_entry->ch == '\0')
            proc_entry->state = myhtml_data_process_state_data;
        else {
            proc_entry->state = myhtml_data_process_state_ampersand_data;
            
            myhtml_data_process_string_append_char(str, data[offset]);
            offset++;
        }
    }
    
    return offset;
}

size_t myhtml_data_process_state_ampersand_data(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size)
{
    size_t tmp_offset = offset;
    
    const charef_entry_t *current_entry = myhtml_charef_find_by_pos(proc_entry->charef_res.curr_entry->next, data, &offset, size, &proc_entry->charef_res);
    
    if(proc_entry->charef_res.is_done) {
        proc_entry->state = myhtml_data_process_state_data;
        
        if(data[offset] == ';')
            offset++;
        else {
            /* if current charef is atrribute */
            if(proc_entry->is_attributes &&
               (data[offset] == '=' || mycore_string_alphanumeric_character[ (unsigned char)data[offset] ] != 0xff))
            {
                MyHTML_DATA_PROCESS_APPEND_WITH_PREPROCESSING()
                
                return offset;
            }
        }
        
        if(current_entry->codepoints_len) {
            for (size_t i = 0; i < current_entry->codepoints_len; i++) {
                MyCORE_STRING_REALLOC_IF_NEED(str, 5, 0);
                
                proc_entry->tmp_str_pos += myencoding_codepoint_to_ascii_utf_8(current_entry->codepoints[i], &str->data[proc_entry->tmp_str_pos]);
            }
            
            str->length = proc_entry->tmp_str_pos;
            str->data[str->length] = '\0';
        }
        else {
            MyHTML_DATA_PROCESS_APPEND_WITH_PREPROCESSING()
        }
        
        proc_entry->charef_res.last_entry = NULL;
    }
    else {
        MyHTML_DATA_PROCESS_APPEND_WITH_PREPROCESSING()
    }
    
    return offset;
}

size_t myhtml_data_process_state_ampersand_hash(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size)
{
    if(data[offset] == 'x' || data[offset] == 'X') {
        myhtml_data_process_string_append_char(str, data[offset]);
        offset++;
        
        proc_entry->state = myhtml_data_process_state_ampersand_hash_x_data;
    }
    else
        proc_entry->state = myhtml_data_process_state_ampersand_hash_data;
    
    return offset;
}

size_t myhtml_data_process_state_ampersand_hash_data(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size)
{
    const unsigned char *u_data = (const unsigned char*)data;
    size_t tmp_offset = offset;
    
    while(offset < size)
    {
        if(mycore_string_chars_num_map[ u_data[offset] ] == 0xff)
        {
            proc_entry->state = myhtml_data_process_state_data;
            
            if((offset - tmp_offset) == 0) {
                MyHTML_DATA_PROCESS_APPEND_WITH_PREPROCESSING()
                
                return offset;
            }
            
            if(data[offset] == ';')
                offset++;
            
            myhtml_data_process_state_end(proc_entry, str);
            return offset;
        }
        
        if(proc_entry->tmp_num <= 0x10FFFF) {
            proc_entry->tmp_num = mycore_string_chars_num_map[ u_data[offset] ] + proc_entry->tmp_num * 10;
        }
        
        offset++;
    }
    
    MyHTML_DATA_PROCESS_APPEND_WITH_PREPROCESSING()
    
    return offset;
}

size_t myhtml_data_process_state_ampersand_hash_x_data(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size)
{
    unsigned const char *u_data = (unsigned const char*)data;
    size_t tmp_offset = offset;
    
    while(offset < size)
    {
        if(mycore_string_chars_hex_map[ u_data[offset] ] == 0xff)
        {
            proc_entry->state = myhtml_data_process_state_data;
            
            if((offset - tmp_offset) == 0) {
                MyHTML_DATA_PROCESS_APPEND_WITH_PREPROCESSING()
                
                return offset;
            }
            
            if(data[offset] == ';')
                offset++;
            
            myhtml_data_process_state_end(proc_entry, str);
            return offset;
        }
        
        if(proc_entry->tmp_num <= 0x10FFFF) {
            proc_entry->tmp_num <<= 4;
            proc_entry->tmp_num |= mycore_string_chars_hex_map[ u_data[offset] ];
        }
        
        offset++;
    }
    
    MyHTML_DATA_PROCESS_APPEND_WITH_PREPROCESSING()
    
    return offset;
}

void myhtml_data_process_state_end(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str)
{
    /* 4 is max utf8 byte + \0 */
    MyCORE_STRING_REALLOC_IF_NEED(str, 5, 0);
    
    if(proc_entry->tmp_num <= 0x9F)
        proc_entry->tmp_num = replacement_character[proc_entry->tmp_num];
    else if(proc_entry->tmp_num >= 0xD800 && proc_entry->tmp_num <= 0xDFFF)
        proc_entry->tmp_num = replacement_character[0];
    else if(proc_entry->tmp_num > 0x10FFFF)
        proc_entry->tmp_num = replacement_character[0];
    
    str->length = proc_entry->tmp_str_pos +
        myencoding_codepoint_to_ascii_utf_8(proc_entry->tmp_num, &str->data[proc_entry->tmp_str_pos]);
    
    str->data[str->length] = '\0';
}

void myhtml_data_process(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t size)
{
    size_t offset = 0;
    
    while (offset < size) {
        offset = proc_entry->state(proc_entry, str, data, offset, size);
    }
}

void myhtml_data_process_end(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str)
{
    if(proc_entry->state == myhtml_data_process_state_ampersand_data && proc_entry->charef_res.last_entry)
    {
        const charef_entry_t *entry = proc_entry->charef_res.last_entry;
        
        for (size_t i = 0; i < entry->codepoints_len; i++) {
            MyCORE_STRING_REALLOC_IF_NEED(str, 5, 0);
            
            proc_entry->tmp_str_pos += myencoding_codepoint_to_ascii_utf_8(entry->codepoints[i], &str->data[proc_entry->tmp_str_pos]);
        }
        
        str->length = proc_entry->tmp_str_pos;
        str->data[str->length] = '\0';
    }
    else if(proc_entry->state == myhtml_data_process_state_ampersand_hash_data) {
        if((str->length - (proc_entry->tmp_str_pos + 2)))
            myhtml_data_process_state_end(proc_entry, str);
    }
    else if(proc_entry->state == myhtml_data_process_state_ampersand_hash_x_data) {
        if((str->length - (proc_entry->tmp_str_pos + 3)))
            myhtml_data_process_state_end(proc_entry, str);
    }
}



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

#include "myhtml/charef.h"
#include "myhtml/charef_resource.h"

const charef_entry_t * myhtml_charef_find(const char *begin, size_t *offset, size_t size, size_t *data_size)
{
    unsigned const char* u_begin = (unsigned const char*)begin;
    size_t pos = (size_t)(*u_begin);
    
    while (named_character_references[pos].ch)
    {
        if(named_character_references[pos].ch == u_begin[*offset])
        {
            (*offset)++;
            
            if(named_character_references[pos].next == 0 || *offset >= size) {
                if(data_size)
                    *data_size = named_character_references[pos].codepoints_len;
                
                return &named_character_references[pos];
            }
            
            pos = named_character_references[pos].next;
        }
        else if(u_begin[*offset] > named_character_references[pos].ch) {
            pos++;
        }
        else {
            break;
        }
    }
    
    if(data_size)
        *data_size = named_character_references[pos].codepoints_len;
    
    return &named_character_references[pos];
}

const charef_entry_t * myhtml_charef_get_first_position(const char begin)
{
    return &named_character_references[ (unsigned const char)begin ];
}

const charef_entry_t * myhtml_charef_find_by_pos(size_t pos, const char *begin, size_t *offset, size_t size, charef_entry_result_t *result)
{
    unsigned const char* u_begin = (unsigned const char*)begin;
    
    if(u_begin[*offset] == '&') {
        result->is_done = 1;
        
        if(result->curr_entry->codepoints_len)
            return result->curr_entry;
        else if(result->last_entry) {
            *offset = result->last_offset;
            return result->last_entry;
        }
        
        return &named_character_references[0];
    }
    
    result->is_done = 0;
    
    while (named_character_references[pos].ch)
    {
        if(named_character_references[pos].ch == u_begin[*offset])
        {
            if(u_begin[*offset] == ';') {
                result->is_done = 1;
                
                result->curr_entry = &named_character_references[pos];
                return result->curr_entry;
            }
            
            (*offset)++;
            
            if(named_character_references[pos].next == 0) {
                result->is_done = 1;
                return &named_character_references[pos];
            }
            
            if(*offset >= size)
            {
                result->curr_entry = &named_character_references[pos];
                
                if(named_character_references[pos].codepoints_len) {
                    result->last_offset = *offset;
                    result->last_entry = &named_character_references[pos];
                }
                
                return result->curr_entry;
            }
            
            if(u_begin[*offset] == '&') {
                result->is_done = 1;
                result->curr_entry = &named_character_references[pos];
                
                if(result->curr_entry->codepoints_len)
                    return result->curr_entry;
                else if(result->last_entry) {
                    *offset = result->last_offset;
                    return result->last_entry;
                }
                
                return &named_character_references[0];
            }
            
            if(named_character_references[pos].codepoints_len) {
                result->last_offset = *offset;
                result->last_entry = &named_character_references[pos];
            }
            
            pos = named_character_references[pos].next;
        }
        else if(u_begin[*offset] > named_character_references[pos].ch) {
            pos++;
        }
        else {
            break;
        }
    }
    
    result->is_done = 1;
    
    if(result->last_entry) {
        *offset = result->last_offset;
        return result->last_entry;
    }
    
    return &named_character_references[0];
}



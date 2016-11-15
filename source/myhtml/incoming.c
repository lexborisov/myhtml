/*
 Copyright (C) 2016 Alexander Borisov
 
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

#include "myhtml/incoming.h"
#include "myhtml/utils/resources.h"

myhtml_incoming_buffer_t * myhtml_incoming_buffer_add(myhtml_incoming_buffer_t *current, mcobject_t *mcobject,
                                                       const char *html, size_t html_size)
{
    myhtml_incoming_buffer_t *inc_buf = mcobject_malloc(mcobject, NULL);
    
    inc_buf->size   = html_size;
    inc_buf->length = 0;
    inc_buf->data   = html;
    
    if(current) {
        inc_buf->offset = current->offset + current->size;
        current->next   = inc_buf;
    }
    else {
        inc_buf->offset = 0;
    }
    
    inc_buf->prev = current;
    inc_buf->next = NULL;
    
    return inc_buf;
}

void myhtml_incoming_buffer_clean(myhtml_incoming_buffer_t *current)
{
    memset(current, 0, sizeof(myhtml_incoming_buffer_t));
}

myhtml_incoming_buffer_t * myhtml_incoming_buffer_split(myhtml_incoming_buffer_t *current, mcobject_t *mcobject, size_t global_pos)
{
    size_t relative_pos = global_pos - current->offset;
    myhtml_incoming_buffer_t *inc_buf = mcobject_malloc(mcobject, NULL);
    
    inc_buf->size   = current->size - relative_pos;
    inc_buf->length = inc_buf->size;
    inc_buf->data   = &current->data[relative_pos];
    inc_buf->offset = current->offset + relative_pos;
    inc_buf->next   = NULL;
    inc_buf->prev   = current;
    
    current->next   = inc_buf;
    current->size   = relative_pos;
    current->length = relative_pos;
    
    return inc_buf;
}

myhtml_incoming_buffer_t * myhtml_incoming_buffer_find_by_position(myhtml_incoming_buffer_t *inc_buf, size_t begin)
{
    if(inc_buf->offset < begin) {
        while(inc_buf && (inc_buf->offset + inc_buf->size) < begin)
            inc_buf = inc_buf->next;
    }
    else {
        while(inc_buf && inc_buf->offset > begin)
            inc_buf = inc_buf->prev;
    }
    
    return inc_buf;
}

const char * myhtml_incoming_buffer_data(myhtml_incoming_buffer_t *inc_buf)
{
    return inc_buf->data;
}

size_t myhtml_incoming_buffer_length(myhtml_incoming_buffer_t *inc_buf)
{
    return inc_buf->length;
}

size_t myhtml_incoming_buffer_size(myhtml_incoming_buffer_t *inc_buf)
{
    return inc_buf->size;
}

size_t myhtml_incoming_buffer_offset(myhtml_incoming_buffer_t *inc_buf)
{
    return inc_buf->offset;
}

size_t myhtml_incoming_buffer_relative_begin(myhtml_incoming_buffer_t *inc_buf, size_t begin)
{
    return (begin - inc_buf->offset);
}

size_t myhtml_incoming_buffer_available_length(myhtml_incoming_buffer_t *inc_buf, size_t relative_begin, size_t length)
{
    if((relative_begin + length) > inc_buf->size)
        return (inc_buf->size - relative_begin);
    
    return length;
}

myhtml_incoming_buffer_t * myhtml_incoming_buffer_next(myhtml_incoming_buffer_t *inc_buf)
{
    return inc_buf->next;
}

myhtml_incoming_buffer_t * myhtml_incoming_buffer_prev(myhtml_incoming_buffer_t *inc_buf)
{
    return inc_buf->prev;
}

// // //
// convert only one 002345 (\002345) to code point
//
size_t myhtml_incoming_buffer_convert_one_escaped_to_code_point(myhtml_incoming_buffer_t **inc_buf, size_t *relative_pos)
{
    const unsigned char *u_data;
    myhtml_incoming_buffer_t *current = *inc_buf;
    
    if(*relative_pos >= current->size) {
        *relative_pos = 0;
        current = current->next;
    }
    
    u_data = (const unsigned char*)current->data;
    
    unsigned int consume = 0;
    size_t code_point = 0;
    
    while(current)
    {
        if(myhtml_string_chars_num_map[ u_data[*relative_pos] ] != 0xff && consume < 6) {
            code_point <<= 4;
            code_point |= myhtml_string_chars_hex_map[ u_data[*relative_pos] ];
            
            ++consume;
        }
        else
            break;
        
        *relative_pos += 1;
        
        if(*relative_pos >= current->size)
        {
            if(current->next == NULL)
                break;
            
            *relative_pos = 0;
            
            u_data  = (const unsigned char*)current->data;
            current = current->next;
        }
    }
    
    *inc_buf = current;
    
    return code_point;
}

size_t myhtml_incoming_buffer_escaped_case_cmp(myhtml_incoming_buffer_t **inc_buf, const char *to, size_t to_size, size_t *relative_pos)
{
    myhtml_incoming_buffer_t *current = *inc_buf;
    
    if(*relative_pos >= current->size) {
        if(current->next == 0)
            return to_size;
        
        *relative_pos = 0;
        current = current->next;
    }
    
    const unsigned char *u_to = (const unsigned char*)to;
    const unsigned char *u_data = (const unsigned char*)current->data;
    
    size_t i = 0;
    
    while(i < to_size)
    {
        if(u_data[*relative_pos] == 0x5C) {
            *relative_pos += 1;
            
            size_t code_point = myhtml_incoming_buffer_convert_one_escaped_to_code_point(&current, relative_pos);
            
            if(code_point > 255 || myhtml_string_chars_lowercase_map[code_point] != myhtml_string_chars_lowercase_map[ u_to[i] ]) {
                break;
            }
            
            u_data = (const unsigned char*)current->data;
        }
        else if(myhtml_string_chars_lowercase_map[ u_data[*relative_pos] ] != myhtml_string_chars_lowercase_map[ u_to[i] ]) {
            break;
        }
        else {
            ++(*relative_pos);
        }
        
        i++;
        
        if(*relative_pos >= current->size)
        {
            if(current->next == NULL)
                break;
            
            current       = current->next;
            u_data        = (const unsigned char*)current->data;
            *relative_pos = 0;
        }
    }
    
    *inc_buf = current;
    return (to_size - i);
}





/*
 Copyright 2016 Alexander Borisov
 
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

#include "myhtml/incoming.h"
#include "myhtml/utils/resources.h"

myhtml_incoming_buffer_t * myhtml_incomming_buffer_add(myhtml_incoming_buffer_t *current, mcobject_async_t *mcobject, size_t mcnode_id,
                                                       const char *html, size_t html_size)
{
    myhtml_incoming_buffer_t *inc_buf = mcobject_async_malloc(mcobject, mcnode_id, NULL);
    
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

void myhtml_incomming_buffer_clean(myhtml_incoming_buffer_t *current)
{
    memset(current, 0, sizeof(myhtml_incoming_buffer_t));
}

myhtml_incoming_buffer_t * myhtml_incoming_buffer_find_by_position(myhtml_incoming_buffer_t *inc_buf, size_t position)
{
    if(inc_buf->offset < position) {
        while(inc_buf && (inc_buf->offset + inc_buf->size) < position)
            inc_buf = inc_buf->next;
    }
    else {
        while(inc_buf && inc_buf->offset > position)
            inc_buf = inc_buf->prev;
    }
    
    return inc_buf;
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
            
            if(code_point > 255 || myhtml_string_chars_num_map[code_point] != myhtml_string_chars_num_map[ u_to[i] ]) {
                break;
            }
            
            u_data = (const unsigned char*)current->data;
        }
        else if(myhtml_string_chars_num_map[ u_data[*relative_pos] ] != myhtml_string_chars_num_map[ u_to[i] ]) {
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





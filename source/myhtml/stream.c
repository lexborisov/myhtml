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

#include "myhtml/stream.h"

myhtml_stream_buffer_t * myhtml_stream_buffer_create(void)
{
    return mycore_calloc(1, sizeof(myhtml_stream_buffer_t));
}

mystatus_t myhtml_stream_buffer_init(myhtml_stream_buffer_t* stream_buffer, size_t entries_size)
{
    stream_buffer->length = 0;
    stream_buffer->size = entries_size;
    stream_buffer->entries = mycore_calloc(entries_size, sizeof(myhtml_stream_buffer_entry_t));
    
    if(stream_buffer->entries == NULL)
        return MyHTML_STATUS_STREAM_BUFFER_ERROR_INIT;
    
    return MyHTML_STATUS_OK;
}

mystatus_t myhtml_stream_buffer_entry_init(myhtml_stream_buffer_entry_t* stream_buffer_entry, size_t size)
{
    if(stream_buffer_entry->data) {
        if(size <= stream_buffer_entry->size)
            return MyHTML_STATUS_OK;
        else
            mycore_free(stream_buffer_entry->data);
    }
    
    stream_buffer_entry->length = 0;
    stream_buffer_entry->size = size;
    stream_buffer_entry->data = mycore_malloc(size * sizeof(char));
    
    if(stream_buffer_entry->data == NULL)
        return MyHTML_STATUS_STREAM_BUFFER_ENTRY_ERROR_INIT;
    
    return MyHTML_STATUS_OK;
}

void myhtml_stream_buffer_entry_clean(myhtml_stream_buffer_entry_t* stream_buffer_entry)
{
    if(stream_buffer_entry)
        stream_buffer_entry->length = 0;
}

myhtml_stream_buffer_entry_t * myhtml_stream_buffer_entry_destroy(myhtml_stream_buffer_entry_t* stream_buffer_entry, bool self_destroy)
{
    if(stream_buffer_entry == NULL)
        return NULL;
    
    if(stream_buffer_entry->data)
        mycore_free(stream_buffer_entry->data);
    
    if(self_destroy) {
        mycore_free(stream_buffer_entry);
        return NULL;
    }
    
    return stream_buffer_entry;
}

void myhtml_stream_buffer_clean(myhtml_stream_buffer_t* stream_buffer)
{
    if(stream_buffer)
        stream_buffer->length = 0;
}

myhtml_stream_buffer_t * myhtml_stream_buffer_destroy(myhtml_stream_buffer_t* stream_buffer, bool self_destroy)
{
    if(stream_buffer == NULL)
        return NULL;
    
    if(stream_buffer->entries) {
        for(size_t i = 0; i < stream_buffer->length; i++)
            myhtml_stream_buffer_entry_destroy(&stream_buffer->entries[i], false);
        
        mycore_free(stream_buffer->entries);
    }
    
    if(self_destroy) {
        mycore_free(stream_buffer);
        return NULL;
    }
    
    return stream_buffer;
}

myhtml_stream_buffer_entry_t * myhtml_stream_buffer_add_entry(myhtml_stream_buffer_t* stream_buffer, size_t entry_data_size)
{
    if(stream_buffer->length >= stream_buffer->size) {
        size_t new_size = stream_buffer->size << 1;
        
        myhtml_stream_buffer_entry_t *entries = mycore_realloc(stream_buffer, sizeof(myhtml_stream_buffer_entry_t) * new_size);
        
        if(entries) {
            memset(&entries[stream_buffer->size], 0, (new_size - stream_buffer->size));
            
            stream_buffer->entries = entries;
            stream_buffer->size = new_size;
        }
        else
            return NULL;
    }
    
    myhtml_stream_buffer_entry_t *entry = &stream_buffer->entries[ stream_buffer->length ];
    mystatus_t status = myhtml_stream_buffer_entry_init(entry, entry_data_size);
    
    if(status != MyHTML_STATUS_OK)
        return NULL;
    
    stream_buffer->length++;
    
    return entry;
}

myhtml_stream_buffer_entry_t * myhtml_stream_buffer_current_entry(myhtml_stream_buffer_t* stream_buffer)
{
    if(stream_buffer->length == 0)
        return NULL;
    
    return &stream_buffer->entries[ (stream_buffer->length - 1) ];
}



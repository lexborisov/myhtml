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

#include "mycore/utils/mcsimple.h"

mcsimple_t * mcsimple_create(void)
{
    return mycore_calloc(1, sizeof(mcsimple_t));
}

void mcsimple_init(mcsimple_t *mcsimple, size_t pos_size, size_t list_size, size_t struct_size)
{
    mcsimple->struct_size = struct_size;
    
    mcsimple->list_pos_length_used = 0;
    mcsimple->list_pos_length = 0;
    mcsimple->list_pos_size = pos_size;
    mcsimple->list = (uint8_t**)mycore_calloc(pos_size, sizeof(uint8_t*));
    
    if(mcsimple->list == NULL) {
        return;
    }
    
    mcsimple->list_size = list_size * struct_size;
    
    if((mcsimple_init_list_entries(mcsimple, mcsimple->list_pos_length) == NULL)) {
        return;
    }
}

void mcsimple_clean(mcsimple_t *mcsimple)
{
    mcsimple->list_length = 0;
    mcsimple->list_pos_length = 0;
}

mcsimple_t * mcsimple_destroy(mcsimple_t *mcsimple, bool destroy_self)
{
    if(mcsimple == NULL)
        return NULL;
    
    if(mcsimple->list) {
        for(size_t i = 0; i < mcsimple->list_pos_length_used; i++) {
            if(mcsimple->list[i])
                mycore_free(mcsimple->list[i]);
        }
        
        mycore_free(mcsimple->list);
    }
    
    if(destroy_self) {
        mycore_free(mcsimple);
        return NULL;
    }
    
    return mcsimple;
}

uint8_t * mcsimple_init_list_entries(mcsimple_t *mcsimple, size_t pos)
{
    if(mcsimple->list_pos_length >= mcsimple->list_pos_size)
    {
        size_t new_size = mcsimple->list_pos_size + 128;
        uint8_t **list = (uint8_t**)mycore_realloc(mcsimple->list, mcsimple->list_pos_size * sizeof(uint8_t*));
        
        if(list) {
            mcsimple->list = list;
            memset(&mcsimple->list[pos], 0, (new_size - mcsimple->list_pos_size) * sizeof(uint8_t*));
            
            mcsimple->list_pos_size = new_size;
        }
        else
            return NULL;
    }
    
    mcsimple->list_length = 0;
    
    if(mcsimple->list[pos] == NULL) {
        mcsimple->list_pos_length_used++;
        mcsimple->list[pos] = (uint8_t*)mycore_malloc(mcsimple->list_size * sizeof(uint8_t));
    }
    
    return mcsimple->list[pos];
}

void * mcsimple_malloc(mcsimple_t *mcsimple)
{
    if(mcsimple->list_length >= mcsimple->list_size)
    {
        mcsimple->list_pos_length++;
        if((mcsimple_init_list_entries(mcsimple, mcsimple->list_pos_length) == NULL)) {
            return NULL;
        }
    }
    
    size_t current = mcsimple->list_length;
    mcsimple->list_length += mcsimple->struct_size;
    
    return &mcsimple->list[mcsimple->list_pos_length][current];
}

void * mcsimple_get_by_absolute_position(mcsimple_t *mcsimple, size_t pos)
{
    pos *= mcsimple->struct_size;
    
    size_t list_index = pos / mcsimple->list_size;
    if(list_index >= mcsimple->list_pos_length_used)
        return NULL;
    
    return &mcsimple->list[list_index][ (pos % mcsimple->list_size) ];
}



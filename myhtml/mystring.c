//
//  myhtml_string.c
//  myhtml
//
//  Created by Alexander Borisov on 29.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

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
    myhtml_string_check(str, length);
    
    memcpy(&str->data[begin], buff, (sizeof(char) * length));
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




//
//  myhtml_string.c
//  myhtml
//
//  Created by Alexander Borisov on 29.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_string.h"

myhtml_string_t * myhtml_string_create(size_t size)
{
    myhtml_string_t* str = (myhtml_string_t*)malloc(sizeof(myhtml_string_t));
    myhtml_string_init(str, size);
    return str;
}

void myhtml_string_init(myhtml_string_t* str, size_t size)
{
    if(size < 128)
        size = 128;
    
    str->size = size;
    str->data = (char*)malloc(sizeof(char) * size); // char allways 1
    
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
    
    if(str->data)
        free(str->data);
    
    if(destroy_obj && str)
        free(str);
    
    return NULL;
}

mybool_t myhtml_string_check(myhtml_string_t* str, size_t length, size_t up_to)
{
    length = str->length + length;
    
    if(length >= str->size)
    {
        if(up_to < 128)
            up_to = 128;
        
        str->size = length + up_to;
        char* tmp = (char*)myrealloc(str->data, str->size);
        
        if(tmp)
            str->data = tmp;
        else
            return myfalse;
    }
    
    str->length = length;
    
    return mytrue;
}

void myhtml_string_append(myhtml_string_t* str, const char* buff, size_t length)
{
    size_t begin = str->length;
    myhtml_string_check(str, length, (4096 * 20));
    
    memcpy(&str->data[begin], buff, (sizeof(char) * length));
}

void myhtml_string_append_with_null(myhtml_string_t* str, const char* buff, size_t length)
{
    size_t begin = str->length;
    myhtml_string_check(str, (length + 1), (4096 * 20));
    
    memcpy(&str->data[begin], buff, (sizeof(char) * length));
    str->data[length] = '\0';
}

void myhtml_string_append_one_without_check(myhtml_string_t* str, const char buff)
{
    str->data[str->length] = buff;
    str->length++;
}

void myhtml_string_append_one(myhtml_string_t* str, const char buff)
{
    myhtml_string_check(str, 1, (4096 * 20));
    
    str->data[str->length] = buff;
    str->length++;
}

void myhtml_string_append_lowercase_with_null(myhtml_string_t* str, const char* buff, size_t length)
{
    size_t begin = str->length;
    myhtml_string_check(str, (length + 1), (4096 * 20));
    
    char* cache = &str->data[begin];
    
    size_t i;
    for(i = 0; i < length; i++) {
        cache[i] = buff[i] > 0x40 && buff[i] < 0x5b ? (buff[i]|0x60) : buff[i];
    }
    cache[i] = '\0';
}


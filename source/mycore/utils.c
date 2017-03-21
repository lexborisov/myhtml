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

#include "mycore/utils.h"
#include "mycore/utils/resources.h"

size_t mycore_power(size_t t, size_t k)
{
    size_t res = 1;
    
    while (k) {
        if(k & 1) res *= t;
        t *= t;
        k >>= 1;
    }
    
    return res;
}

size_t mycore_strncasecmp(const char* str1, const char* str2, size_t size)
{
    if(str1 == NULL || str2 == NULL) {
        if(str1 == str2)
            return 0;
        
        return 1;
    }
    
    const unsigned char *ustr1 = (const unsigned char *)str1;
    const unsigned char *ustr2 = (const unsigned char *)str2;
    
    size_t i = 0;
    while (i < size) {
        if(mycore_string_chars_lowercase_map[*ustr1] != mycore_string_chars_lowercase_map[*ustr2])
            return (size - i);
        
        ustr1++;
        ustr2++;
        
        i++;
    }
    
    return 0;
}

size_t mycore_strcasecmp(const char* str1, const char* str2)
{
    if(str1 == NULL || str2 == NULL) {
        if(str1 == str2)
            return 0;
        
        return 1;
    }
    
    const unsigned char *ustr1 = (const unsigned char *)str1;
    const unsigned char *ustr2 = (const unsigned char *)str2;
    
    for (size_t i = 0;;)
    {
        if(mycore_string_chars_lowercase_map[*ustr1] != mycore_string_chars_lowercase_map[*ustr2])
            return (i + 1);
        
        if(*ustr1 == '\0')
            return 0;
        
        ustr1++;
        ustr2++;
        
        i++;
    }
}

size_t mycore_strncmp(const char* str1, const char* str2, size_t size)
{
    if(str1 == NULL || str2 == NULL) {
        if(str1 == str2)
            return 0;
        
        return 1;
    }
    
    const unsigned char *ustr1 = (const unsigned char *)str1;
    const unsigned char *ustr2 = (const unsigned char *)str2;
    
    size_t i = 0;
    while (i < size) {
        if(*ustr1 != *ustr2)
            return (size - i);
        
        ustr1++;
        ustr2++;
        
        i++;
    }
    
    return 0;
}

size_t mycore_strcmp(const char* str1, const char* str2)
{
    if(str1 == NULL || str2 == NULL) {
        if(str1 == str2)
            return 0;
        
        return 1;
    }
    
    const unsigned char *ustr1 = (const unsigned char *)str1;
    const unsigned char *ustr2 = (const unsigned char *)str2;
    
    for (size_t i = 0;;)
    {
        if(*ustr1 != *ustr2)
            return (i + 1);
        
        if(*ustr1 == '\0')
            return 0;
        
        ustr1++;
        ustr2++;
        
        i++;
    }
}

size_t mycore_strcmp_ws(const char* str1, const char* str2)
{
    if(str1 == NULL || str2 == NULL) {
        if(str1 == str2)
            return 0;
        
        return 1;
    }
    
    const unsigned char *ustr1 = (const unsigned char *)str1;
    const unsigned char *ustr2 = (const unsigned char *)str2;
    
    for (size_t i = 0;;)
    {
        if(*ustr1 != *ustr2)
            return (i + 1);
        
        if(mycore_utils_whithspace(*ustr1, ==, ||) || *ustr1 == '\0')
            return 0;
        
        ustr1++;
        ustr2++;
        
        i++;
    }
}

bool mycore_ustrcasecmp_without_checks_by_secondary(const unsigned char* ustr1, const unsigned char* ustr2)
{
    while (*ustr1 != '\0') {
        if(mycore_string_chars_lowercase_map[*ustr1] != mycore_string_chars_lowercase_map[*ustr2])
            return false;
        
        ustr1++;
        ustr2++;
    }
    
    return true;
}



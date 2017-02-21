/*
 Copyright (C) 2015-2016 Alexander Borisov
 
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

#include "myhtml/utils.h"

size_t myhtml_power(size_t t, size_t k)
{
    size_t res = 1;
    
    while (k) {
        if(k & 1) res *= t;
        t *= t;
        k >>= 1;
    }
    
    return res;
}

static const unsigned char myhtml_utils_chars_map[] = {
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

size_t myhtml_strncasecmp(const char* str1, const char* str2, size_t size)
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
        if(myhtml_utils_chars_map[*ustr1] != myhtml_utils_chars_map[*ustr2])
            return (size - i);
        
        ustr1++;
        ustr2++;
        
        i++;
    }
    
    return 0;
}

size_t myhtml_strcasecmp(const char* str1, const char* str2)
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
        if(myhtml_utils_chars_map[*ustr1] != myhtml_utils_chars_map[*ustr2])
            return (i + 1);
        
        if(*ustr1 == '\0')
            return 0;
        
        ustr1++;
        ustr2++;
        
        i++;
    }
}

size_t myhtml_strncmp(const char* str1, const char* str2, size_t size)
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

size_t myhtml_strcmp(const char* str1, const char* str2)
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

size_t myhtml_strcmp_ws(const char* str1, const char* str2)
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
        
        if(myhtml_utils_whithspace(*ustr1, ==, ||) || *ustr1 == '\0')
            return 0;
        
        ustr1++;
        ustr2++;
        
        i++;
    }
}

bool myhtml_ustrcasecmp_without_checks_by_secondary(const unsigned char* ustr1, const unsigned char* ustr2)
{
    while (*ustr1 != '\0') {
        if(myhtml_utils_chars_map[*ustr1] != myhtml_utils_chars_map[*ustr2])
            return false;
        
        ustr1++;
        ustr2++;
    }
    
    return true;
}



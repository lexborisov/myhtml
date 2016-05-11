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

#include "myhtml/myosi.h"

void * mymalloc(size_t size)
{
    //printf("Call malloc\n");
    return malloc(size);
}

// see who's call realloc
void * myrealloc(void* dst, size_t size)
{
    //printf("Call realloc\n");
    return realloc(dst, size);
}

void * mycalloc(size_t num, size_t size)
{
    //printf("Call calloc\n");
    return calloc(num, size);
}

void myfree(void* dst)
{
    //printf("Call free\n");
    free(dst);
}

void myhtml_print(FILE* out, const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vfprintf(out, format, argptr);
    va_end(argptr);
}

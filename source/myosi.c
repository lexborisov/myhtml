/*
 Copyright 2015 Alexander Borisov
 
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

#include "myosi.h"

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
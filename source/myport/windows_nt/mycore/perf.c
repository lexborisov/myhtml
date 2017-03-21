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
 
 Authors: lex.borisov@gmail.com (Alexander Borisov)
*/

#include "mycore/myosi.h"
#include "mycore/perf.h"

#ifdef MyCORE_WITH_PERF
#include <windows.h>

struct mycore_perf {
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    LARGE_INTEGER freq;
}
typedef mycore_perf_t;

void * mycore_perf_create(void)
{
    mycore_perf_t *perf = mycore_calloc(1, sizeof(mycore_perf_t));
    if(perf == NULL)
        return NULL;
    
    QueryPerformanceFrequency(&perf->freq);
    
    return perf;
}

void mycore_perf_clean(void* perf)
{
    memset(perf, 0, sizeof(mycore_perf_t));
}

void mycore_perf_destroy(void* perf)
{
    if(perf)
        mycore_free(perf);
}

mycore_status_t myhtml_perf_begin(void* perf)
{
    QueryPerformanceCounter(&((mycore_perf_t*)(perf))->start);
    return MyCORE_STATUS_OK;
}

mycore_status_t myhtml_perf_end(void* perf)
{
    QueryPerformanceCounter(&((mycore_perf_t*)(perf))->end);
    return MyCORE_STATUS_OK;
}

double myhtml_perf_in_sec(void* perf)
{
    mycore_perf_t *obj_perf = (mycore_perf_t*)perf;
    
    if(obj_perf->freq.QuadPart)
        return ((double)(obj_perf->end.QuadPart - obj_perf->start.QuadPart) / (double)obj_perf->freq.QuadPart);
    
    return 0.0f;
}

unsigned long long mycore_perf_clock(void)
{
    LARGE_INTEGER x;
    QueryPerformanceFrequency(&x);
    return (unsigned long long)x.QuadPart;
}

unsigned long long mycore_perf_frequency(void)
{
    LARGE_INTEGER x;
    QueryPerformanceFrequency(&x);
    return (unsigned long long)x.QuadPart;
}

#endif /* MyCORE_WITH_PERF */

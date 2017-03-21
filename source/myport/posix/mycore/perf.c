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
 
 Authors: lex.borisov@gmail.com (Alexander Borisov)
*/

#include "mycore/myosi.h"
#include "mycore/perf.h"

#ifdef MyCORE_WITH_PERF

#if MODEST_BUILD_OS == Darwin || MODEST_BUILD_OS == FreeBSD
#include <sys/sysctl.h>
#elif MODEST_BUILD_OS == Linux
#endif

struct mycore_perf {
    unsigned long long start;
    unsigned long long end;
    unsigned long long freq;
}
typedef mycore_perf_t;

void * mycore_perf_create(void)
{
    mycore_perf_t *perf = mycore_calloc(1, sizeof(mycore_perf_t));
    if(perf == NULL)
        return NULL;
    
    perf->freq = mycore_perf_frequency();
    
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
    ((mycore_perf_t*)(perf))->start = mycore_perf_clock();
    return MyCORE_STATUS_OK;
}

mycore_status_t myhtml_perf_end(void* perf)
{
    ((mycore_perf_t*)(perf))->end = mycore_perf_clock();
    return MyCORE_STATUS_OK;
}

double myhtml_perf_in_sec(void* perf)
{
    mycore_perf_t *obj_perf = (mycore_perf_t*)perf;
    
    if(obj_perf->freq)
        return ((double)(obj_perf->end - obj_perf->start) / (double)obj_perf->freq);
    
    return 0.0f;
}

unsigned long long mycore_perf_clock(void)
{
    unsigned long long x;
    
    __asm__ volatile (
                      "cpuid\n\t" /* cpuid serializes any out-of-order prefetches before executing rdtsc (clobbers ebx, ecx, edx) */
                      "rdtsc\n\t"
                      "shl $32, %%rdx\n\t"
                      "or %%rdx, %%rax"
                      : "=a" (x)
                      :
                      : "rdx", "ebx", "ecx");
    
    return x;
}

unsigned long long mycore_perf_frequency(void)
{
    unsigned long long freq = 0;
    
#if MODEST_BUILD_OS == Darwin && defined(CTL_HW) && defined(HW_CPU_FREQ)
    /* OSX kernel: sysctl(CTL_HW | HW_CPU_FREQ) */
    size_t len = sizeof(freq);
    int mib[2] = {CTL_HW, HW_CPU_FREQ};
    
    if(sysctl(mib, 2, &freq, &len, NULL, 0))
        return 0;
    
    return freq;
    
#elif MODEST_BUILD_OS == Linux
    /* Use procfs on linux */
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if(fp == NULL)
        return 0;
    
    /* Find 'CPU MHz :' */
    char buf[1024] = {0};
    double fval = 0.0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (sscanf(buf, "cpu MHz : %lf\n", &fval) == 1) {
            freq = (unsigned long long)(fval * 1000000ull);
            break;
        }
    }
    
    fclose(fp);
    return freq;

#else
    return freq;
#endif /* MODEST_BUILD_OS == Darwin || Linux */
}

#endif /* MyCORE_WITH_PERF */

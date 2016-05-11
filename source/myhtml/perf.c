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
 
 Authors: insoreiges@gmail.com (Evgeny Yakovlev), lex.borisov@gmail.com (Alexander Borisov)
*/

/**
 * Platform-specific hdef performance clock value. 
 */ 

#include "myhtml/myhtml.h"
#include <time.h>

#if !defined(IS_OS_WINDOWS)
#include <unistd.h>
#endif

#if !defined(MyHTML_WITH_PERF)

uint64_t myhtml_hperf_res(myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_PERF_ERROR_COMPILED_WITHOUT_PERF;
    
    return 0;
}

uint64_t myhtml_hperf_clock(myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_PERF_ERROR_COMPILED_WITHOUT_PERF;
    
    return 0;
}

#else

#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(IS_OS_WINDOWS)
#endif

#if defined(MyHTML_FORCE_RDTSC) /* Force using rdtsc, useful for comparison */

/**
 * Get CPU rdtsc frequency.
 * 
 * TODO: I think using rdtsc for measuring user-space counters is not correct:
 * - rdtsc does not have a constant rate. instead ot is scaled to physical core's internal clock which changes due to power saving modes on modern CPUs
 * - rdtsc is software-emulated in virtual machines which will introduce an inconsistency in reported ticks
 * - user space process can be preempted between consecutive rdtsc measures but the physical clock will still tick while it is executing a different thread.
 *   also think what would happen if preempted process will be re-scheduled on a different physical core which has a different tsc value.
 * - computing rdtsc frequency produces unreliable results (due to all of the above)
 *
 * Consider using platform-specific monotonic hperf timers (ftrace/dtrace) or even clock().
 */
uint64_t myhtml_hperf_res(myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_OK;
    
#if defined(__APPLE__) && defined(CTL_HW) && defined(HW_CPU_FREQ)
    unsigned long long freq = 0;
    
    /* OSX kernel: sysctl(CTL_HW | HW_CPU_FREQ) */
    size_t len = sizeof(freq);
    int mib[2] = {CTL_HW, HW_CPU_FREQ};

    int error = sysctl(mib, 2, &freq, &len, NULL, 0);
    if (error) {
        if(status)
            *status = MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK;
        
        return 0;
    }

    return freq;

#elif defined(__linux__)
    unsigned long long freq = 0;
    
    /* Use procfs on linux */
    FILE* fp = NULL;
    fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        if(status)
            *status = MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK;
        
        return 0;
    }

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
#   warning Cant figure out cpu frequency on this platfrom
    
    if(status)
        *status = MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK;
    
    return 0;
#endif /* defined __APPLE__ || __linux__ ... */
}

uint64_t myhtml_hperf_clock(myhtml_status_t *status)
{
    uint64_t x;

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

#elif defined(_POSIX_TIMERS) &&  defined(_POSIX_CPUTIME) \
    && defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L) /* Do we have clock_gettime? */

#define NSEC_PER_SECOND         1000000000ull
#define TIMESPEC_TO_USEC(tspec) (((uint64_t)(tspec).tv_sec * NSEC_PER_SECOND) + (tspec).tv_nsec)

uint64_t myhtml_hperf_res(myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_OK;
    
    struct timespec tspec;
    int error = clock_getres(CLOCK_PROCESS_CPUTIME_ID, &tspec);
    if (error) {
        if(status)
            *status = MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK;
        
        return 0;
    }

    unsigned long long ticks_per_sec = (unsigned long long)((double)NSEC_PER_SECOND / TIMESPEC_TO_USEC(tspec));
    return ticks_per_sec;
}

uint64_t myhtml_hperf_clock(myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_OK;
    
    struct timespec tspec;
    int error = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tspec);
    if (error) {
        if(status)
            *status = MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK;
        
        return 0;
    }

    return TIMESPEC_TO_USEC(tspec);
}

#elif defined(__APPLE__) && defined(__MACH__)

/* 
 * TODO: on OSX we can use clock_get_time: http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x 
 * Or this: http://web.archive.org/web/20100517095152/http://www.wand.net.nz/~smr26/wordpress/2009/01/19/monotonic-time-in-mac-os-x/comment-page-1/
 */

// TODO: this is incorrect plug for mac os x
// look at links before this comment

#include <mach/mach_time.h>

uint64_t myhtml_hperf_res(myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_OK;
    
    unsigned long long freq = 0;
    
    size_t len = sizeof(freq);
    int mib[2] = {CTL_HW, HW_CPU_FREQ};
    
    int error = sysctl(mib, 2, &freq, &len, NULL, 0);
    if (error) {
        if(status)
            *status = MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK;
        
        return 0;
    }
    
    return freq;
}

uint64_t myhtml_hperf_clock(myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_OK;
    
    return mach_absolute_time();
}

#else

#   warning No hperf implementation for this platform

uint64_t myhtml_hperf_res(myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK;
    
    return 0;
}

uint64_t myhtml_hperf_clock(myhtml_status_t *status)
{
    if(status)
        *status = MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK;
    
    return 0;
}

#endif /* defined(MyHTML_FORCE_RDTSC) ... */
#endif /* MyHTML_WITH_PERF */

#define _MyHTML_CHECK_STATUS_AND_PRINT_ERROR \
    if(status == MyHTML_STATUS_PERF_ERROR_COMPILED_WITHOUT_PERF) { \
        fprintf(fh, "MyHTML: Library compiled without perf source. Please, build library with -DMyHTML_WITH_PERF flag\n"); \
    } \
    else if(status) { \
        fprintf(fh, "MyHTML: Something wrong! Perhaps, your platform does not support the measurement of performance\n"); \
    } \
    else

myhtml_status_t myhtml_hperf_print(const char *name, uint64_t x, uint64_t y, FILE *fh) {
    myhtml_status_t status;
    
    unsigned long long freq = myhtml_hperf_res(&status);
    
    if(freq) {
        _MyHTML_CHECK_STATUS_AND_PRINT_ERROR {
            fprintf(fh, "%s: %0.5f\n", name, (((float)(y - x) / (float)freq)));
        }
    }
    
    return status;
}

myhtml_status_t myhtml_hperf_print_by_val(const char *name, uint64_t x, FILE *fh) {
    myhtml_status_t status;
    
    unsigned long long freq = myhtml_hperf_res(&status);
    
    if(freq) {
        _MyHTML_CHECK_STATUS_AND_PRINT_ERROR {
            fprintf(fh, "%s: %0.5f\n", name, ((float)x / (float)freq));
        }
    }
    
    return status;
}



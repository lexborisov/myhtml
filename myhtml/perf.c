/** 
 * Platform-specific hdef performance clock value. 
 */ 

#include <unistd.h>
#include <time.h>

#include "myhtml.h"

#if defined(MYHTML_FORCE_RDTSC) /* Force using rdtsc, useful for comparison */

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
uint64_t myhtml_hperf_res(void)
{
    unsigned long long freq = 0;

#if defined(CTL_HW) && defined(HW_CPU_FREQ)
    
    /* OSX kernel: sysctl(CTL_HW | HW_CPU_FREQ) */
    size_t len = sizeof(freq);
    int mib[2] = {CTL_HW, HW_CPU_FREQ}

    int error = sysctl(mib, 2, &freq, &len, NULL, 0);
    if (error) {
        /* TODO: error checking */
        return 0;
    }

    return freq;

#elif defined(__linux__)

    /* Use procfs on linux */
    FILE* fp = NULL;
    fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        /* TODO: error checking */
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
#   error Cant figure out cpu frequency on this platfrom
#endif 
}

uint64_t myhtml_hperf_clock(void) 
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

#elif defined(_POSIX_TIMERS) &&  defined(_POSIX_CPUTIME) /* Do we have clock_gettime? */

/* clock_gettime needs _POSIX_C_SOURCE >= 199309L */
#if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 199309L)
#   error clock_gettime needs _POSIX_C_SOURCE >= 199309L
#endif

#define NSEC_PER_SECOND         1000000000ull
#define TIMESPEC_TO_USEC(tspec) (((uint64_t)(tspec).tv_sec * NSEC_PER_SECOND) + (tspec).tv_nsec)

uint64_t myhtml_hperf_res(void)
{
    struct timespec tspec;
    int error = clock_getres(CLOCK_PROCESS_CPUTIME_ID, &tspec);
    if (error) {
        /* TODO: error checking */
        return 0;
    }

    unsigned long long ticks_per_sec = (unsigned long long)((double)NSEC_PER_SECOND / TIMESPEC_TO_USEC(tspec));
    return ticks_per_sec;
}

uint64_t myhtml_hperf_clock(void)
{
    struct timespec tspec;
    int error = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tspec);
    if (error) {
        /* TODO: error checking */
        return 0;
    }

    return TIMESPEC_TO_USEC(tspec);
}

#else 

/* 
 * TODO: on OSX we can use clock_get_time: http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x 
 * Or this: http://web.archive.org/web/20100517095152/http://www.wand.net.nz/~smr26/wordpress/2009/01/19/monotonic-time-in-mac-os-x/comment-page-1/
 */
#   error No hperf implementation for this platform

#endif

// 2... is fixed cpu 
void myhtml_hperf_print(const char *name, uint64_t x, uint64_t y) {
    unsigned long long freq = myhtml_hperf_res();
    printf("%s: %0.5f\n", name, (((float)(y - x) / (float)freq)));
    //printf("%s: %0.5f\n", name, (0.001f * ((float)(y - x) / 2000000.0f)));
}

void myhtml_hperf_print_by_val(const char *name, uint64_t x) {
    unsigned long long freq = myhtml_hperf_res();
    printf("%s: %0.5f\n", name, ((float)x / (float)freq));
}

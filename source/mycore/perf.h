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

#ifndef MyCORE_PERF_H
#define MyCORE_PERF_H
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "mycore/myosi.h"

#ifdef MyCORE_WITH_PERF
void * mycore_perf_create(void);
void mycore_perf_clean(void* perf);
void mycore_perf_destroy(void* perf);

mycore_status_t myhtml_perf_begin(void* perf);
mycore_status_t myhtml_perf_end(void* perf);
double myhtml_perf_in_sec(void* perf);

unsigned long long mycore_perf_clock(void);
unsigned long long mycore_perf_frequency(void);
#endif /* MyCORE_WITH_PERF */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyCORE_PERF_H */

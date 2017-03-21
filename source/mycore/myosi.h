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

#ifndef MyCORE_MYOSI_H
#define MyCORE_MYOSI_H
#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <inttypes.h>

#define MyCORE_STR_HELPER(x) #x
#define MyCORE_STR(x) MyCORE_STR_HELPER(x)

#define MyCORE_VERSION_MAJOR 0
#define MyCORE_VERSION_MINOR 1
#define MyCORE_VERSION_PATCH 0

#define MyCORE_VERSION_STRING MyCORE_STR(MyCORE_VERSION_MAJOR) MyCORE_STR(.) MyCORE_STR(MyCORE_VERSION_MINOR) MyCORE_STR(.) MyCORE_STR(MyCORE_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif

/* Format */
#ifdef MyCORE_OS_WINDOWS_NT
#define MyCORE_FORMAT_Z "%Iu"
#else
#define MyCORE_FORMAT_Z "%zu"
#endif

#if defined(_MSC_VER)
#  define MyCORE_DEPRECATED(func) __declspec(deprecated) func
#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
#  define MyCORE_DEPRECATED(func) func __attribute__((deprecated))
#else
#  define MyCORE_DEPRECATED(func) func
#endif

/* Debug */
#define MyCORE_DEBUG(...) do {} while(0)
#define MyCORE_DEBUG_ERROR(...) do {} while(0)

#define MyCORE_FAILED(_status_) ((_status_) != MyCORE_STATUS_OK)

// base
/*
 Very important!!!
 
 for mycore             0..008fff;      MyCORE_STATUS_OK      == 0x000000
 for myhtml             009000..0093ff; MyHTML_STATUS_OK      == 0x000000
 for myencoding         009400..00942f; MyENCODING_STATUS_OK  == 0x000000
 not occupied           009430..00ffff;
 for mycss and modules  010000..01ffff; MyCSS_STATUS_OK       == 0x000000
 for modest             020000..02ffff; MODEST_STATUS_OK      == 0x000000
 for myrender           030000..037777; MyRENDER_STATUS_OK    == 0x000000
 for myurl              038888..0388ff; MyURL_STATUS_OK       == 0x000000
 for myunicode          038900..038bff; MyUNICODE_STATUS_OK   == 0x000000
 not occupied           038c00..03ffff;
 for mydom              040000..04ffff; MyDOM_STATUS_OK       == 0x000000
 for mynetwork          050000..05ffff; MyNETWORK_STATUS_OK   == 0x000000
 for myecma             060000..06ffff; MyECMA_STATUS_OK      == 0x000000
 not occupied           070000..
*/
enum mycore_status {
    MyCORE_STATUS_OK                                   = 0x0000,
    MyCORE_STATUS_ERROR                                = 0x0001,
    MyCORE_STATUS_ERROR_MEMORY_ALLOCATION              = 0x0002,
    MyCORE_STATUS_THREAD_ERROR_MEMORY_ALLOCATION       = 0x0009,
    MyCORE_STATUS_THREAD_ERROR_LIST_INIT               = 0x000a,
    MyCORE_STATUS_THREAD_ERROR_ATTR_MALLOC             = 0x000b,
    MyCORE_STATUS_THREAD_ERROR_ATTR_INIT               = 0x000c,
    MyCORE_STATUS_THREAD_ERROR_ATTR_SET                = 0x000d,
    MyCORE_STATUS_THREAD_ERROR_ATTR_DESTROY            = 0x000e,
    MyCORE_STATUS_THREAD_ERROR_NO_SLOTS                = 0x000f,
    MyCORE_STATUS_THREAD_ERROR_BATCH_INIT              = 0x0010,
    MyCORE_STATUS_THREAD_ERROR_WORKER_MALLOC           = 0x0011,
    MyCORE_STATUS_THREAD_ERROR_WORKER_SEM_CREATE       = 0x0012,
    MyCORE_STATUS_THREAD_ERROR_WORKER_THREAD_CREATE    = 0x0013,
    MyCORE_STATUS_THREAD_ERROR_MASTER_THREAD_CREATE    = 0x0014,
    MyCORE_STATUS_THREAD_ERROR_SEM_PREFIX_MALLOC       = 0x0032,
    MyCORE_STATUS_THREAD_ERROR_SEM_CREATE              = 0x0033,
    MyCORE_STATUS_THREAD_ERROR_QUEUE_MALLOC            = 0x003c,
    MyCORE_STATUS_THREAD_ERROR_QUEUE_NODES_MALLOC      = 0x003d,
    MyCORE_STATUS_THREAD_ERROR_QUEUE_NODE_MALLOC       = 0x003e,
    MyCORE_STATUS_THREAD_ERROR_MUTEX_MALLOC            = 0x0046,
    MyCORE_STATUS_THREAD_ERROR_MUTEX_INIT              = 0x0047,
    MyCORE_STATUS_THREAD_ERROR_MUTEX_LOCK              = 0x0048,
    MyCORE_STATUS_THREAD_ERROR_MUTEX_UNLOCK            = 0x0049,
    MyCORE_STATUS_PERF_ERROR_COMPILED_WITHOUT_PERF     = 0x0050,
    MyCORE_STATUS_PERF_ERROR_FIND_CPU_CLOCK            = 0x0051,
    MyCORE_STATUS_MCOBJECT_ERROR_CACHE_CREATE          = 0x0055,
    MyCORE_STATUS_MCOBJECT_ERROR_CHUNK_CREATE          = 0x0056,
    MyCORE_STATUS_MCOBJECT_ERROR_CHUNK_INIT            = 0x0057,
    MyCORE_STATUS_MCOBJECT_ERROR_CACHE_REALLOC         = 0x0058,
    MyCORE_STATUS_ASYNC_ERROR_LOCK                     = 0x0060,
    MyCORE_STATUS_ASYNC_ERROR_UNLOCK                   = 0x0061,
    MyCORE_STATUS_ERROR_NO_FREE_SLOT                   = 0x0062,
}
typedef mycore_status_t;

typedef unsigned int mystatus_t;

/* thread */
typedef struct mythread_queue_list_entry mythread_queue_list_entry_t;
typedef struct mythread_queue_thread_param mythread_queue_thread_param_t;
typedef struct mythread_queue_list mythread_queue_list_t;
typedef struct mythread_queue_node mythread_queue_node_t;
typedef struct mythread_queue mythread_queue_t;

typedef size_t mythread_id_t;
typedef struct mythread_context mythread_context_t;
typedef struct mythread_entry mythread_entry_t;
typedef struct mythread mythread_t;

/* mystring */
typedef struct mycore_string_raw mycore_string_raw_t;
typedef struct mycore_string mycore_string_t;

/* incoming buffer */
typedef struct mycore_incoming_buffer mycore_incoming_buffer_t;

/* callbacks */
typedef mystatus_t (*mycore_callback_serialize_f)(const char* buffer, size_t size, void* ctx);

void * mycore_malloc(size_t size);
void * mycore_realloc(void* dst, size_t size);
void * mycore_calloc(size_t num, size_t size);
void * mycore_free(void* dst);

/* io */
FILE * mycore_fopen(const char *filename, const char *mode);
int mycore_fclose(FILE *stream);
    
size_t mycore_fread(void *buffer, size_t size, size_t count, FILE *stream);
size_t mycore_fwrite(const void *buffer, size_t size, size_t count, FILE *stream);
int mycore_fflush(FILE *stream);
int mycore_fseek(FILE* stream, long offset, int origin);
long mycore_ftell(FILE* stream);
int mycore_ferror(FILE *stream);
    
void mycore_setbuf(FILE *stream, char *buffer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyCORE_MYOSI_H */


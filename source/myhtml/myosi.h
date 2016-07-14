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

#ifndef MyHTML_MYOSI_H
#define MyHTML_MYOSI_H
#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <stdarg.h>

#define MyHTML_VERSION_MAJOR 1
#define MyHTML_VERSION_MINOR 0
#define MyHTML_VERSION_PATCH 2

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__WINPTHREADS_VERSION)
#define IS_OS_WINDOWS
#include <windows.h>
#endif

#if defined(_MSC_VER)
#  define MyHTML_DEPRECATED(func, message) __declspec(deprecated(message)) func
#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
#  define MyHTML_DEPRECATED(func, message) func __attribute__((deprecated(message)))
#else
#  define MyHTML_DEPRECATED(func, message) func
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Mem */
#ifdef MyHTML_EXTERN_MALLOC
    extern void * MyHTML_EXTERN_MALLOC(size_t size);
    #define myhtml_malloc MyHTML_EXTERN_MALLOC
#else
    #define myhtml_malloc myhtml_mem_malloc
#endif

#ifdef MyHTML_EXTERN_REALLOC
    extern void * MyHTML_EXTERN_REALLOC(void* dst, size_t size);
    #define myhtml_realloc MyHTML_EXTERN_REALLOC
#else
    #define myhtml_realloc myhtml_mem_realloc
#endif

#ifdef MyHTML_EXTERN_CALLOC
    extern void * MyHTML_EXTERN_CALLOC(size_t num, size_t size);
    #define myhtml_calloc MyHTML_EXTERN_CALLOC
#else
    #define myhtml_calloc myhtml_mem_calloc
#endif

#ifdef MyHTML_EXTERN_FREE
    extern void MyHTML_EXTERN_FREE(void* dst);
    #define myhtml_free MyHTML_EXTERN_FREE
#else
    #define myhtml_free myhtml_mem_free
#endif

/* Debug */
#ifdef DEBUG_MODE
    #define MyHTML_DEBUG(format, ...)      \
        myhtml_print(stderr, "DEBUG: "format"\n", ##__VA_ARGS__)
#else
    #define MyHTML_DEBUG(format, ...)
#endif

#ifdef DEBUG_MODE
    #define MyHTML_DEBUG_ERROR(format, ...)      \
        myhtml_print(stderr, "DEBUG ERROR: "format"\n", ##__VA_ARGS__)
#else
    #define MyHTML_DEBUG_ERROR(format, ...)
#endif

#define myhtml_base_add(myhtml, point, lenn, sizen, strcn, size)    \
    myhtml->Lenn++;                                                 \
    if(myhtml->lenn == myhtml->sizen) {                             \
        myhtml->sizen += size;                                      \
        myhtml->point = (strcn*)myhtml_realloc(myhtml->point,            \
            sizeof(strcn) * myhtml->sizen);                         \
    }


// encoding
// https://encoding.spec.whatwg.org/#the-encoding
// https://encoding.spec.whatwg.org/#legacy-single-byte-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-chinese-(simplified)-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-chinese-(traditional)-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-japanese-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-korean-encodings
// https://encoding.spec.whatwg.org/#legacy-miscellaneous-encodings

enum myhtml_encoding_list {
    MyHTML_ENCODING_DEFAULT          = 0x00,
//  MyHTML_ENCODING_AUTO             = 0x01, // future
//  MyHTML_ENCODING_CUSTOM           = 0x02, // future
    MyHTML_ENCODING_UTF_8            = 0x00, // default encoding
    MyHTML_ENCODING_UTF_16LE         = 0x04,
    MyHTML_ENCODING_UTF_16BE         = 0x05,
    MyHTML_ENCODING_X_USER_DEFINED   = 0x06,
    MyHTML_ENCODING_BIG5             = 0x07,
    MyHTML_ENCODING_EUC_KR           = 0x08,
    MyHTML_ENCODING_GB18030          = 0x09,
    MyHTML_ENCODING_IBM866           = 0x0a,
    MyHTML_ENCODING_ISO_8859_10      = 0x0b,
    MyHTML_ENCODING_ISO_8859_13      = 0x0c,
    MyHTML_ENCODING_ISO_8859_14      = 0x0d,
    MyHTML_ENCODING_ISO_8859_15      = 0x0e,
    MyHTML_ENCODING_ISO_8859_16      = 0x0f,
    MyHTML_ENCODING_ISO_8859_2       = 0x10,
    MyHTML_ENCODING_ISO_8859_3       = 0x11,
    MyHTML_ENCODING_ISO_8859_4       = 0x12,
    MyHTML_ENCODING_ISO_8859_5       = 0x13,
    MyHTML_ENCODING_ISO_8859_6       = 0x14,
    MyHTML_ENCODING_ISO_8859_7       = 0x15,
    MyHTML_ENCODING_ISO_8859_8       = 0x16,
    MyHTML_ENCODING_KOI8_R           = 0x17,
    MyHTML_ENCODING_KOI8_U           = 0x18,
    MyHTML_ENCODING_MACINTOSH        = 0x19,
    MyHTML_ENCODING_WINDOWS_1250     = 0x1a,
    MyHTML_ENCODING_WINDOWS_1251     = 0x1b,
    MyHTML_ENCODING_WINDOWS_1252     = 0x1c,
    MyHTML_ENCODING_WINDOWS_1253     = 0x1d,
    MyHTML_ENCODING_WINDOWS_1254     = 0x1e,
    MyHTML_ENCODING_WINDOWS_1255     = 0x1f,
    MyHTML_ENCODING_WINDOWS_1256     = 0x20,
    MyHTML_ENCODING_WINDOWS_1257     = 0x21,
    MyHTML_ENCODING_WINDOWS_1258     = 0x22,
    MyHTML_ENCODING_WINDOWS_874      = 0x23,
    MyHTML_ENCODING_X_MAC_CYRILLIC   = 0x24,
    MyHTML_ENCODING_ISO_2022_JP      = 0x25,
    MyHTML_ENCODING_GBK              = 0x26,
    MyHTML_ENCODING_SHIFT_JIS        = 0x27,
    MyHTML_ENCODING_EUC_JP           = 0x28,
    MyHTML_ENCODING_ISO_8859_8_I     = 0x29,
    MyHTML_ENCODING_LAST_ENTRY       = 0x2a
}
typedef myhtml_encoding_t;

// char references
typedef struct myhtml_data_process_entry myhtml_data_process_entry_t;

// strings
typedef struct myhtml_string myhtml_string_t;

// thread
enum mythread_thread_opt {
    MyTHREAD_OPT_UNDEF = 0x00,
    MyTHREAD_OPT_WAIT  = 0x01,
    MyTHREAD_OPT_QUIT  = 0x02,
    MyTHREAD_OPT_STOP  = 0x04
}
typedef mythread_thread_opt_t;

typedef struct mythread_queue_list_entry mythread_queue_list_entry_t;
typedef struct mythread_queue_thread_param mythread_queue_thread_param_t;
typedef struct mythread_queue_list mythread_queue_list_t;
typedef struct mythread_queue_node mythread_queue_node_t;
typedef struct mythread_queue mythread_queue_t;

typedef size_t mythread_id_t;
typedef struct mythread_workers_list mythread_workers_list_t;
typedef struct mythread_context mythread_context_t;
typedef struct mythread_list mythread_list_t;
typedef struct mythread mythread_t;

// tree
enum myhtml_tree_flags {
    MyHTML_TREE_FLAGS_CLEAN                   = 0x000,
    MyHTML_TREE_FLAGS_SCRIPT                  = 0x001,
    MyHTML_TREE_FLAGS_FRAMESET_OK             = 0x002,
    MyHTML_TREE_FLAGS_IFRAME_SRCDOC           = 0x004,
    MyHTML_TREE_FLAGS_ALREADY_STARTED         = 0x008,
    MyHTML_TREE_FLAGS_SINGLE_MODE             = 0x010,
    MyHTML_TREE_FLAGS_PARSE_END               = 0x020,
    MyHTML_TREE_FLAGS_PARSE_FLAG              = 0x040,
    MyHTML_TREE_FLAGS_PARSE_FLAG_EMIT_NEWLINE = 0x080
};

enum myhtml_tree_parse_flags {
    MyHTML_TREE_PARSE_FLAGS_CLEAN                   = 0x000,
    MyHTML_TREE_PARSE_FLAGS_WITHOUT_BUILD_TREE      = 0x001,
    MyHTML_TREE_PARSE_FLAGS_WITHOUT_PROCESS_TOKEN   = 0x003,
    MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN   = 0x004,
    MyHTML_TREE_PARSE_FLAGS_WITHOUT_DOCTYPE_IN_TREE = 0x008,
}
typedef myhtml_tree_parse_flags_t;

typedef struct myhtml_tree_temp_tag_name myhtml_tree_temp_tag_name_t;
typedef struct myhtml_tree_insertion_list myhtml_tree_insertion_list_t;
typedef struct myhtml_tree_token_list myhtml_tree_token_list_t;
typedef struct myhtml_tree_list myhtml_tree_list_t;
typedef struct myhtml_tree_indexes myhtml_tree_indexes_t;
typedef struct myhtml_tree_doctype myhtml_tree_doctype_t;
typedef struct myhtml_async_args myhtml_async_args_t;

typedef struct myhtml_tree_node myhtml_tree_node_t;
typedef struct myhtml_tree myhtml_tree_t;

// token
enum myhtml_token_type {
    MyHTML_TOKEN_TYPE_OPEN             = 0x000,
    MyHTML_TOKEN_TYPE_CLOSE            = 0x001,
    MyHTML_TOKEN_TYPE_CLOSE_SELF       = 0x002,
    MyHTML_TOKEN_TYPE_DONE             = 0x004,
    MyHTML_TOKEN_TYPE_WHITESPACE       = 0x008,
    MyHTML_TOKEN_TYPE_RCDATA           = 0x010,
    MyHTML_TOKEN_TYPE_RAWTEXT          = 0x020,
    MyHTML_TOKEN_TYPE_SCRIPT           = 0x040,
    MyHTML_TOKEN_TYPE_PLAINTEXT        = 0x080,
    MyHTML_TOKEN_TYPE_CDATA            = 0x100,
    MyHTML_TOKEN_TYPE_DATA             = 0x200,
    MyHTML_TOKEN_TYPE_COMMENT          = 0x400,
    MyHTML_TOKEN_TYPE_NULL             = 0x800
};

typedef size_t myhtml_token_index_t;
typedef size_t myhtml_token_attr_index_t;
typedef struct myhtml_token_replacement_entry myhtml_token_replacement_entry_t;
typedef struct myhtml_token_namespace_replacement myhtml_token_namespace_replacement_t;

typedef struct myhtml_token_attr myhtml_token_attr_t;
typedef struct myhtml_token_node myhtml_token_node_t;
typedef struct myhtml_token myhtml_token_t;

// tags
enum myhtml_tag_categories {
    MyHTML_TAG_CATEGORIES_UNDEF            = 0x000,
    MyHTML_TAG_CATEGORIES_ORDINARY         = 0x001,
    MyHTML_TAG_CATEGORIES_SPECIAL          = 0x002,
    MyHTML_TAG_CATEGORIES_FORMATTING       = 0x004,
    MyHTML_TAG_CATEGORIES_SCOPE            = 0x008,
    MyHTML_TAG_CATEGORIES_SCOPE_LIST_ITEM  = 0x010,
    MyHTML_TAG_CATEGORIES_SCOPE_BUTTON     = 0x020,
    MyHTML_TAG_CATEGORIES_SCOPE_TABLE      = 0x040,
    MyHTML_TAG_CATEGORIES_SCOPE_SELECT     = 0x080
};

typedef struct myhtml_tag_index_node myhtml_tag_index_node_t;
typedef struct myhtml_tag_index_entry myhtml_tag_index_entry_t;
typedef struct myhtml_tag_index myhtml_tag_index_t;

typedef size_t myhtml_tag_id_t;

typedef struct myhtml_tag myhtml_tag_t;

// stream
typedef struct myhtml_stream_buffer_entry myhtml_stream_buffer_entry_t;
typedef struct myhtml_stream_buffer myhtml_stream_buffer_t;

// parse
enum myhtml_tokenizer_state {
    MyHTML_TOKENIZER_STATE_DATA                                          = 0x000,
    MyHTML_TOKENIZER_STATE_CHARACTER_REFERENCE_IN_DATA                   = 0x001,
    MyHTML_TOKENIZER_STATE_RCDATA                                        = 0x002,
    MyHTML_TOKENIZER_STATE_CHARACTER_REFERENCE_IN_RCDATA                 = 0x003,
    MyHTML_TOKENIZER_STATE_RAWTEXT                                       = 0x004,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA                                   = 0x005,
    MyHTML_TOKENIZER_STATE_PLAINTEXT                                     = 0x006,
    MyHTML_TOKENIZER_STATE_TAG_OPEN                                      = 0x007,
    MyHTML_TOKENIZER_STATE_END_TAG_OPEN                                  = 0x008,
    MyHTML_TOKENIZER_STATE_TAG_NAME                                      = 0x009,
    MyHTML_TOKENIZER_STATE_RCDATA_LESS_THAN_SIGN                         = 0x00a,
    MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_OPEN                           = 0x00b,
    MyHTML_TOKENIZER_STATE_RCDATA_END_TAG_NAME                           = 0x00c,
    MyHTML_TOKENIZER_STATE_RAWTEXT_LESS_THAN_SIGN                        = 0x00d,
    MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_OPEN                          = 0x00e,
    MyHTML_TOKENIZER_STATE_RAWTEXT_END_TAG_NAME                          = 0x00f,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_LESS_THAN_SIGN                    = 0x010,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_OPEN                      = 0x011,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_END_TAG_NAME                      = 0x012,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPE_START                      = 0x013,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPE_START_DASH                 = 0x014,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED                           = 0x015,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH                      = 0x016,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_DASH_DASH                 = 0x017,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN            = 0x018,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_OPEN              = 0x019,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_ESCAPED_END_TAG_NAME              = 0x01a,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_START               = 0x01b,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED                    = 0x01c,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH               = 0x01d,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH_DASH          = 0x01e,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN     = 0x01f,
    MyHTML_TOKENIZER_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_END                 = 0x020,
    MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_NAME                         = 0x021,
    MyHTML_TOKENIZER_STATE_ATTRIBUTE_NAME                                = 0x022,
    MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_NAME                          = 0x023,
    MyHTML_TOKENIZER_STATE_BEFORE_ATTRIBUTE_VALUE                        = 0x024,
    MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED                 = 0x025,
    MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED                 = 0x026,
    MyHTML_TOKENIZER_STATE_ATTRIBUTE_VALUE_UNQUOTED                      = 0x027,
    MyHTML_TOKENIZER_STATE_CHARACTER_REFERENCE_IN_ATTRIBUTE_VALUE        = 0x028,
    MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_VALUE__QUOTED                 = 0x029,
    MyHTML_TOKENIZER_STATE_SELF_CLOSING_START_TAG                        = 0x02a,
    MyHTML_TOKENIZER_STATE_BOGUS_COMMENT                                 = 0x02b,
    MyHTML_TOKENIZER_STATE_MARKUP_DECLARATION_OPEN                       = 0x02c,
    MyHTML_TOKENIZER_STATE_COMMENT_START                                 = 0x02d,
    MyHTML_TOKENIZER_STATE_COMMENT_START_DASH                            = 0x02e,
    MyHTML_TOKENIZER_STATE_COMMENT                                       = 0x02f,
    MyHTML_TOKENIZER_STATE_COMMENT_END_DASH                              = 0x030,
    MyHTML_TOKENIZER_STATE_COMMENT_END                                   = 0x031,
    MyHTML_TOKENIZER_STATE_COMMENT_END_BANG                              = 0x032,
    MyHTML_TOKENIZER_STATE_DOCTYPE                                       = 0x033,
    MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_NAME                           = 0x034,
    MyHTML_TOKENIZER_STATE_DOCTYPE_NAME                                  = 0x035,
    MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_NAME                            = 0x036,
    MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_PUBLIC_KEYWORD                  = 0x037,
    MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_PUBLIC_IDENTIFIER              = 0x038,
    MyHTML_TOKENIZER_STATE_DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED       = 0x039,
    MyHTML_TOKENIZER_STATE_DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED       = 0x03a,
    MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_PUBLIC_IDENTIFIER               = 0x03b,
    MyHTML_TOKENIZER_STATE_BETWEEN_DOCTYPE_PUBLIC_AND_SYSTEM_IDENTIFIERS = 0x03c,
    MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_SYSTEM_KEYWORD                  = 0x03d,
    MyHTML_TOKENIZER_STATE_BEFORE_DOCTYPE_SYSTEM_IDENTIFIER              = 0x03e,
    MyHTML_TOKENIZER_STATE_DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED       = 0x03f,
    MyHTML_TOKENIZER_STATE_DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED       = 0x040,
    MyHTML_TOKENIZER_STATE_AFTER_DOCTYPE_SYSTEM_IDENTIFIER               = 0x041,
    MyHTML_TOKENIZER_STATE_BOGUS_DOCTYPE                                 = 0x042,
    MyHTML_TOKENIZER_STATE_CDATA_SECTION                                 = 0x043,
    MyHTML_TOKENIZER_STATE_CUSTOM_AFTER_DOCTYPE_NAME_A_Z                 = 0x044,
    MyHTML_TOKENIZER_STATE_FIRST_ENTRY                                   = MyHTML_TOKENIZER_STATE_DATA,
    MyHTML_TOKENIZER_STATE_LAST_ENTRY                                    = 0x045
};

enum myhtml_insertion_mode {
    MyHTML_INSERTION_MODE_INITIAL              = 0x000,
    MyHTML_INSERTION_MODE_BEFORE_HTML          = 0x001,
    MyHTML_INSERTION_MODE_BEFORE_HEAD          = 0x002,
    MyHTML_INSERTION_MODE_IN_HEAD              = 0x003,
    MyHTML_INSERTION_MODE_IN_HEAD_NOSCRIPT     = 0x004,
    MyHTML_INSERTION_MODE_AFTER_HEAD           = 0x005,
    MyHTML_INSERTION_MODE_IN_BODY              = 0x006,
    MyHTML_INSERTION_MODE_TEXT                 = 0x007,
    MyHTML_INSERTION_MODE_IN_TABLE             = 0x008,
    MyHTML_INSERTION_MODE_IN_TABLE_TEXT        = 0x009,
    MyHTML_INSERTION_MODE_IN_CAPTION           = 0x00a,
    MyHTML_INSERTION_MODE_IN_COLUMN_GROUP      = 0x00b,
    MyHTML_INSERTION_MODE_IN_TABLE_BODY        = 0x00c,
    MyHTML_INSERTION_MODE_IN_ROW               = 0x00d,
    MyHTML_INSERTION_MODE_IN_CELL              = 0x00e,
    MyHTML_INSERTION_MODE_IN_SELECT            = 0x00f,
    MyHTML_INSERTION_MODE_IN_SELECT_IN_TABLE   = 0x010,
    MyHTML_INSERTION_MODE_IN_TEMPLATE          = 0x011,
    MyHTML_INSERTION_MODE_AFTER_BODY           = 0x012,
    MyHTML_INSERTION_MODE_IN_FRAMESET          = 0x013,
    MyHTML_INSERTION_MODE_AFTER_FRAMESET       = 0x014,
    MyHTML_INSERTION_MODE_AFTER_AFTER_BODY     = 0x015,
    MyHTML_INSERTION_MODE_AFTER_AFTER_FRAMESET = 0x016,
    MyHTML_INSERTION_MODE_LAST_ENTRY           = 0x017
};

// base
/*
 Very important!!!
 
 for myhtml             0..00ffff;      MyHTML_STATUS_OK    == 0x000000
 for mycss and modules  010000..01ffff; MyCSS_STATUS_OK     == 0x000000
 for myrender           020000..02ffff; MyRENDER_STATUS_OK  == 0x000000
 for mydom              030000..03ffff; MyDOM_STATUS_OK     == 0x000000
 for mynetwork          040000..04ffff; MyNETWORK_STATUS_OK == 0x000000
 for myecma             050000..05ffff; MyECMA_STATUS_OK    == 0x000000
 not occupied           060000..
*/
enum myhtml_status {
    MyHTML_STATUS_OK                                   = 0x0000,
    MyHTML_STATUS_ERROR_MEMORY_ALLOCATION              = 0x0001,
    MyHTML_STATUS_THREAD_ERROR_LIST_INIT               = 0x000a,
    MyHTML_STATUS_THREAD_ERROR_ATTR_MALLOC             = 0x000b,
    MyHTML_STATUS_THREAD_ERROR_ATTR_INIT               = 0x000c,
    MyHTML_STATUS_THREAD_ERROR_ATTR_SET                = 0x000d,
    MyHTML_STATUS_THREAD_ERROR_ATTR_DESTROY            = 0x000e,
    MyHTML_STATUS_THREAD_ERROR_NO_SLOTS                = 0x000f,
    MyHTML_STATUS_THREAD_ERROR_BATCH_INIT              = 0x0010,
    MyHTML_STATUS_THREAD_ERROR_WORKER_MALLOC           = 0x0011,
    MyHTML_STATUS_THREAD_ERROR_WORKER_SEM_CREATE       = 0x0012,
    MyHTML_STATUS_THREAD_ERROR_WORKER_THREAD_CREATE    = 0x0013,
    MyHTML_STATUS_THREAD_ERROR_MASTER_THREAD_CREATE    = 0x0014,
    MyHTML_STATUS_THREAD_ERROR_SEM_PREFIX_MALLOC       = 0x0032,
    MyHTML_STATUS_THREAD_ERROR_SEM_CREATE              = 0x0033,
    MyHTML_STATUS_THREAD_ERROR_QUEUE_MALLOC            = 0x003c,
    MyHTML_STATUS_THREAD_ERROR_QUEUE_NODES_MALLOC      = 0x003d,
    MyHTML_STATUS_THREAD_ERROR_QUEUE_NODE_MALLOC       = 0x003e,
    MyHTML_STATUS_THREAD_ERROR_MUTEX_MALLOC            = 0x0046,
    MyHTML_STATUS_THREAD_ERROR_MUTEX_INIT              = 0x0047,
    MyHTML_STATUS_THREAD_ERROR_MUTEX_LOCK              = 0x0048,
    MyHTML_STATUS_THREAD_ERROR_MUTEX_UNLOCK            = 0x0049,
    MyHTML_STATUS_RULES_ERROR_MEMORY_ALLOCATION        = 0x0064,
    MyHTML_STATUS_PERF_ERROR_COMPILED_WITHOUT_PERF     = 0x00c8,
    MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK            = 0x00c9,
    MyHTML_STATUS_TOKENIZER_ERROR_MEMORY_ALLOCATION    = 0x012c,
    MyHTML_STATUS_TOKENIZER_ERROR_FRAGMENT_INIT        = 0x012d,
    MyHTML_STATUS_TAGS_ERROR_MEMORY_ALLOCATION         = 0x0190,
    MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE           = 0x0191,
    MyHTML_STATUS_TAGS_ERROR_MCOBJECT_MALLOC           = 0x0192,
    MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE_NODE      = 0x0193,
    MyHTML_STATUS_TAGS_ERROR_CACHE_MEMORY_ALLOCATION   = 0x0194,
    MyHTML_STATUS_TAGS_ERROR_INDEX_MEMORY_ALLOCATION   = 0x0195,
    MyHTML_STATUS_TREE_ERROR_MEMORY_ALLOCATION         = 0x01f4,
    MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE           = 0x01f5,
    MyHTML_STATUS_TREE_ERROR_MCOBJECT_INIT             = 0x01f6,
    MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE_NODE      = 0x01f7,
    MyHTML_STATUS_TREE_ERROR_INCOMING_BUFFER_CREATE    = 0x01f8,
    MyHTML_STATUS_ATTR_ERROR_ALLOCATION                = 0x0258,
    MyHTML_STATUS_ATTR_ERROR_CREATE                    = 0x0259,
    MyHTML_STATUS_STREAM_BUFFER_ERROR_CREATE           = 0x0300,
    MyHTML_STATUS_STREAM_BUFFER_ERROR_INIT             = 0x0301,
    MyHTML_STATUS_STREAM_BUFFER_ENTRY_ERROR_CREATE     = 0x0302,
    MyHTML_STATUS_STREAM_BUFFER_ENTRY_ERROR_INIT       = 0x0303,
    MyHTML_STATUS_STREAM_BUFFER_ERROR_ADD_ENTRY        = 0x0304,
    MyHTML_STATUS_MCOBJECT_ERROR_CACHE_CREATE          = 0x0340,
    MyHTML_STATUS_MCOBJECT_ERROR_CHUNK_CREATE          = 0x0341,
    MyHTML_STATUS_MCOBJECT_ERROR_CHUNK_INIT            = 0x0342,
    MyHTML_STATUS_MCOBJECT_ERROR_CACHE_REALLOC         = 0x0343
}
typedef myhtml_status_t;

enum myhtml_namespace {
    MyHTML_NAMESPACE_UNDEF      = 0x00,
    MyHTML_NAMESPACE_HTML       = 0x01,
    MyHTML_NAMESPACE_MATHML     = 0x02,
    MyHTML_NAMESPACE_SVG        = 0x03,
    MyHTML_NAMESPACE_XLINK      = 0x04,
    MyHTML_NAMESPACE_XML        = 0x05,
    MyHTML_NAMESPACE_XMLNS      = 0x06,
    MyHTML_NAMESPACE_LAST_ENTRY = 0x07
}
typedef myhtml_namespace_t;

enum myhtml_options {
    MyHTML_OPTIONS_DEFAULT                 = 0x00,
    MyHTML_OPTIONS_PARSE_MODE_SINGLE       = 0x01,
    MyHTML_OPTIONS_PARSE_MODE_ALL_IN_ONE   = 0x02,
    MyHTML_OPTIONS_PARSE_MODE_SEPARATELY   = 0x04
};

struct myhtml_position {
    size_t begin;
    size_t length;
}
typedef myhtml_position_t;

struct myhtml_version {
    int major;
    int minor;
    int patch;
}
typedef myhtml_version_t;

typedef struct myhtml_incoming_buffer myhtml_incoming_buffer_t;
typedef myhtml_token_attr_t myhtml_tree_attr_t;
typedef struct myhtml_collection myhtml_collection_t;
typedef struct myhtml myhtml_t;

// parser state function
typedef size_t (*myhtml_tokenizer_state_f)(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);

// parser stream function
typedef void (*mythread_f)(mythread_id_t thread_id, mythread_queue_node_t *qnode);

// parser insertion mode function
typedef bool (*myhtml_insertion_f)(myhtml_tree_t* tree, myhtml_token_node_t* token);

// char references state
typedef size_t (*myhtml_data_process_state_f)(myhtml_data_process_entry_t* charef, myhtml_string_t* str, const char* data, size_t offset, size_t size);

// callback functions
typedef void* (*myhtml_callback_token_f)(myhtml_tree_t* tree, myhtml_token_node_t* token, void* ctx);
typedef void (*myhtml_callback_tree_node_f)(myhtml_tree_t* tree, myhtml_tree_node_t* node, void* ctx);

// find attribute value functions
typedef bool (*myhtml_attribute_value_find_f)(myhtml_string_t* str_key, const char* value, size_t value_len);


void * myhtml_mem_malloc(size_t size);
void * myhtml_mem_realloc(void* dst, size_t size);
void * myhtml_mem_calloc(size_t num, size_t size);
void   myhtml_mem_free(void* dst);

void myhtml_print(FILE* out, const char* format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif


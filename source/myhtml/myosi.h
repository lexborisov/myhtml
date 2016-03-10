/*
 Copyright 2015-2016 Alexander Borisov
 
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

#ifndef MyHTML_MYOSI_H
#define MyHTML_MYOSI_H
#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
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

#ifdef DEBUG_MODE
    #define MyHTML_DEBUG(__format__, ...)      \
        myhtml_print(stderr, "DEBUG: "__format__"\n", ##__VA_ARGS__)
#else
    #define MyHTML_DEBUG(__format__, ...)
#endif

#ifdef DEBUG_MODE
    #define MyHTML_DEBUG_ERROR(__format__, ...)      \
        myhtml_print(stderr, "DEBUG ERROR: "__format__"\n", ##__VA_ARGS__)
#else
    #define MyHTML_DEBUG_ERROR(__format__, ...)
#endif

#define myhtml_base_add(__myhtml__, __point__, __lenn__, __sizen__, __strcn__, __size__)    \
    __myhtml__->__lenn__++;                                                                 \
    if(__myhtml__->__lenn__ == __myhtml__->__sizen__) {                                     \
        __myhtml__->__sizen__ += __size__;                                                  \
        __myhtml__->__point__ = (__strcn__*)myrealloc(__myhtml__->__point__,                  \
            sizeof(__strcn__) * __myhtml__->__sizen__);                                     \
    }


typedef enum {
    myfalse = 0,
    mytrue  = 1
} MyHTML_DEPRECATED(mybool_t, "use bool");

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
    MyHTML_ENCODING_LAST_ENTRY       = 0x25
}
typedef myhtml_encoding_t;

// strings
typedef struct myhtml_string myhtml_string_t;

// thread
enum mythread_thread_opt {
    MyTHREAD_OPT_UNDEF = 0,
    MyTHREAD_OPT_WAIT  = 1,
    MyTHREAD_OPT_QUIT  = 2
}
typedef mythread_thread_opt_t;

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

typedef struct myhtml_tree_temp_stream myhtml_tree_temp_stream_t;
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
enum myhtml_status {
    MyHTML_STATUS_OK                                   = 0,
    MyHTML_STATUS_ERROR_MEMORY_ALLOCATION              = 1,
    MyHTML_STATUS_THREAD_ERROR_LIST_INIT               = 10,
    MyHTML_STATUS_THREAD_ERROR_ATTR_MALLOC             = 11,
    MyHTML_STATUS_THREAD_ERROR_ATTR_INIT               = 12,
    MyHTML_STATUS_THREAD_ERROR_ATTR_SET                = 13,
    MyHTML_STATUS_THREAD_ERROR_ATTR_DESTROY            = 14,
    MyHTML_STATUS_THREAD_ERROR_NO_SLOTS                = 15,
    MyHTML_STATUS_THREAD_ERROR_BATCH_INIT              = 16,
    MyHTML_STATUS_THREAD_ERROR_WORKER_MALLOC           = 17,
    MyHTML_STATUS_THREAD_ERROR_WORKER_SEM_CREATE       = 18,
    MyHTML_STATUS_THREAD_ERROR_WORKER_THREAD_CREATE    = 19,
    MyHTML_STATUS_THREAD_ERROR_MASTER_THREAD_CREATE    = 20,
    MyHTML_STATUS_THREAD_ERROR_SEM_PREFIX_MALLOC       = 50,
    MyHTML_STATUS_THREAD_ERROR_SEM_CREATE              = 51,
    MyHTML_STATUS_THREAD_ERROR_QUEUE_MALLOC            = 60,
    MyHTML_STATUS_THREAD_ERROR_QUEUE_NODES_MALLOC      = 61,
    MyHTML_STATUS_THREAD_ERROR_QUEUE_NODE_MALLOC       = 62,
    MyHTML_STATUS_RULES_ERROR_MEMORY_ALLOCATION        = 100,
    MyHTML_STATUS_PERF_ERROR_COMPILED_WITHOUT_PERF     = 200,
    MyHTML_STATUS_PERF_ERROR_FIND_CPU_CLOCK            = 201,
    MyHTML_STATUS_TOKENIZER_ERROR_MEMORY_ALLOCATION    = 300,
    MyHTML_STATUS_TAGS_ERROR_MEMORY_ALLOCATION         = 400,
    MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE           = 401,
    MyHTML_STATUS_TAGS_ERROR_MCOBJECT_MALLOC           = 402,
    MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE_NODE      = 403,
    MyHTML_STATUS_TAGS_ERROR_CACHE_MEMORY_ALLOCATION   = 404,
    MyHTML_STATUS_TAGS_ERROR_INDEX_MEMORY_ALLOCATION   = 405,
    MyHTML_STATUS_TREE_ERROR_MEMORY_ALLOCATION         = 500,
    MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE           = 501,
    MyHTML_STATUS_TREE_ERROR_MCOBJECT_INIT             = 502,
    MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE_NODE      = 503,
    MyHTML_STATUS_ATTR_ERROR_ALLOCATION                = 600,
    MyHTML_STATUS_ATTR_ERROR_CREATE                    = 601
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
    MyHTML_OPTIONS_PARSE_MODE_SEPARATELY   = 0x04,
    MyHTML_OPTIONS_PARSE_MODE_WORKER_TREE  = 0x08,
    MyHTML_OPTIONS_PARSE_MODE_WORKER_INDEX = 0x10,
    MyHTML_OPTIONS_PARSE_MODE_TREE_INDEX   = 0x20
};

typedef struct myhtml_incoming_buf myhtml_incoming_buf_t;
typedef myhtml_token_attr_t myhtml_tree_attr_t;
typedef struct myhtml_collection myhtml_collection_t;
typedef struct myhtml myhtml_t;

// parser state function
typedef size_t (*myhtml_tokenizer_state_f)(myhtml_tree_t* tree, mythread_queue_node_t* qnode, const char* html, size_t html_offset, size_t html_size);

// parser stream function
typedef void (*mythread_f)(mythread_id_t thread_id, mythread_queue_node_t *qnode);

// parser insertion mode function
typedef bool (*myhtml_insertion_f)(myhtml_tree_t* tree, myhtml_token_node_t* token);

void * mymalloc(size_t size);
void * myrealloc(void* dst, size_t size);
void * mycalloc(size_t num, size_t size);
void   myfree(void* dst);

void myhtml_print(FILE* out, const char* format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif


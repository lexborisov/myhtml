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

#ifndef MyHTML_MYOSI_H
#define MyHTML_MYOSI_H
#pragma once

#include <mycore/myosi.h>

#define MyHTML_VERSION_MAJOR 4
#define MyHTML_VERSION_MINOR 0
#define MyHTML_VERSION_PATCH 5

#define MyHTML_VERSION_STRING MyCORE_STR(MyHTML_VERSION_MAJOR) MyCORE_STR(.) MyCORE_STR(MyHTML_VERSION_MINOR) MyCORE_STR(.) MyCORE_STR(MyHTML_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif

#define MyHTML_FAILED(_status_) ((_status_) != MyHTML_STATUS_OK)

// char references
typedef struct myhtml_data_process_entry myhtml_data_process_entry_t;

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
    MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN   = 0x004, /* skip ws token, but not for RCDATA, RAWTEXT, CDATA and PLAINTEXT */
    MyHTML_TREE_PARSE_FLAGS_WITHOUT_DOCTYPE_IN_TREE = 0x008
}
typedef myhtml_tree_parse_flags_t;

typedef struct myhtml_tree_temp_tag_name myhtml_tree_temp_tag_name_t;
typedef struct myhtml_tree_insertion_list myhtml_tree_insertion_list_t;
typedef struct myhtml_tree_token_list myhtml_tree_token_list_t;
typedef struct myhtml_tree_list myhtml_tree_list_t;
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
    MyHTML_TOKENIZER_STATE_AFTER_ATTRIBUTE_VALUE_QUOTED                  = 0x029,
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
    MyHTML_TOKENIZER_STATE_PARSE_ERROR_STOP                              = 0x045,
    MyHTML_TOKENIZER_STATE_FIRST_ENTRY                                   = MyHTML_TOKENIZER_STATE_DATA,
    MyHTML_TOKENIZER_STATE_LAST_ENTRY                                    = 0x046
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
 See mycore/myosi.h:mystatus_t
*/
enum myhtml_status {
    MyHTML_STATUS_OK                                   = 0x0000,
    MyHTML_STATUS_ERROR                                = 0x0001,
    MyHTML_STATUS_ERROR_MEMORY_ALLOCATION              = 0x0002,
    MyHTML_STATUS_RULES_ERROR_MEMORY_ALLOCATION        = 0x9064,
    MyHTML_STATUS_TOKENIZER_ERROR_MEMORY_ALLOCATION    = 0x912c,
    MyHTML_STATUS_TOKENIZER_ERROR_FRAGMENT_INIT        = 0x912d,
    MyHTML_STATUS_TAGS_ERROR_MEMORY_ALLOCATION         = 0x9190,
    MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE           = 0x9191,
    MyHTML_STATUS_TAGS_ERROR_MCOBJECT_MALLOC           = 0x9192,
    MyHTML_STATUS_TAGS_ERROR_MCOBJECT_CREATE_NODE      = 0x9193,
    MyHTML_STATUS_TAGS_ERROR_CACHE_MEMORY_ALLOCATION   = 0x9194,
    MyHTML_STATUS_TAGS_ERROR_INDEX_MEMORY_ALLOCATION   = 0x9195,
    MyHTML_STATUS_TREE_ERROR_MEMORY_ALLOCATION         = 0x91f4,
    MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE           = 0x91f5,
    MyHTML_STATUS_TREE_ERROR_MCOBJECT_INIT             = 0x91f6,
    MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE_NODE      = 0x91f7,
    MyHTML_STATUS_TREE_ERROR_INCOMING_BUFFER_CREATE    = 0x91f8,
    MyHTML_STATUS_ATTR_ERROR_ALLOCATION                = 0x9258,
    MyHTML_STATUS_ATTR_ERROR_CREATE                    = 0x9259,
    MyHTML_STATUS_STREAM_BUFFER_ERROR_CREATE           = 0x9300,
    MyHTML_STATUS_STREAM_BUFFER_ERROR_INIT             = 0x9301,
    MyHTML_STATUS_STREAM_BUFFER_ENTRY_ERROR_CREATE     = 0x9302,
    MyHTML_STATUS_STREAM_BUFFER_ENTRY_ERROR_INIT       = 0x9303,
    MyHTML_STATUS_STREAM_BUFFER_ERROR_ADD_ENTRY        = 0x9304
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
    
    /* MyHTML_NAMESPACE_ANY == MyHTML_NAMESPACE_LAST_ENTRY */
    MyHTML_NAMESPACE_ANY        = 0x07,
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

typedef myhtml_token_attr_t myhtml_tree_attr_t;
typedef struct myhtml_collection myhtml_collection_t;
typedef struct myhtml myhtml_t;

// parser state function
typedef size_t (*myhtml_tokenizer_state_f)(myhtml_tree_t* tree, myhtml_token_node_t* token_node, const char* html, size_t html_offset, size_t html_size);

// parser insertion mode function
typedef bool (*myhtml_insertion_f)(myhtml_tree_t* tree, myhtml_token_node_t* token);

// char references state
typedef size_t (*myhtml_data_process_state_f)(myhtml_data_process_entry_t* charef, mycore_string_t* str, const char* data, size_t offset, size_t size);

// callback functions
typedef void* (*myhtml_callback_token_f)(myhtml_tree_t* tree, myhtml_token_node_t* token, void* ctx);
typedef void (*myhtml_callback_tree_node_f)(myhtml_tree_t* tree, myhtml_tree_node_t* node, void* ctx);

// find attribute value functions
typedef bool (*myhtml_attribute_value_find_f)(mycore_string_t* str_key, const char* value, size_t value_len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif


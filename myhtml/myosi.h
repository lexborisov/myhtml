//
//  myosi.h
//  myhtml
//
//  Created by Alexander Borisov on 30.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_myosi_h
#define myhtml_myosi_h

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#define myhtml_base_add(__myhtml__, __point__, __lenn__, __sizen__, __strcn__, __size__)    \
    __myhtml__->__lenn__++;                                                                 \
    if(__myhtml__->__lenn__ == __myhtml__->__sizen__) {                                     \
        __myhtml__->__sizen__ += __size__;                                                  \
        __myhtml__->__point__ = (__strcn__*)realloc(__myhtml__->__point__,                  \
            sizeof(__strcn__) * __myhtml__->__sizen__);                                     \
    }

typedef enum {myfalse = 0, mytrue = 1} mybool_t;

// thread
typedef size_t myhtml_thread_index_t;

typedef struct myhtml_thread_context myhtml_thread_context_t;
typedef struct myhtml_thread_list myhtml_thread_list_t;
typedef struct myhtml_thread myhtml_thread_t;

// tree
typedef struct myhtml_tree_indexes myhtml_tree_indexes_t;

typedef size_t myhtml_tree_index_t;

typedef struct myhtml_tree_node myhtml_tree_node_t;
typedef struct myhtml_tree myhtml_tree_t;

// queue
enum myhtml_queue_node_opt {
    MyHTML_QUEUE_OPT_CLEAN = 0x00,
    MyHTML_QUEUE_OPT_QUIT  = 0x01
};

enum myhtml_token_type {
    MyHTML_TOKEN_TYPE_OPEN             = 0x00,
    MyHTML_TOKEN_TYPE_CLOSE            = 0x01,
    MyHTML_TOKEN_TYPE_CLOSE_SELF       = 0x02
};

typedef size_t myhtml_queue_node_index_t;

typedef struct myhtml_queue_node myhtml_queue_node_t;
typedef struct myhtml_queue myhtml_queue_t;

// token
typedef size_t myhtml_token_index_t;
typedef size_t myhtml_token_attr_index_t;

typedef struct myhtml_token_attr myhtml_token_attr_t;
typedef struct myhtml_token_node myhtml_token_node_t;
typedef struct myhtml_token myhtml_token_t;

// tags
typedef struct mytags_index_tag_node mytags_index_tag_node_t;
typedef struct mytags_index_tag mytags_index_tag_t;
typedef struct mytags_index mytags_index_t;

typedef size_t mytags_ctx_index_t;
typedef size_t mytags_tag_id_t;

typedef struct mytags mytags_t;

// parse
enum myhtml_parse_state {
    MyHTML_PARSE_STATE_DATA = 0,
    MyHTML_PARSE_STATE_CHARACTER_REFERENCE_IN_DATA = 1,
    MyHTML_PARSE_STATE_RCDATA = 2,
    MyHTML_PARSE_STATE_CHARACTER_REFERENCE_IN_RCDATA = 3,
    MyHTML_PARSE_STATE_RAWTEXT = 4,
    MyHTML_PARSE_STATE_SCRIPT_DATA = 5,
    MyHTML_PARSE_STATE_PLAINTEXT = 6,
    MyHTML_PARSE_STATE_TAG_OPEN = 7,
    MyHTML_PARSE_STATE_END_TAG_OPEN = 8,
    MyHTML_PARSE_STATE_TAG_NAME = 9,
    MyHTML_PARSE_STATE_RCDATA_LESS_THAN_SIGN = 10,
    MyHTML_PARSE_STATE_RCDATA_END_TAG_OPEN = 11,
    MyHTML_PARSE_STATE_RCDATA_END_TAG_NAME = 12,
    MyHTML_PARSE_STATE_RAWTEXT_LESS_THAN_SIGN = 13,
    MyHTML_PARSE_STATE_RAWTEXT_END_TAG_OPEN = 14,
    MyHTML_PARSE_STATE_RAWTEXT_END_TAG_NAME = 15,
    MyHTML_PARSE_STATE_SCRIPT_DATA_LESS_THAN_SIGN = 16,
    MyHTML_PARSE_STATE_SCRIPT_DATA_END_TAG_OPEN = 17,
    MyHTML_PARSE_STATE_SCRIPT_DATA_END_TAG_NAME = 18,
    MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPE_START = 19,
    MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPE_START_DASH = 20,
    MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED = 21,
    MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_DASH = 22,
    MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_DASH_DASH = 23,
    MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN = 24,
    MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_END_TAG_OPEN = 25,
    MyHTML_PARSE_STATE_SCRIPT_DATA_ESCAPED_END_TAG_NAME = 26,
    MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_START = 27,
    MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPED = 28,
    MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH = 29,
    MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_DASH_DASH = 30,
    MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN = 31,
    MyHTML_PARSE_STATE_SCRIPT_DATA_DOUBLE_ESCAPE_END = 32,
    MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_NAME = 33,
    MyHTML_PARSE_STATE_ATTRIBUTE_NAME = 34,
    MyHTML_PARSE_STATE_AFTER_ATTRIBUTE_NAME = 35,
    MyHTML_PARSE_STATE_BEFORE_ATTRIBUTE_VALUE = 36,
    MyHTML_PARSE_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED = 37,
    MyHTML_PARSE_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED = 38,
    MyHTML_PARSE_STATE_ATTRIBUTE_VALUE_UNQUOTED = 39,
    MyHTML_PARSE_STATE_CHARACTER_REFERENCE_IN_ATTRIBUTE_VALUE = 40,
    MyHTML_PARSE_STATE_AFTER_ATTRIBUTE_VALUE__QUOTED = 41,
    MyHTML_PARSE_STATE_SELF_CLOSING_START_TAG = 42,
    MyHTML_PARSE_STATE_BOGUS_COMMENT = 43,
    MyHTML_PARSE_STATE_MARKUP_DECLARATION_OPEN = 44,
    MyHTML_PARSE_STATE_COMMENT_START = 45,
    MyHTML_PARSE_STATE_COMMENT_START_DASH = 46,
    MyHTML_PARSE_STATE_COMMENT = 47,
    MyHTML_PARSE_STATE_COMMENT_END_DASH = 48,
    MyHTML_PARSE_STATE_COMMENT_END = 49,
    MyHTML_PARSE_STATE_COMMENT_END_BANG = 50,
    MyHTML_PARSE_STATE_DOCTYPE = 51,
    MyHTML_PARSE_STATE_BEFORE_DOCTYPE_NAME = 52,
    MyHTML_PARSE_STATE_DOCTYPE_NAME = 53,
    MyHTML_PARSE_STATE_AFTER_DOCTYPE_NAME = 54,
    MyHTML_PARSE_STATE_AFTER_DOCTYPE_PUBLIC_KEYWORD = 55,
    MyHTML_PARSE_STATE_BEFORE_DOCTYPE_PUBLIC_IDENTIFIER = 56,
    MyHTML_PARSE_STATE_DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED = 57,
    MyHTML_PARSE_STATE_DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED = 58,
    MyHTML_PARSE_STATE_AFTER_DOCTYPE_PUBLIC_IDENTIFIER = 59,
    MyHTML_PARSE_STATE_BETWEEN_DOCTYPE_PUBLIC_AND_SYSTEM_IDENTIFIERS = 60,
    MyHTML_PARSE_STATE_AFTER_DOCTYPE_SYSTEM_KEYWORD = 61,
    MyHTML_PARSE_STATE_BEFORE_DOCTYPE_SYSTEM_IDENTIFIER = 62,
    MyHTML_PARSE_STATE_DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED = 63,
    MyHTML_PARSE_STATE_DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED = 64,
    MyHTML_PARSE_STATE_AFTER_DOCTYPE_SYSTEM_IDENTIFIER = 65,
    MyHTML_PARSE_STATE_BOGUS_DOCTYPE = 66,
    MyHTML_PARSE_STATE_CDATA_SECTION = 67,
    MyHTML_PARSE_STATE_FIRST_ENTRY = 0,
    MyHTML_PARSE_STATE_LAST_ENTRY = 68
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
typedef struct myhtml myhtml_t;


// parser state function
typedef size_t (*myhtml_parse_state_f)(myhtml_tree_t* tree, const char* html, size_t html_offset, size_t html_size);

// parser stream function
typedef void (*myhtml_thread_f)(myhtml_tree_t* tree, myhtml_queue_node_index_t qnode_idx, myhtml_token_index_t token_idx);

// parser insertion mode function
typedef void (*myhtml_insertion_f)(myhtml_tree_t* tree, myhtml_token_index_t token_idx);

#endif


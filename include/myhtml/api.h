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

#ifndef MyHTML_MyHTML_API_H
#define MyHTML_MyHTML_API_H

/**
 * @file myhtml/api.h
 *
 * Fast C/C++ HTML 5 Parser. Using threads.
 * With possibility of a Single Mode.
 * 
 * C99 and POSIX Threads! No dependencies!
 *
 * By https://html.spec.whatwg.org/ specification.
 *
 */

#define MyHTML_VERSION_MAJOR 0
#define MyHTML_VERSION_MINOR 1
#define MyHTML_VERSION_PATCH 0

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * bool
 */
typedef enum {myfalse = 0, mytrue = 1} mybool_t;

/**
 * @struct basic tag ids
 */
enum myhtml_tags {
    MyHTML_TAG__UNDEF         = 0x000,
    MyHTML_TAG__TEXT          = 0x001,
    MyHTML_TAG__COMMENT       = 0x002,
    MyHTML_TAG__DOCTYPE       = 0x003,
    MyHTML_TAG_A              = 0x004,
    MyHTML_TAG_ABBR           = 0x005,
    MyHTML_TAG_ACRONYM        = 0x006,
    MyHTML_TAG_ADDRESS        = 0x007,
    MyHTML_TAG_ANNOTATION_XML = 0x008,
    MyHTML_TAG_APPLET         = 0x009,
    MyHTML_TAG_AREA           = 0x00a,
    MyHTML_TAG_ARTICLE        = 0x00b,
    MyHTML_TAG_ASIDE          = 0x00c,
    MyHTML_TAG_AUDIO          = 0x00d,
    MyHTML_TAG_B              = 0x00e,
    MyHTML_TAG_BASE           = 0x00f,
    MyHTML_TAG_BASEFONT       = 0x010,
    MyHTML_TAG_BDI            = 0x011,
    MyHTML_TAG_BDO            = 0x012,
    MyHTML_TAG_BGSOUND        = 0x013,
    MyHTML_TAG_BIG            = 0x014,
    MyHTML_TAG_BLINK          = 0x015,
    MyHTML_TAG_BLOCKQUOTE     = 0x016,
    MyHTML_TAG_BODY           = 0x017,
    MyHTML_TAG_BR             = 0x018,
    MyHTML_TAG_BUTTON         = 0x019,
    MyHTML_TAG_CANVAS         = 0x01a,
    MyHTML_TAG_CAPTION        = 0x01b,
    MyHTML_TAG_CENTER         = 0x01c,
    MyHTML_TAG_CITE           = 0x01d,
    MyHTML_TAG_CODE           = 0x01e,
    MyHTML_TAG_COL            = 0x01f,
    MyHTML_TAG_COLGROUP       = 0x020,
    MyHTML_TAG_COMMAND        = 0x021,
    MyHTML_TAG_COMMENT        = 0x022,
    MyHTML_TAG_DATALIST       = 0x023,
    MyHTML_TAG_DD             = 0x024,
    MyHTML_TAG_DEL            = 0x025,
    MyHTML_TAG_DESC           = 0x026,
    MyHTML_TAG_DETAILS        = 0x027,
    MyHTML_TAG_DFN            = 0x028,
    MyHTML_TAG_DIALOG         = 0x029,
    MyHTML_TAG_DIR            = 0x02a,
    MyHTML_TAG_DIV            = 0x02b,
    MyHTML_TAG_DL             = 0x02c,
    MyHTML_TAG_DT             = 0x02d,
    MyHTML_TAG_EM             = 0x02e,
    MyHTML_TAG_EMBED          = 0x02f,
    MyHTML_TAG_FIELDSET       = 0x030,
    MyHTML_TAG_FIGCAPTION     = 0x031,
    MyHTML_TAG_FIGURE         = 0x032,
    MyHTML_TAG_FONT           = 0x033,
    MyHTML_TAG_FOOTER         = 0x034,
    MyHTML_TAG_FOREIGNOBJECT  = 0x035,
    MyHTML_TAG_FORM           = 0x036,
    MyHTML_TAG_FRAME          = 0x037,
    MyHTML_TAG_FRAMESET       = 0x038,
    MyHTML_TAG_H1             = 0x039,
    MyHTML_TAG_H2             = 0x03a,
    MyHTML_TAG_H3             = 0x03b,
    MyHTML_TAG_H4             = 0x03c,
    MyHTML_TAG_H5             = 0x03d,
    MyHTML_TAG_H6             = 0x03e,
    MyHTML_TAG_HEAD           = 0x03f,
    MyHTML_TAG_HEADER         = 0x040,
    MyHTML_TAG_HGROUP         = 0x041,
    MyHTML_TAG_HR             = 0x042,
    MyHTML_TAG_HTML           = 0x043,
    MyHTML_TAG_I              = 0x044,
    MyHTML_TAG_IFRAME         = 0x045,
    MyHTML_TAG_IMAGE          = 0x046,
    MyHTML_TAG_IMG            = 0x047,
    MyHTML_TAG_INPUT          = 0x048,
    MyHTML_TAG_INS            = 0x049,
    MyHTML_TAG_ISINDEX        = 0x04a,
    MyHTML_TAG_KBD            = 0x04b,
    MyHTML_TAG_KEYGEN         = 0x04c,
    MyHTML_TAG_LABEL          = 0x04d,
    MyHTML_TAG_LEGEND         = 0x04e,
    MyHTML_TAG_LI             = 0x04f,
    MyHTML_TAG_LINK           = 0x050,
    MyHTML_TAG_LISTING        = 0x051,
    MyHTML_TAG_MAIN           = 0x052,
    MyHTML_TAG_MALIGNMARK     = 0x053,
    MyHTML_TAG_MAP            = 0x054,
    MyHTML_TAG_MARK           = 0x055,
    MyHTML_TAG_MARQUEE        = 0x056,
    MyHTML_TAG_MATH           = 0x057,
    MyHTML_TAG_MENU           = 0x058,
    MyHTML_TAG_MENUITEM       = 0x059,
    MyHTML_TAG_MERROR         = 0x05a,
    MyHTML_TAG_META           = 0x05b,
    MyHTML_TAG_METER          = 0x05c,
    MyHTML_TAG_MGLYPH         = 0x05d,
    MyHTML_TAG_MI             = 0x05e,
    MyHTML_TAG_MO             = 0x05f,
    MyHTML_TAG_MN             = 0x060,
    MyHTML_TAG_MS             = 0x061,
    MyHTML_TAG_MTEXT          = 0x062,
    MyHTML_TAG_NAV            = 0x063,
    MyHTML_TAG_NOBR           = 0x064,
    MyHTML_TAG_NOEMBED        = 0x065,
    MyHTML_TAG_NOFRAMES       = 0x066,
    MyHTML_TAG_NOSCRIPT       = 0x067,
    MyHTML_TAG_OBJECT         = 0x068,
    MyHTML_TAG_OL             = 0x069,
    MyHTML_TAG_OPTGROUP       = 0x06a,
    MyHTML_TAG_OPTION         = 0x06b,
    MyHTML_TAG_OUTPUT         = 0x06c,
    MyHTML_TAG_P              = 0x06d,
    MyHTML_TAG_PARAM          = 0x06e,
    MyHTML_TAG_PLAINTEXT      = 0x06f,
    MyHTML_TAG_PRE            = 0x070,
    MyHTML_TAG_PROGRESS       = 0x071,
    MyHTML_TAG_Q              = 0x072,
    MyHTML_TAG_RB             = 0x073,
    MyHTML_TAG_RP             = 0x074,
    MyHTML_TAG_RT             = 0x075,
    MyHTML_TAG_RTC            = 0x076,
    MyHTML_TAG_RUBY           = 0x077,
    MyHTML_TAG_S              = 0x078,
    MyHTML_TAG_SAMP           = 0x079,
    MyHTML_TAG_SCRIPT         = 0x07a,
    MyHTML_TAG_SECTION        = 0x07b,
    MyHTML_TAG_SELECT         = 0x07c,
    MyHTML_TAG_SMALL          = 0x07d,
    MyHTML_TAG_SOURCE         = 0x07e,
    MyHTML_TAG_SPAN           = 0x07f,
    MyHTML_TAG_STRIKE         = 0x080,
    MyHTML_TAG_STRONG         = 0x081,
    MyHTML_TAG_STYLE          = 0x082,
    MyHTML_TAG_SUB            = 0x083,
    MyHTML_TAG_SUMMARY        = 0x084,
    MyHTML_TAG_SUP            = 0x085,
    MyHTML_TAG_SVG            = 0x086,
    MyHTML_TAG_TABLE          = 0x087,
    MyHTML_TAG_TBODY          = 0x088,
    MyHTML_TAG_TD             = 0x089,
    MyHTML_TAG_TEMPLATE       = 0x08a,
    MyHTML_TAG_TEXTAREA       = 0x08b,
    MyHTML_TAG_TFOOT          = 0x08c,
    MyHTML_TAG_TH             = 0x08d,
    MyHTML_TAG_THEAD          = 0x08e,
    MyHTML_TAG_TIME           = 0x08f,
    MyHTML_TAG_TITLE          = 0x090,
    MyHTML_TAG_TR             = 0x091,
    MyHTML_TAG_TRACK          = 0x092,
    MyHTML_TAG_TT             = 0x093,
    MyHTML_TAG_U              = 0x094,
    MyHTML_TAG_UL             = 0x095,
    MyHTML_TAG_VAR            = 0x096,
    MyHTML_TAG_VIDEO          = 0x097,
    MyHTML_TAG_WBR            = 0x098,
    MyHTML_TAG_XMP            = 0x099,
    MyHTML_TAG__END_OF_FILE   = 0x09a,
    MyHTML_TAG_FIRST_ENTRY    = MyHTML_TAG__TEXT,
    MyHTML_TAG_LAST_ENTRY     = 0x09b
};

/**
 * @struct myhtml statuses
 */
enum myhtml_status {
    MyHTML_STATUS_OK                                   = 0,
    MyHTML_STATUS_ERROR_MEMORY_ALLOCATION              = 1,
    MyHTML_STATUS_THREAD_ERROR_LIST_INIT               = 10,
    MyHTML_STATUS_THREAD_ERROR_ATTR_MALLOC             = 11,
    MyHTML_STATUS_THREAD_ERROR_ATTR_INIT               = 12,
    MyHTML_STATUS_THREAD_ERROR_ATTR_SET                = 13,
    MyHTML_STATUS_THREAD_ERROR_NO_SLOTS                = 14,
    MyHTML_STATUS_THREAD_ERROR_BATCH_INIT              = 15,
    MyHTML_STATUS_THREAD_ERROR_WORKER_MALLOC           = 16,
    MyHTML_STATUS_THREAD_ERROR_WORKER_SEM_CREATE       = 17,
    MyHTML_STATUS_THREAD_ERROR_WORKER_THREAD_CREATE    = 18,
    MyHTML_STATUS_THREAD_ERROR_MASTER_THREAD_CREATE    = 19,
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
typedef myhtml_status_t

#define MYHTML_FAILED(_status_) ((_status_) != MyHTML_STATUS_OK)

/**
 * @struct myhtml namespace
 */
enum myhtml_namespace {
    MyHTML_NAMESPACE_UNDEF      = 0x00,
    MyHTML_NAMESPACE_HTML       = 0x01,
    MyHTML_NAMESPACE_MATHML     = 0x02,
    MyHTML_NAMESPACE_SVG        = 0x03,
    MyHTML_NAMESPACE_XLINK      = 0x04,
    MyHTML_NAMESPACE_XML        = 0x05,
    MyHTML_NAMESPACE_XMLNS      = 0x06,
    MyHTML_NAMESPACE_LAST_ENTRY = 0x07
};

/**
 * @struct myhtml options
 */
enum myhtml_options {
    MyHTML_OPTIONS_DEFAULT                 = 0x00,
    MyHTML_OPTIONS_PARSE_MODE_SINGLE       = 0x01,
    MyHTML_OPTIONS_PARSE_MODE_ALL_IN_ONE   = 0x02,
    MyHTML_OPTIONS_PARSE_MODE_SEPARATELY   = 0x04,
    MyHTML_OPTIONS_PARSE_MODE_WORKER_TREE  = 0x08,
    MyHTML_OPTIONS_PARSE_MODE_WORKER_INDEX = 0x10,
    MyHTML_OPTIONS_PARSE_MODE_TREE_INDEX   = 0x20
};

/**
 * @struct myhtml_t MyHTML
 *
 * Basic structure. Create once for using many times.
*/
typedef struct myhtml myhtml_t;

/**
 * @struct myhtml_tree_t MyHTML_TREE
 *
 * Secondary structure. Create once for using many times.
 */
typedef struct myhtml_tree myhtml_tree_t;

typedef struct myhtml_token_attr myhtml_tree_attr_t;
typedef struct myhtml_tree_node myhtml_tree_node_t;

/**
 * MyHTML_TAG
 *
 */
typedef size_t myhtml_tag_id_t;

typedef struct myhtml_tag_index_node myhtml_tag_index_node_t;
typedef struct myhtml_tag_index_entry myhtml_tag_index_entry_t;
typedef struct myhtml_tag_index myhtml_tag_index_t;

typedef struct myhtml_tag myhtml_tag_t;

/**
 * @struct myhtml_collection_t
 */
struct myhtml_collection {
    myhtml_tree_node_t **list;
    size_t size;
    size_t length;
}
typedef myhtml_collection_t;

/***********************************************************************************
 *
 * MyHTML
 *
 ***********************************************************************************/

/**
 * Create a MyHTML structure
 *
 * @return myhtml_t* if successful, otherwise an NULL value.
 */
myhtml_t*
myhtml_create(void);

/**
 * Allocating and Initialization resources for a MyHTML structure
 *
 * @param[in] myhtml_t*
 * @param[in] work options, how many threads will be. 
 * Default: MyHTML_OPTIONS_PARSE_MODE_SEPARATELY
 *
 * @param[in] thread count, it depends on the choice of work options
 * Default: 1
 *
 * @param[in] queue size for a tokens. Dynamically increasing the specified number here. Default: 4096
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status value.
 */
myhtml_status_t
myhtml_init(myhtml_t* myhtml, enum myhtml_options opt,
            size_t thread_count, size_t queue_size);

/**
 * Clears queue and threads resources
 *
 * @param[in] myhtml_t*
 */
void
myhtml_clean(myhtml_t* myhtml);

/**
 * Destroy of a MyHTML structure
 *
 * @param[in] myhtml_t*
 * @return NULL if successful, otherwise an MyHTML structure.
 */
myhtml_t*
myhtml_destroy(myhtml_t* myhtml);

/**
 * Parsing HTML
 *
 * @param[in] previously created structure myhtml_tree_t*
 * @param[in] HTML
 * @param[in] HTML size
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
myhtml_status_t
myhtml_parse(myhtml_tree_t* tree, const char* html, size_t html_size);

/**
 * Parsing fragment of HTML
 *
 * @param[in] previously created structure myhtml_tree_t*
 * @param[in] HTML
 * @param[in] HTML size
 * @param[in] fragment base (root) tag id. Default: MyHTML_TAG_DIV if set 0
 * @param[in] fragment NAMESPACE. Default: MyHTML_NAMESPACE_HTML if set 0
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
myhtml_status_t
myhtml_parse_fragment(myhtml_tree_t* tree, const char* html, size_t html_size,
                      myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace);

/**
 * Parsing HTML in Single Mode. 
 * No matter what was said during initialization MyHTML
 *
 * @param[in] previously created structure myhtml_tree_t*
 * @param[in] HTML
 * @param[in] HTML size
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
myhtml_status_t
myhtml_parse_single(myhtml_tree_t* tree, const char* html, size_t html_size);

/**
 * Parsing fragment of HTML in Single Mode. 
 * No matter what was said during initialization MyHTML
 *
 * @param[in] previously created structure myhtml_tree_t*
 * @param[in] HTML
 * @param[in] HTML size
 * @param[in] fragment base (root) tag id. Default: MyHTML_TAG_DIV if set 0
 * @param[in] fragment NAMESPACE. Default: MyHTML_NAMESPACE_HTML if set 0
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
myhtml_status_t
myhtml_parse_fragment_single(myhtml_tree_t* tree, const char* html, size_t html_size,
                      myhtml_tag_id_t tag_id, enum myhtml_namespace my_namespace);

/**
 * Get myhtml_tag_t* from a myhtml_t*
 *
 * @param[in] myhtml_t*
 *
 * @return myhtml_tag_t* if exists, otherwise a NULL value
 */
myhtml_tag_t*
myhtml_get_tag(myhtml_t* myhtml);

/***********************************************************************************
 *
 * MyHTML_TREE
 *
 ***********************************************************************************/

/**
 * Create a MyHTML_TREE structure
 *
 * @return myhtml_tree_t* if successful, otherwise an NULL value.
 */
myhtml_tree_t*
myhtml_tree_create(void);

/**
 * Allocating and Initialization resources for a MyHTML_TREE structure
 *
 * @param[in] myhtml_tree_t*
 * @param[in] workmyhtml_t*
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
myhtml_status_t
myhtml_tree_init(myhtml_tree_t* tree, myhtml_t* myhtml);

/**
 * Clears resources before new parsing
 *
 * @param[in] myhtml_tree_t*
 */
void
myhtml_tree_clean(myhtml_tree_t* tree);

/**
 * Destroy of a MyHTML_TREE structure
 *
 * @param[in] myhtml_tree_t*
 *
 * @return NULL if successful, otherwise an MyHTML_TREE structure
 */
myhtml_tree_t*
myhtml_tree_destroy(myhtml_tree_t* tree);

/**
 * Get myhtml_t* from a myhtml_tree_t*
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_t* if exists, otherwise a NULL value
 */
myhtml_t*
myhtml_tree_get_myhtml(myhtml_tree_t* tree);

/**
 * Get myhtml_tag_t* from a myhtml_tree_t*
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_tag_t* if exists, otherwise a NULL value
 */
myhtml_tag_t*
myhtml_tree_get_tag(myhtml_tree_t* tree);

/**
 * Get myhtml_tag_index_t* from a myhtml_tree_t*
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_tag_index_t* if exists, otherwise a NULL value
 */
myhtml_tag_index_t*
myhtml_tree_get_tag_index(myhtml_tree_t* tree);

/**
 * Get Tree Document (Root of Tree)
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_tree_get_document(myhtml_tree_t* tree);

/**
 * Print tree of a node. Print including current node
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_tree_node_t*
 * @param[in] file handle, for example use stdout
 * @param[in] tab (\t) increment for pretty print, set 0
 */
void
myhtml_tree_print_by_node(myhtml_tree_t* tree, myhtml_tree_node_t* node,
                          FILE* out, size_t inc);

/**
 * Print tree of a node. Print excluding current node
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_tree_node_t*
 * @param[in] file handle, for example use stdout
 * @param[in] tab (\t) increment for pretty print, set 0
 */
void
myhtml_tree_print_node_childs(myhtml_tree_t* tree, myhtml_tree_node_t* node,
                              FILE* out, size_t inc);

/**
 * Print a node
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_tree_node_t*
 * @param[in] file handle, for example use stdout
 */
void
myhtml_tree_print_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, FILE* out);

/***********************************************************************************
 *
 * MyHTML_NODE
 *
 ***********************************************************************************/

/**
 * Get nodes by tag id
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, creates new collection if NULL
 * @param[in] tag id
 * @param[out] status of this operation
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_tag_id(myhtml_tree_t* tree, myhtml_collection_t *collection,
                           myhtml_tag_id_t tag_id, myhtml_status_t *status);

/**
 * Get nodes by tag name
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, creates new collection if NULL
 * @param[in] tag name
 * @param[in] tag name length
 * @param[out] status of this operation
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_name(myhtml_tree_t* tree, myhtml_collection_t *collection,
                         const char* name, size_t length, myhtml_status_t *status);

/**
 * Get next sibling node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_node_t* if exists, otherwise an NULL value
 */
myhtml_tree_node_t*
myhtml_node_next(myhtml_tree_node_t *node);

/**
 * Get previous sibling node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_node_t* if exists, otherwise an NULL value
 */
myhtml_tree_node_t*
myhtml_node_prev(myhtml_tree_node_t *node);

/**
 * Get parent node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_node_t* if exists, otherwise an NULL value
 */
myhtml_tree_node_t*
myhtml_node_parent(myhtml_tree_node_t *node);

/**
 * Get child (first child) of node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_node_t* if exists, otherwise an NULL value
 */
myhtml_tree_node_t*
myhtml_node_child(myhtml_tree_node_t *node);

/**
 * Get last child of node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_node_t* if exists, otherwise an NULL value
 */
myhtml_tree_node_t*
myhtml_node_last_child(myhtml_tree_node_t *node);

/**
 * Get node namespace
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return enum myhtml_namespace
 */
enum myhtml_namespace
myhtml_node_namespace(myhtml_tree_node_t *node);

/**
 * Get node tag id
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tag_id_t
 */
myhtml_tag_id_t
myhtml_node_tag_id(myhtml_tree_node_t *node);

/**
 * Get tag name by tag id
 *
 * @param[in] myhtml_tree_t*
 * @param[in] tag id
 * @param[out] optional, name length
 *
 * @return const char* if exists, otherwise a NULL value
 */
const char*
myhtml_tag_name_by_id(myhtml_tree_t* tree,
                      myhtml_tag_id_t tag_id, size_t *length);

/**
 * Node has self-closing flag?
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return mytrue or myfalse (1 or 0)
 */
mybool_t
myhtml_node_is_close_self(myhtml_tree_node_t *node);

/**
 * Get first attribute of a node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_attr_t* if exists, otherwise an NULL value
 */
myhtml_tree_attr_t*
myhtml_node_attribute_first(myhtml_tree_node_t *node);

/**
 * Get last attribute of a node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_attr_t* if exists, otherwise an NULL value
 */
myhtml_tree_attr_t*
myhtml_node_attribute_last(myhtml_tree_node_t *node);

/**
 * Get text of a node. Only for a MyHTML_TAG__TEXT or MyHTML_TAG__COMMENT tags
 *
 * @param[in] myhtml_tree_node_t*
 * @param[out] optional, text length
 *
 * @return const char* if exists, otherwise an NULL value
 */
const char*
myhtml_node_text(myhtml_tree_node_t *node, size_t *length);


/***********************************************************************************
 *
 * MyHTML_ATTRIBUTE
 *
 ***********************************************************************************/

/**
 * Get next sibling attribute of one node
 *
 * @param[in] myhtml_tree_attr_t*
 *
 * @return myhtml_tree_attr_t* if exists, otherwise an NULL value
 */
myhtml_tree_attr_t*
myhtml_attribute_next(myhtml_tree_attr_t *attr);

/**
 * Get previous sibling attribute of one node
 *
 * @param[in] myhtml_tree_attr_t*
 *
 * @return myhtml_tree_attr_t* if exists, otherwise an NULL value
 */
myhtml_tree_attr_t*
myhtml_attribute_prev(myhtml_tree_attr_t *attr);

/**
 * Get attribute namespace
 *
 * @param[in] myhtml_tree_attr_t*
 *
 * @return enum myhtml_namespace
 */
enum myhtml_namespace
myhtml_attribute_namespace(myhtml_tree_attr_t *attr);

/**
 * Get attribute name (key)
 *
 * @param[in] myhtml_tree_attr_t*
 * @param[out] optional, name length
 *
 * @return const char* if exists, otherwise an NULL value
 */
const char*
myhtml_attribute_name(myhtml_tree_attr_t *attr, size_t *length);

/**
 * Get attribute value
 *
 * @param[in] myhtml_tree_attr_t*
 * @param[out] optional, value length
 *
 * @return const char* if exists, otherwise an NULL value
 */
const char*
myhtml_attribute_value(myhtml_tree_attr_t *attr, size_t *length);

/**
 * Get attribute by key
 *
 * @param[in] myhtml_tree_node_t*
 * @param[in] attr key name
 * @param[in] attr key name length
 *
 * @return myhtml_tree_attr_t* if exists, otherwise a NULL value
 */
myhtml_tree_attr_t*
myhtml_attribute_by_key(myhtml_tree_node_t *node,
                        const char *key, size_t key_len);

/**
 * Added attribute to tree node
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_tree_node_t*
 * @param[in] attr key name
 * @param[in] attr key name length
 * @param[in] attr value name
 * @param[in] attr value name length
 *
 * @return created myhtml_tree_attr_t* if successful, otherwise a NULL value
 */
myhtml_tree_attr_t*
myhtml_attribute_add(myhtml_tree_t *tree, myhtml_tree_node_t *node,
                     const char *key, size_t key_len,
                     const char *value, size_t value_len);

/**
 * Remove attribute reference. Do not release the resources
 *
 * @param[in] myhtml_tree_node_t*
 * @param[in] myhtml_tree_attr_t*
 *
 * @return myhtml_tree_attr_t* if successful, otherwise a NULL value
 */
myhtml_tree_attr_t*
myhtml_attribute_remove(myhtml_tree_node_t *node, myhtml_tree_attr_t *attr);

/**
 * Remove attribute by key reference. Do not release the resources
 *
 * @param[in] myhtml_tree_node_t*
 * @param[in] attr key name
 * @param[in] attr key name length
 *
 * @return myhtml_tree_attr_t* if successful, otherwise a NULL value
 */
myhtml_tree_attr_t*
myhtml_attribute_remove_by_key(myhtml_tree_node_t *node, const char *key, size_t key_len);

/**
 * Remove attribute and release allocated resources
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_tree_node_t*
 * @param[in] myhtml_tree_attr_t*
 *
 * @return myhtml_tree_attr_t* if successful, otherwise a NULL value
 */
void
myhtml_attribute_delete(myhtml_tree_t *tree, myhtml_tree_node_t *node,
                        myhtml_tree_attr_t *attr);

/**
 * Release allocated resources
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_tree_attr_t*
 *
 * @return myhtml_tree_attr_t* if successful, otherwise a NULL value
 */
void
myhtml_attribute_free(myhtml_tree_t *tree, myhtml_tree_attr_t *attr);

/***********************************************************************************
 *
 * MyHTML_TAG_INDEX
 *
 ***********************************************************************************/

/**
 * Create tag index structure
 *
 * @param[in] myhtml_tag_t*
 *
 * @return myhtml_tag_index_t* if successful, otherwise a NULL value
 */
myhtml_tag_index_t*
myhtml_tag_index_create(myhtml_tag_t* tag);

/**
 * Allocating and Initialization resources for a tag index structure
 *
 * @param[in] myhtml_tag_t*
 * @param[in] myhtml_tag_index_t*
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status.
 */
myhtml_status_t
myhtml_tag_index_init(myhtml_tag_t* tag, myhtml_tag_index_t* tag_index);

/**
 * Clears tag index
 *
 * @param[in] myhtml_tag_t*
 * @param[in] myhtml_tag_index_t*
 *
 */
void
myhtml_tag_index_clean(myhtml_tag_t* tag, myhtml_tag_index_t* tag_index);

/**
 * Free allocated resources
 *
 * @param[in] myhtml_tag_t*
 * @param[in] myhtml_tag_index_t*
 *
 * @return NULL if successful, otherwise an myhtml_tag_index_t* structure
 */
myhtml_tag_index_t*
myhtml_tag_index_destroy(myhtml_tag_t* tag, myhtml_tag_index_t* tag_index);

/**
 * Adds myhtml_tree_node_t* to tag index
 *
 * @param[in] myhtml_tag_t*
 * @param[in] myhtml_tag_index_t*
 * @param[in] myhtml_tree_node_t*
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status.
 */
myhtml_status_t
myhtml_tag_index_add(myhtml_tag_t* tag, myhtml_tag_index_t* tag_index, myhtml_tree_node_t* node);

/**
 * Get root tag index. Is the initial entry for a tag. It contains statistics and other items by tag
 *
 * @param[in] myhtml_tag_index_t*
 * @param[in] myhtml_tag_id_t
 *
 * @return myhtml_tag_index_entry_t* if successful, otherwise a NULL value.
 */
myhtml_tag_index_entry_t*
myhtml_tag_index_entry(myhtml_tag_index_t* tag_index, myhtml_tag_id_t tag_id);

/**
 * Get first index node for tag
 *
 * @param[in] myhtml_tag_index_t*
 * @param[in] myhtml_tag_id_t
 *
 * @return myhtml_tag_index_node_t* if exists, otherwise a NULL value.
 */
myhtml_tag_index_node_t*
myhtml_tag_index_first(myhtml_tag_index_t* tag_index, myhtml_tag_id_t tag_id);

/**
 * Get last index node for tag
 *
 * @param[in] myhtml_tag_index_t*
 * @param[in] myhtml_tag_id_t
 *
 * @return myhtml_tag_index_node_t* if exists, otherwise a NULL value.
 */
myhtml_tag_index_node_t*
myhtml_tag_index_last(myhtml_tag_index_t* tag_index, myhtml_tag_id_t tag_id);

/**
 * Get next index node for tag, by index node
 *
 * @param[in] myhtml_tag_index_node_t*
 *
 * @return myhtml_tag_index_node_t* if exists, otherwise a NULL value.
 */
myhtml_tag_index_node_t*
myhtml_tag_index_next(myhtml_tag_index_node_t *index_node);

/**
 * Get previous index node for tag, by index node
 *
 * @param[in] myhtml_tag_index_node_t*
 *
 * @return myhtml_tag_index_node_t* if exists, otherwise a NULL value.
 */
myhtml_tag_index_node_t*
myhtml_tag_index_prev(myhtml_tag_index_node_t *index_node);

/**
 * Get myhtml_tree_node_t* by myhtml_tag_index_node_t*
 *
 * @param[in] myhtml_tag_index_node_t*
 *
 * @return myhtml_tree_node_t* if exists, otherwise a NULL value.
 */
myhtml_tree_node_t*
myhtml_tag_index_tree_node(myhtml_tag_index_node_t *index_node);

/**
 * Get count of elements in index by tag id
 *
 * @param[in] myhtml_tag_index_t*
 * @param[in] tag id
 *
 * @return count of elements
 */
size_t
myhtml_tag_index_entry_count(myhtml_tag_index_t* tag_index, myhtml_tag_id_t tag_id);

/***********************************************************************************
 *
 * MyHTML_COLLECTION
 *
 ***********************************************************************************/

/**
 * Create collection
 *
 * @param[in] list size
 * @param[out] optional, status of operation
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_collection_create(size_t size, myhtml_status_t *status);

/**
 * Clears collection
 *
 * @param[in] myhtml_collection_t*
 */
void
myhtml_collection_clean(myhtml_collection_t *collection);

/**
 * Destroy allocated resources
 *
 * @param[in] myhtml_collection_t*
 *
 * @return NULL if successful, otherwise an myhtml_collection_t* structure
 */
myhtml_collection_t*
myhtml_collection_destroy(myhtml_collection_t *collection);

/**
 * Check size by length and increase if necessary
 *
 * @param[in] myhtml_collection_t*
 * @param[in] count of add nodes
 *
 * @return NULL if successful, otherwise an myhtml_collection_t* structure
 */
myhtml_status_t
myhtml_collection_check_size(myhtml_collection_t *collection, size_t up_to_length);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_api.h */

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

#ifndef MyHTML_MyHTML_API_H
#define MyHTML_MyHTML_API_H
#pragma once

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

#define MyHTML_VERSION_MAJOR 4
#define MyHTML_VERSION_MINOR 0
#define MyHTML_VERSION_PATCH 6

#define MyHTML_VERSION_STRING MyCORE_STR(MyHTML_VERSION_MAJOR) MyCORE_STR(.) MyCORE_STR(MyHTML_VERSION_MINOR) MyCORE_STR(.) MyCORE_STR(MyHTML_VERSION_PATCH)

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include <mycore/myosi.h>
#include <mycore/incoming.h>
#include <mycore/mystring.h>
#include <mycore/utils/mchar_async.h>
#include <myencoding/myosi.h>

#if defined(_MSC_VER)
#  define MyHTML_DEPRECATED(func) __declspec(deprecated) func
#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
#  define MyHTML_DEPRECATED(func) func __attribute__((deprecated))
#else
#  define MyHTML_DEPRECATED(func) func
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct basic tag ids
 */
enum myhtml_tags {
    MyHTML_TAG__UNDEF              = 0x000,
    MyHTML_TAG__TEXT               = 0x001,
    MyHTML_TAG__COMMENT            = 0x002,
    MyHTML_TAG__DOCTYPE            = 0x003,
    MyHTML_TAG_A                   = 0x004,
    MyHTML_TAG_ABBR                = 0x005,
    MyHTML_TAG_ACRONYM             = 0x006,
    MyHTML_TAG_ADDRESS             = 0x007,
    MyHTML_TAG_ANNOTATION_XML      = 0x008,
    MyHTML_TAG_APPLET              = 0x009,
    MyHTML_TAG_AREA                = 0x00a,
    MyHTML_TAG_ARTICLE             = 0x00b,
    MyHTML_TAG_ASIDE               = 0x00c,
    MyHTML_TAG_AUDIO               = 0x00d,
    MyHTML_TAG_B                   = 0x00e,
    MyHTML_TAG_BASE                = 0x00f,
    MyHTML_TAG_BASEFONT            = 0x010,
    MyHTML_TAG_BDI                 = 0x011,
    MyHTML_TAG_BDO                 = 0x012,
    MyHTML_TAG_BGSOUND             = 0x013,
    MyHTML_TAG_BIG                 = 0x014,
    MyHTML_TAG_BLINK               = 0x015,
    MyHTML_TAG_BLOCKQUOTE          = 0x016,
    MyHTML_TAG_BODY                = 0x017,
    MyHTML_TAG_BR                  = 0x018,
    MyHTML_TAG_BUTTON              = 0x019,
    MyHTML_TAG_CANVAS              = 0x01a,
    MyHTML_TAG_CAPTION             = 0x01b,
    MyHTML_TAG_CENTER              = 0x01c,
    MyHTML_TAG_CITE                = 0x01d,
    MyHTML_TAG_CODE                = 0x01e,
    MyHTML_TAG_COL                 = 0x01f,
    MyHTML_TAG_COLGROUP            = 0x020,
    MyHTML_TAG_COMMAND             = 0x021,
    MyHTML_TAG_COMMENT             = 0x022,
    MyHTML_TAG_DATALIST            = 0x023,
    MyHTML_TAG_DD                  = 0x024,
    MyHTML_TAG_DEL                 = 0x025,
    MyHTML_TAG_DETAILS             = 0x026,
    MyHTML_TAG_DFN                 = 0x027,
    MyHTML_TAG_DIALOG              = 0x028,
    MyHTML_TAG_DIR                 = 0x029,
    MyHTML_TAG_DIV                 = 0x02a,
    MyHTML_TAG_DL                  = 0x02b,
    MyHTML_TAG_DT                  = 0x02c,
    MyHTML_TAG_EM                  = 0x02d,
    MyHTML_TAG_EMBED               = 0x02e,
    MyHTML_TAG_FIELDSET            = 0x02f,
    MyHTML_TAG_FIGCAPTION          = 0x030,
    MyHTML_TAG_FIGURE              = 0x031,
    MyHTML_TAG_FONT                = 0x032,
    MyHTML_TAG_FOOTER              = 0x033,
    MyHTML_TAG_FORM                = 0x034,
    MyHTML_TAG_FRAME               = 0x035,
    MyHTML_TAG_FRAMESET            = 0x036,
    MyHTML_TAG_H1                  = 0x037,
    MyHTML_TAG_H2                  = 0x038,
    MyHTML_TAG_H3                  = 0x039,
    MyHTML_TAG_H4                  = 0x03a,
    MyHTML_TAG_H5                  = 0x03b,
    MyHTML_TAG_H6                  = 0x03c,
    MyHTML_TAG_HEAD                = 0x03d,
    MyHTML_TAG_HEADER              = 0x03e,
    MyHTML_TAG_HGROUP              = 0x03f,
    MyHTML_TAG_HR                  = 0x040,
    MyHTML_TAG_HTML                = 0x041,
    MyHTML_TAG_I                   = 0x042,
    MyHTML_TAG_IFRAME              = 0x043,
    MyHTML_TAG_IMAGE               = 0x044,
    MyHTML_TAG_IMG                 = 0x045,
    MyHTML_TAG_INPUT               = 0x046,
    MyHTML_TAG_INS                 = 0x047,
    MyHTML_TAG_ISINDEX             = 0x048,
    MyHTML_TAG_KBD                 = 0x049,
    MyHTML_TAG_KEYGEN              = 0x04a,
    MyHTML_TAG_LABEL               = 0x04b,
    MyHTML_TAG_LEGEND              = 0x04c,
    MyHTML_TAG_LI                  = 0x04d,
    MyHTML_TAG_LINK                = 0x04e,
    MyHTML_TAG_LISTING             = 0x04f,
    MyHTML_TAG_MAIN                = 0x050,
    MyHTML_TAG_MAP                 = 0x051,
    MyHTML_TAG_MARK                = 0x052,
    MyHTML_TAG_MARQUEE             = 0x053,
    MyHTML_TAG_MENU                = 0x054,
    MyHTML_TAG_MENUITEM            = 0x055,
    MyHTML_TAG_META                = 0x056,
    MyHTML_TAG_METER               = 0x057,
    MyHTML_TAG_MTEXT               = 0x058,
    MyHTML_TAG_NAV                 = 0x059,
    MyHTML_TAG_NOBR                = 0x05a,
    MyHTML_TAG_NOEMBED             = 0x05b,
    MyHTML_TAG_NOFRAMES            = 0x05c,
    MyHTML_TAG_NOSCRIPT            = 0x05d,
    MyHTML_TAG_OBJECT              = 0x05e,
    MyHTML_TAG_OL                  = 0x05f,
    MyHTML_TAG_OPTGROUP            = 0x060,
    MyHTML_TAG_OPTION              = 0x061,
    MyHTML_TAG_OUTPUT              = 0x062,
    MyHTML_TAG_P                   = 0x063,
    MyHTML_TAG_PARAM               = 0x064,
    MyHTML_TAG_PLAINTEXT           = 0x065,
    MyHTML_TAG_PRE                 = 0x066,
    MyHTML_TAG_PROGRESS            = 0x067,
    MyHTML_TAG_Q                   = 0x068,
    MyHTML_TAG_RB                  = 0x069,
    MyHTML_TAG_RP                  = 0x06a,
    MyHTML_TAG_RT                  = 0x06b,
    MyHTML_TAG_RTC                 = 0x06c,
    MyHTML_TAG_RUBY                = 0x06d,
    MyHTML_TAG_S                   = 0x06e,
    MyHTML_TAG_SAMP                = 0x06f,
    MyHTML_TAG_SCRIPT              = 0x070,
    MyHTML_TAG_SECTION             = 0x071,
    MyHTML_TAG_SELECT              = 0x072,
    MyHTML_TAG_SMALL               = 0x073,
    MyHTML_TAG_SOURCE              = 0x074,
    MyHTML_TAG_SPAN                = 0x075,
    MyHTML_TAG_STRIKE              = 0x076,
    MyHTML_TAG_STRONG              = 0x077,
    MyHTML_TAG_STYLE               = 0x078,
    MyHTML_TAG_SUB                 = 0x079,
    MyHTML_TAG_SUMMARY             = 0x07a,
    MyHTML_TAG_SUP                 = 0x07b,
    MyHTML_TAG_SVG                 = 0x07c,
    MyHTML_TAG_TABLE               = 0x07d,
    MyHTML_TAG_TBODY               = 0x07e,
    MyHTML_TAG_TD                  = 0x07f,
    MyHTML_TAG_TEMPLATE            = 0x080,
    MyHTML_TAG_TEXTAREA            = 0x081,
    MyHTML_TAG_TFOOT               = 0x082,
    MyHTML_TAG_TH                  = 0x083,
    MyHTML_TAG_THEAD               = 0x084,
    MyHTML_TAG_TIME                = 0x085,
    MyHTML_TAG_TITLE               = 0x086,
    MyHTML_TAG_TR                  = 0x087,
    MyHTML_TAG_TRACK               = 0x088,
    MyHTML_TAG_TT                  = 0x089,
    MyHTML_TAG_U                   = 0x08a,
    MyHTML_TAG_UL                  = 0x08b,
    MyHTML_TAG_VAR                 = 0x08c,
    MyHTML_TAG_VIDEO               = 0x08d,
    MyHTML_TAG_WBR                 = 0x08e,
    MyHTML_TAG_XMP                 = 0x08f,
    MyHTML_TAG_ALTGLYPH            = 0x090,
    MyHTML_TAG_ALTGLYPHDEF         = 0x091,
    MyHTML_TAG_ALTGLYPHITEM        = 0x092,
    MyHTML_TAG_ANIMATE             = 0x093,
    MyHTML_TAG_ANIMATECOLOR        = 0x094,
    MyHTML_TAG_ANIMATEMOTION       = 0x095,
    MyHTML_TAG_ANIMATETRANSFORM    = 0x096,
    MyHTML_TAG_CIRCLE              = 0x097,
    MyHTML_TAG_CLIPPATH            = 0x098,
    MyHTML_TAG_COLOR_PROFILE       = 0x099,
    MyHTML_TAG_CURSOR              = 0x09a,
    MyHTML_TAG_DEFS                = 0x09b,
    MyHTML_TAG_DESC                = 0x09c,
    MyHTML_TAG_ELLIPSE             = 0x09d,
    MyHTML_TAG_FEBLEND             = 0x09e,
    MyHTML_TAG_FECOLORMATRIX       = 0x09f,
    MyHTML_TAG_FECOMPONENTTRANSFER = 0x0a0,
    MyHTML_TAG_FECOMPOSITE         = 0x0a1,
    MyHTML_TAG_FECONVOLVEMATRIX    = 0x0a2,
    MyHTML_TAG_FEDIFFUSELIGHTING   = 0x0a3,
    MyHTML_TAG_FEDISPLACEMENTMAP   = 0x0a4,
    MyHTML_TAG_FEDISTANTLIGHT      = 0x0a5,
    MyHTML_TAG_FEDROPSHADOW        = 0x0a6,
    MyHTML_TAG_FEFLOOD             = 0x0a7,
    MyHTML_TAG_FEFUNCA             = 0x0a8,
    MyHTML_TAG_FEFUNCB             = 0x0a9,
    MyHTML_TAG_FEFUNCG             = 0x0aa,
    MyHTML_TAG_FEFUNCR             = 0x0ab,
    MyHTML_TAG_FEGAUSSIANBLUR      = 0x0ac,
    MyHTML_TAG_FEIMAGE             = 0x0ad,
    MyHTML_TAG_FEMERGE             = 0x0ae,
    MyHTML_TAG_FEMERGENODE         = 0x0af,
    MyHTML_TAG_FEMORPHOLOGY        = 0x0b0,
    MyHTML_TAG_FEOFFSET            = 0x0b1,
    MyHTML_TAG_FEPOINTLIGHT        = 0x0b2,
    MyHTML_TAG_FESPECULARLIGHTING  = 0x0b3,
    MyHTML_TAG_FESPOTLIGHT         = 0x0b4,
    MyHTML_TAG_FETILE              = 0x0b5,
    MyHTML_TAG_FETURBULENCE        = 0x0b6,
    MyHTML_TAG_FILTER              = 0x0b7,
    MyHTML_TAG_FONT_FACE           = 0x0b8,
    MyHTML_TAG_FONT_FACE_FORMAT    = 0x0b9,
    MyHTML_TAG_FONT_FACE_NAME      = 0x0ba,
    MyHTML_TAG_FONT_FACE_SRC       = 0x0bb,
    MyHTML_TAG_FONT_FACE_URI       = 0x0bc,
    MyHTML_TAG_FOREIGNOBJECT       = 0x0bd,
    MyHTML_TAG_G                   = 0x0be,
    MyHTML_TAG_GLYPH               = 0x0bf,
    MyHTML_TAG_GLYPHREF            = 0x0c0,
    MyHTML_TAG_HKERN               = 0x0c1,
    MyHTML_TAG_LINE                = 0x0c2,
    MyHTML_TAG_LINEARGRADIENT      = 0x0c3,
    MyHTML_TAG_MARKER              = 0x0c4,
    MyHTML_TAG_MASK                = 0x0c5,
    MyHTML_TAG_METADATA            = 0x0c6,
    MyHTML_TAG_MISSING_GLYPH       = 0x0c7,
    MyHTML_TAG_MPATH               = 0x0c8,
    MyHTML_TAG_PATH                = 0x0c9,
    MyHTML_TAG_PATTERN             = 0x0ca,
    MyHTML_TAG_POLYGON             = 0x0cb,
    MyHTML_TAG_POLYLINE            = 0x0cc,
    MyHTML_TAG_RADIALGRADIENT      = 0x0cd,
    MyHTML_TAG_RECT                = 0x0ce,
    MyHTML_TAG_SET                 = 0x0cf,
    MyHTML_TAG_STOP                = 0x0d0,
    MyHTML_TAG_SWITCH              = 0x0d1,
    MyHTML_TAG_SYMBOL              = 0x0d2,
    MyHTML_TAG_TEXT                = 0x0d3,
    MyHTML_TAG_TEXTPATH            = 0x0d4,
    MyHTML_TAG_TREF                = 0x0d5,
    MyHTML_TAG_TSPAN               = 0x0d6,
    MyHTML_TAG_USE                 = 0x0d7,
    MyHTML_TAG_VIEW                = 0x0d8,
    MyHTML_TAG_VKERN               = 0x0d9,
    MyHTML_TAG_MATH                = 0x0da,
    MyHTML_TAG_MACTION             = 0x0db,
    MyHTML_TAG_MALIGNGROUP         = 0x0dc,
    MyHTML_TAG_MALIGNMARK          = 0x0dd,
    MyHTML_TAG_MENCLOSE            = 0x0de,
    MyHTML_TAG_MERROR              = 0x0df,
    MyHTML_TAG_MFENCED             = 0x0e0,
    MyHTML_TAG_MFRAC               = 0x0e1,
    MyHTML_TAG_MGLYPH              = 0x0e2,
    MyHTML_TAG_MI                  = 0x0e3,
    MyHTML_TAG_MLABELEDTR          = 0x0e4,
    MyHTML_TAG_MLONGDIV            = 0x0e5,
    MyHTML_TAG_MMULTISCRIPTS       = 0x0e6,
    MyHTML_TAG_MN                  = 0x0e7,
    MyHTML_TAG_MO                  = 0x0e8,
    MyHTML_TAG_MOVER               = 0x0e9,
    MyHTML_TAG_MPADDED             = 0x0ea,
    MyHTML_TAG_MPHANTOM            = 0x0eb,
    MyHTML_TAG_MROOT               = 0x0ec,
    MyHTML_TAG_MROW                = 0x0ed,
    MyHTML_TAG_MS                  = 0x0ee,
    MyHTML_TAG_MSCARRIES           = 0x0ef,
    MyHTML_TAG_MSCARRY             = 0x0f0,
    MyHTML_TAG_MSGROUP             = 0x0f1,
    MyHTML_TAG_MSLINE              = 0x0f2,
    MyHTML_TAG_MSPACE              = 0x0f3,
    MyHTML_TAG_MSQRT               = 0x0f4,
    MyHTML_TAG_MSROW               = 0x0f5,
    MyHTML_TAG_MSTACK              = 0x0f6,
    MyHTML_TAG_MSTYLE              = 0x0f7,
    MyHTML_TAG_MSUB                = 0x0f8,
    MyHTML_TAG_MSUP                = 0x0f9,
    MyHTML_TAG_MSUBSUP             = 0x0fa,
    MyHTML_TAG__END_OF_FILE        = 0x0fb,
    MyHTML_TAG_FIRST_ENTRY         = MyHTML_TAG__TEXT,
    MyHTML_TAG_LAST_ENTRY          = 0x0fc
};

/**
 * @struct myhtml statuses
 */
// base
/*
 Very important!!!
 
 for myhtml             0..00ffff;      MyHTML_STATUS_OK    == 0x000000
 for mycss and modules  010000..01ffff; MyCSS_STATUS_OK     == 0x000000
 for modest             020000..02ffff; MODEST_STATUS_OK    == 0x000000
 for myrender           030000..03ffff; MyRENDER_STATUS_OK  == 0x000000
 for mydom              040000..04ffff; MyDOM_STATUS_OK     == 0x000000
 for mynetwork          050000..05ffff; MyNETWORK_STATUS_OK == 0x000000
 for myecma             060000..06ffff; MyECMA_STATUS_OK    == 0x000000
 not occupied           070000..
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

#define MYHTML_FAILED(_status_) ((_status_) != MyHTML_STATUS_OK) /* deprecated */
#define MyHTML_FAILED(_status_) ((_status_) != MyHTML_STATUS_OK)

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
    
    /* MyHTML_NAMESPACE_ANY == MyHTML_NAMESPACE_LAST_ENTRY */
    MyHTML_NAMESPACE_ANY        = 0x07,
    MyHTML_NAMESPACE_LAST_ENTRY = 0x07
}
typedef myhtml_namespace_t;

/**
 * @struct myhtml options
 */
enum myhtml_options {
    MyHTML_OPTIONS_DEFAULT                 = 0x00,
    MyHTML_OPTIONS_PARSE_MODE_SINGLE       = 0x01,
    MyHTML_OPTIONS_PARSE_MODE_ALL_IN_ONE   = 0x02,
    MyHTML_OPTIONS_PARSE_MODE_SEPARATELY   = 0x04
};

/**
 * @struct myhtml_tree parse flags
 */
enum myhtml_tree_parse_flags {
    MyHTML_TREE_PARSE_FLAGS_CLEAN                   = 0x000,
    MyHTML_TREE_PARSE_FLAGS_WITHOUT_BUILD_TREE      = 0x001,
    MyHTML_TREE_PARSE_FLAGS_WITHOUT_PROCESS_TOKEN   = 0x003,
    MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN   = 0x004, /* skip ws token, but not for RCDATA, RAWTEXT, CDATA and PLAINTEXT */
    MyHTML_TREE_PARSE_FLAGS_WITHOUT_DOCTYPE_IN_TREE = 0x008
}
typedef myhtml_tree_parse_flags_t;

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

/**
 * @struct myhtml_token_t MyHTML_TOKEN
 */
typedef struct myhtml_token myhtml_token_t;

typedef struct myhtml_token_attr myhtml_tree_attr_t;
typedef struct myhtml_tree_node myhtml_tree_node_t;

/**
 * MyHTML_TAG
 *
 */
typedef size_t myhtml_tag_id_t;
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

/**
 * @struct myhtml_position_t
 */
struct myhtml_position {
    size_t begin;
    size_t length;
}
typedef myhtml_position_t;

/**
 * @struct myhtml_token_node_t
 */
typedef struct myhtml_token_node myhtml_token_node_t;

/**
 * @struct myhtml_version_t
 */
struct myhtml_version {
    int major;
    int minor;
    int patch;
}
typedef myhtml_version_t;
    
// callback functions
typedef void* (*myhtml_callback_token_f)(myhtml_tree_t* tree, myhtml_token_node_t* token, void* ctx);
typedef void (*myhtml_callback_tree_node_f)(myhtml_tree_t* tree, myhtml_tree_node_t* node, void* ctx);

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
mystatus_t
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
 * @param[in] Input character encoding; Default: MyENCODING_UTF_8 or MyENCODING_DEFAULT or 0
 * @param[in] HTML
 * @param[in] HTML size
 *
 * All input character encoding decode to utf-8
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
mystatus_t
myhtml_parse(myhtml_tree_t* tree, myencoding_t encoding,
             const char* html, size_t html_size);

/**
 * Parsing fragment of HTML
 *
 * @param[in] previously created structure myhtml_tree_t*
 * @param[in] Input character encoding; Default: MyENCODING_UTF_8 or MyENCODING_DEFAULT or 0
 * @param[in] HTML
 * @param[in] HTML size
 * @param[in] fragment base (root) tag id. Default: MyHTML_TAG_DIV if set 0
 * @param[in] fragment NAMESPACE. Default: MyHTML_NAMESPACE_HTML if set 0
 *
 * All input character encoding decode to utf-8
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
mystatus_t
myhtml_parse_fragment(myhtml_tree_t* tree, myencoding_t encoding,
                      const char* html, size_t html_size,
                      myhtml_tag_id_t tag_id, enum myhtml_namespace ns);

/**
 * Parsing HTML in Single Mode. 
 * No matter what was said during initialization MyHTML
 *
 * @param[in] previously created structure myhtml_tree_t*
 * @param[in] Input character encoding; Default: MyENCODING_UTF_8 or MyENCODING_DEFAULT or 0
 * @param[in] HTML
 * @param[in] HTML size
 *
 * All input character encoding decode to utf-8
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
mystatus_t
myhtml_parse_single(myhtml_tree_t* tree, myencoding_t encoding,
                    const char* html, size_t html_size);

/**
 * Parsing fragment of HTML in Single Mode. 
 * No matter what was said during initialization MyHTML
 *
 * @param[in] previously created structure myhtml_tree_t*
 * @param[in] Input character encoding; Default: MyENCODING_UTF_8 or MyENCODING_DEFAULT or 0
 * @param[in] HTML
 * @param[in] HTML size
 * @param[in] fragment base (root) tag id. Default: MyHTML_TAG_DIV if set 0
 * @param[in] fragment NAMESPACE. Default: MyHTML_NAMESPACE_HTML if set 0
 *
 * All input character encoding decode to utf-8
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
mystatus_t
myhtml_parse_fragment_single(myhtml_tree_t* tree, myencoding_t encoding,
                             const char* html, size_t html_size,
                             myhtml_tag_id_t tag_id, enum myhtml_namespace ns);

/**
 * Parsing HTML chunk. For end parsing call myhtml_parse_chunk_end function
 *
 * @param[in] myhtml_tree_t*
 * @param[in] HTML
 * @param[in] HTML size
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
mystatus_t
myhtml_parse_chunk(myhtml_tree_t* tree, const char* html, size_t html_size);

/**
 * Parsing chunk of fragment HTML. For end parsing call myhtml_parse_chunk_end function
 *
 * @param[in] myhtml_tree_t*
 * @param[in] HTML
 * @param[in] HTML size
 * @param[in] fragment base (root) tag id. Default: MyHTML_TAG_DIV if set 0
 * @param[in] fragment NAMESPACE. Default: MyHTML_NAMESPACE_HTML if set 0
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
mystatus_t
myhtml_parse_chunk_fragment(myhtml_tree_t* tree, const char* html,size_t html_size,
                            myhtml_tag_id_t tag_id, enum myhtml_namespace ns);

/**
 * Parsing HTML chunk in Single Mode.
 * No matter what was said during initialization MyHTML
 *
 * @param[in] myhtml_tree_t*
 * @param[in] HTML
 * @param[in] HTML size
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
mystatus_t
myhtml_parse_chunk_single(myhtml_tree_t* tree, const char* html, size_t html_size);

/**
 * Parsing chunk of fragment of HTML in Single Mode.
 * No matter what was said during initialization MyHTML
 *
 * @param[in] myhtml_tree_t*
 * @param[in] HTML
 * @param[in] HTML size
 * @param[in] fragment base (root) tag id. Default: MyHTML_TAG_DIV if set 0
 * @param[in] fragment NAMESPACE. Default: MyHTML_NAMESPACE_HTML if set 0
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
mystatus_t
myhtml_parse_chunk_fragment_single(myhtml_tree_t* tree, const char* html, size_t html_size,
                                   myhtml_tag_id_t tag_id, enum myhtml_namespace ns);

/**
 * End of parsing HTML chunks
 *
 * @param[in] myhtml_tree_t*
 *
 * @return MyHTML_STATUS_OK if successful, otherwise an error status
 */
mystatus_t
myhtml_parse_chunk_end(myhtml_tree_t* tree);

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
mystatus_t
myhtml_tree_init(myhtml_tree_t* tree, myhtml_t* myhtml);

/**
 * Get Parse Flags of Tree
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_tree_parse_flags_t
 */
myhtml_tree_parse_flags_t
myhtml_tree_parse_flags(myhtml_tree_t* tree);

/**
 * Set Parse Flags for Tree
 * See enum myhtml_tree_parse_flags in this file
 *
 * @example myhtml_tree_parse_flags_set(tree, MyHTML_TREE_PARSE_FLAGS_WITHOUT_BUILD_TREE|
 *                                            MyHTML_TREE_PARSE_FLAGS_WITHOUT_DOCTYPE_IN_TREE|
 *                                            MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN);
 *
 * @param[in] myhtml_tree_t*
 * @param[in] parse flags. You can combine their
 */
void
myhtml_tree_parse_flags_set(myhtml_tree_t* tree, myhtml_tree_parse_flags_t parse_flags);

/**
 * Clears resources before new parsing
 *
 * @param[in] myhtml_tree_t*
 */
void
myhtml_tree_clean(myhtml_tree_t* tree);

/**
 * Add child node to node. If children already exists it will be added to the last
 *
 * @param[in] myhtml_tree_node_t* The node to which we add child node
 * @param[in] myhtml_tree_node_t* The node which adds
 */
void
myhtml_tree_node_add_child(myhtml_tree_node_t* root, myhtml_tree_node_t* node);

/**
 * Add a node immediately before the existing node
 *
 * @param[in] myhtml_tree_node_t* add for this node
 * @param[in] myhtml_tree_node_t* add this node
 */
void
myhtml_tree_node_insert_before(myhtml_tree_node_t* root, myhtml_tree_node_t* node);

/**
 * Add a node immediately after the existing node
 *
 * @param[in] myhtml_tree_node_t* add for this node
 * @param[in] myhtml_tree_node_t* add this node
 */
void
myhtml_tree_node_insert_after(myhtml_tree_node_t* root, myhtml_tree_node_t* node);

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
 * Get Tree Document (Root of Tree)
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_tree_get_document(myhtml_tree_t* tree);

/**
 * Get node HTML (Document -> HTML, Root of HTML Document)
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_tree_get_node_html(myhtml_tree_t* tree);

/**
 * Get node HEAD (Document -> HTML -> HEAD)
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_tree_get_node_head(myhtml_tree_t* tree);

/**
 * Get node BODY (Document -> HTML -> BODY)
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_tree_get_node_body(myhtml_tree_t* tree);

/**
 * Get mchar_async_t object
 *
 * @param[in] myhtml_tree_t*
 *
 * @return mchar_async_t* if exists, otherwise a NULL value
 */
mchar_async_t*
myhtml_tree_get_mchar(myhtml_tree_t* tree);

/**
 * Get node_id from main thread for mchar_async_t object
 *
 * @param[in] myhtml_tree_t*
 *
 * @return size_t, node id
 */
size_t
myhtml_tree_get_mchar_node_id(myhtml_tree_t* tree);

/**
 * Get first Incoming Buffer
 *
 * @param[in] myhtml_tree_t*
 *
 * @return mycore_incoming_buffer_t* if successful, otherwise a NULL value
 */
mycore_incoming_buffer_t*
myhtml_tree_incoming_buffer_first(myhtml_tree_t *tree);

/***********************************************************************************
 *
 * MyHTML_NODE
 *
 ***********************************************************************************/

/**
 * Get first (begin) node of tree
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_node_first(myhtml_tree_t* tree);

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
                           myhtml_tag_id_t tag_id, mystatus_t *status);

/**
 * Get nodes by tag name
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, creates new collection if NULL
 * @param[in] tag name
 * @param[in] tag name length
 * @param[out] status of this operation, optional
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_name(myhtml_tree_t* tree, myhtml_collection_t *collection,
                         const char* name, size_t length, mystatus_t *status);

/**
 * Get nodes by attribute key
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, optional; creates new collection if NULL
 * @param[in] myhtml_tree_node_t*, optional; scope node; html if NULL
 * @param[in] find key
 * @param[in] find key length
 * @param[out] status of this operation, optional
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_attribute_key(myhtml_tree_t *tree, myhtml_collection_t* collection,
                                  myhtml_tree_node_t* scope_node,
                                  const char* key, size_t key_len, mystatus_t* status);

/**
 * Get nodes by attribute value; exactly equal; like a [foo="bar"]
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, optional; creates new collection if NULL
 * @param[in] myhtml_tree_node_t*, optional; scope node; html if NULL
 * @param[in] case-insensitive if true
 * @param[in] find in key; if NULL find in all attributes
 * @param[in] find in key length; if 0 find in all attributes
 * @param[in] find value
 * @param[in] find value length
 * @param[out] status of this operation, optional
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_attribute_value(myhtml_tree_t *tree,
                                    myhtml_collection_t* collection,
                                    myhtml_tree_node_t* node,
                                    bool case_insensitive,
                                    const char* key, size_t key_len,
                                    const char* value, size_t value_len,
                                    mystatus_t* status);

/**
 * Get nodes by attribute value; whitespace separated; like a [foo~="bar"]
 *
 * @example if value="bar" and node attr value="lalala bar bebebe", then this node is found
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, optional; creates new collection if NULL
 * @param[in] myhtml_tree_node_t*, optional; scope node; html if NULL
 * @param[in] case-insensitive if true
 * @param[in] find in key; if NULL find in all attributes
 * @param[in] find in key length; if 0 find in all attributes
 * @param[in] find value
 * @param[in] find value length
 * @param[out] status of this operation, optional
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_attribute_value_whitespace_separated(myhtml_tree_t *tree,
                                                         myhtml_collection_t* collection,
                                                         myhtml_tree_node_t* node,
                                                         bool case_insensitive,
                                                         const char* key, size_t key_len,
                                                         const char* value, size_t value_len,
                                                         mystatus_t* status);

/**
 * Get nodes by attribute value; value begins exactly with the string; like a [foo^="bar"]
 *
 * @example if value="bar" and node attr value="barmumumu", then this node is found
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, optional; creates new collection if NULL
 * @param[in] myhtml_tree_node_t*, optional; scope node; html if NULL
 * @param[in] case-insensitive if true
 * @param[in] find in key; if NULL find in all attributes
 * @param[in] find in key length; if 0 find in all attributes
 * @param[in] find value
 * @param[in] find value length
 * @param[out] status of this operation, optional
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_attribute_value_begin(myhtml_tree_t *tree,
                                          myhtml_collection_t* collection,
                                          myhtml_tree_node_t* node,
                                          bool case_insensitive,
                                          const char* key, size_t key_len,
                                          const char* value, size_t value_len,
                                          mystatus_t* status);


/**
 * Get nodes by attribute value; value ends exactly with the string; like a [foo$="bar"]
 *
 * @example if value="bar" and node attr value="mumumubar", then this node is found
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, optional; creates new collection if NULL
 * @param[in] myhtml_tree_node_t*, optional; scope node; html if NULL
 * @param[in] case-insensitive if true
 * @param[in] find in key; if NULL find in all attributes
 * @param[in] find in key length; if 0 find in all attributes
 * @param[in] find value
 * @param[in] find value length
 * @param[out] status of this operation, optional
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_attribute_value_end(myhtml_tree_t *tree,
                                        myhtml_collection_t* collection,
                                        myhtml_tree_node_t* node,
                                        bool case_insensitive,
                                        const char* key, size_t key_len,
                                        const char* value, size_t value_len,
                                        mystatus_t* status);

/**
 * Get nodes by attribute value; value contains the substring; like a [foo*="bar"]
 *
 * @example if value="bar" and node attr value="bububarmumu", then this node is found
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, optional; creates new collection if NULL
 * @param[in] myhtml_tree_node_t*, optional; scope node; html if NULL
 * @param[in] case-insensitive if true
 * @param[in] find in key; if NULL find in all attributes
 * @param[in] find in key length; if 0 find in all attributes
 * @param[in] find value
 * @param[in] find value length
 * @param[out] status of this operation, optional
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_attribute_value_contain(myhtml_tree_t *tree,
                                            myhtml_collection_t* collection,
                                            myhtml_tree_node_t* node,
                                            bool case_insensitive,
                                            const char* key, size_t key_len,
                                            const char* value, size_t value_len,
                                            mystatus_t* status);

/**
 * Get nodes by attribute value; attribute value is a hyphen-separated list of values beginning; 
 * like a [foo|="bar"]
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, optional; creates new collection if NULL
 * @param[in] myhtml_tree_node_t*, optional; scope node; html if NULL
 * @param[in] case-insensitive if true
 * @param[in] find in key; if NULL find in all attributes
 * @param[in] find in key length; if 0 find in all attributes
 * @param[in] find value
 * @param[in] find value length
 * @param[out] optional; status of this operation
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_attribute_value_hyphen_separated(myhtml_tree_t *tree,
                                                     myhtml_collection_t* collection,
                                                     myhtml_tree_node_t* node,
                                                     bool case_insensitive,
                                                     const char* key, size_t key_len,
                                                     const char* value, size_t value_len,
                                                     mystatus_t* status);

/**
 * Get nodes by tag id in node scope
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, creates new collection if NULL
 * @param[in] node for search tag_id in children nodes
 * @param[in] tag_id for search
 * @param[out] status of this operation
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_tag_id_in_scope(myhtml_tree_t* tree, myhtml_collection_t *collection,
                                    myhtml_tree_node_t *node, myhtml_tag_id_t tag_id,
                                    mystatus_t *status);

/**
 * Get nodes by tag name in node scope
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_collection_t*, creates new collection if NULL
 * @param[in] node for search tag_id in children nodes
 * @param[in] tag name
 * @param[in] tag name length
 * @param[out] status of this operation
 *
 * @return myhtml_collection_t* if successful, otherwise an NULL value
 */
myhtml_collection_t*
myhtml_get_nodes_by_name_in_scope(myhtml_tree_t* tree, myhtml_collection_t *collection,
                                  myhtml_tree_node_t *node, const char* html, size_t length,
                                  mystatus_t *status);

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
 * Create new node
 *
 * @param[in] myhtml_tree_t*
 * @param[in] tag id, see enum myhtml_tags
 * @param[in] enum myhtml_namespace
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_node_create(myhtml_tree_t* tree, myhtml_tag_id_t tag_id,
                   enum myhtml_namespace ns);

/**
 * Cloning a node
 *
 * @param[in] the tree into which the cloned node will be inserted. myhtml_tree_t*
 * @param[in] cloning node
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t *
myhtml_node_clone(myhtml_tree_t* dest_tree, myhtml_tree_node_t* src);

/**
 * Cloning a node with all children
 *
 * @param[in] the tree into which the cloned node will be inserted. myhtml_tree_t*
 * @param[in] cloning node
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t *
myhtml_node_clone_deep(myhtml_tree_t* dest_tree, myhtml_tree_node_t* src);

/**
 * Release allocated resources
 *
 * @param[in] myhtml_tree_node_t*
 */
void
myhtml_node_free(myhtml_tree_node_t *node);

/**
 * Remove node of tree
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_node_t* if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_node_remove(myhtml_tree_node_t *node);

/**
 * Remove node of tree and release allocated resources
 *
 * @param[in] myhtml_tree_node_t*
 */
void
myhtml_node_delete(myhtml_tree_node_t *node);

/**
 * Remove nodes of tree recursively and release allocated resources
 *
 * @param[in] myhtml_tree_node_t*
 */
void
myhtml_node_delete_recursive(myhtml_tree_node_t *node);

/**
 * The appropriate place for inserting a node. Insertion with validation.
 * If try insert <a> node to <table> node, then <a> node inserted before <table> node
 *
 * @param[in] target node
 * @param[in] insertion node
 *
 * @return insertion node if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_node_insert_to_appropriate_place(myhtml_tree_node_t *target, myhtml_tree_node_t *node);

/**
 * Append to target node as last child. Insertion without validation.
 *
 * @param[in] target node
 * @param[in] insertion node
 *
 * @return insertion node if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_node_append_child(myhtml_tree_node_t *target, myhtml_tree_node_t *node);

/**
 * Append sibling node after target node. Insertion without validation.
 *
 * @param[in] target node
 * @param[in] insertion node
 *
 * @return insertion node if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_node_insert_after(myhtml_tree_node_t *target, myhtml_tree_node_t *node);

/**
 * Append sibling node before target node. Insertion without validation.
 *
 * @param[in] target node
 * @param[in] insertion node
 *
 * @return insertion node if successful, otherwise a NULL value
 */
myhtml_tree_node_t*
myhtml_node_insert_before(myhtml_tree_node_t *target, myhtml_tree_node_t *node);

/**
 * Add text for a node with convert character encoding.
 *
 * @param[in] target node
 * @param[in] text
 * @param[in] text length
 * @param[in] character encoding
 *
 * @return mycore_string_t* if successful, otherwise a NULL value
 */
mycore_string_t*
myhtml_node_text_set(myhtml_tree_node_t *node, const char* text, size_t length,
                     myencoding_t encoding);

/**
 * Add text for a node with convert character encoding.
 *
 * @param[in] target node
 * @param[in] text
 * @param[in] text length
 * @param[in] character encoding
 *
 * @return mycore_string_t* if successful, otherwise a NULL value
 */
mycore_string_t*
myhtml_node_text_set_with_charef(myhtml_tree_node_t *node, const char* text, size_t length,
                                 myencoding_t encoding);

/**
 * Get token node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_token_node_t*
 */
myhtml_token_node_t*
myhtml_node_token(myhtml_tree_node_t *node);

/**
 * Get node namespace
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_namespace_t
 */
myhtml_namespace_t
myhtml_node_namespace(myhtml_tree_node_t *node);

/**
 * Set node namespace
 *
 * @param[in] myhtml_tree_node_t*
 * @param[in] myhtml_namespace_t
 */
void
myhtml_node_namespace_set(myhtml_tree_node_t *node, myhtml_namespace_t ns);

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
 * Node has self-closing flag?
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return true or false (1 or 0)
 */
bool
myhtml_node_is_close_self(myhtml_tree_node_t *node);

/**
 * Node is a void element?
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return true or false (1 or 0)
 */
bool
myhtml_node_is_void_element(myhtml_tree_node_t *node);

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

/**
 * Get mycore_string_t object by Tree node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return mycore_string_t* if exists, otherwise an NULL value
 */
mycore_string_t*
myhtml_node_string(myhtml_tree_node_t *node);

/**
 * Get raw position for Tree Node in Incoming Buffer
 *
 * @example <[BEGIN]div[LENGTH] attr=lalala>
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_node_t
 */
myhtml_position_t
myhtml_node_raw_position(myhtml_tree_node_t *node);

/**
 * Get element position for Tree Node in Incoming Buffer
 *
 * @example [BEGIN]<div attr=lalala>[LENGTH]
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_node_t
 */
myhtml_position_t
myhtml_node_element_position(myhtml_tree_node_t *node);

/**
 * Get data value from tree node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return void*
 */
void*
myhtml_node_get_data(myhtml_tree_node_t *node);

/**
 * Set data value to tree node
 *
 * @param[in] myhtml_tree_node_t*
 * @param[in] void*
 */
void
myhtml_node_set_data(myhtml_tree_node_t *node, void* data);

/**
 * Get current tree (myhtml_tree_t*) from node
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return myhtml_tree_t*
 */
myhtml_tree_t*
myhtml_node_tree(myhtml_tree_node_t *node);

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
myhtml_namespace_t
myhtml_attribute_namespace(myhtml_tree_attr_t *attr);

/**
 * Set attribute namespace
 *
 * @param[in] myhtml_tree_attr_t*
 * @param[in] myhtml_namespace_t
 */
void
myhtml_attribute_namespace_set(myhtml_tree_attr_t *attr, myhtml_namespace_t ns);

/**
 * Get attribute key
 *
 * @param[in] myhtml_tree_attr_t*
 * @param[out] optional, name length
 *
 * @return const char* if exists, otherwise an NULL value
 */
const char*
myhtml_attribute_key(myhtml_tree_attr_t *attr, size_t *length);

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
 * Get attribute key string
 *
 * @param[in] myhtml_tree_attr_t*
 *
 * @return mycore_string_t* if exists, otherwise an NULL value
 */
mycore_string_t*
myhtml_attribute_key_string(myhtml_tree_attr_t* attr);

/**
 * Get attribute value string
 *
 * @param[in] myhtml_tree_attr_t*
 *
 * @return mycore_string_t* if exists, otherwise an NULL value
 */
mycore_string_t*
myhtml_attribute_value_string(myhtml_tree_attr_t* attr);

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
 * @param[in] myhtml_tree_node_t*
 * @param[in] attr key name
 * @param[in] attr key name length
 * @param[in] attr value name
 * @param[in] attr value name length
 * @param[in] character encoding; Default: MyENCODING_UTF_8 or MyENCODING_DEFAULT or 0
 *
 * @return created myhtml_tree_attr_t* if successful, otherwise a NULL value
 */
myhtml_tree_attr_t*
myhtml_attribute_add(myhtml_tree_node_t *node,
                     const char *key, size_t key_len,
                     const char *value, size_t value_len,
                     myencoding_t encoding);

/**
 * Remove attribute reference. Not release the resources
 *
 * @param[in] myhtml_tree_node_t*
 * @param[in] myhtml_tree_attr_t*
 *
 * @return myhtml_tree_attr_t* if successful, otherwise a NULL value
 */
myhtml_tree_attr_t*
myhtml_attribute_remove(myhtml_tree_node_t *node, myhtml_tree_attr_t *attr);

/**
 * Remove attribute by key reference. Not release the resources
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

/**
 * Get raw position for Attribute Key in Incoming Buffer
 *
 * @param[in] myhtml_tree_attr_t*
 *
 * @return myhtml_position_t
 */
myhtml_position_t
myhtml_attribute_key_raw_position(myhtml_tree_attr_t *attr);

/**
 * Get raw position for Attribute Value in Incoming Buffer
 *
 * @param[in] myhtml_tree_attr_t*
 *
 * @return myhtml_position_t
 */
myhtml_position_t
myhtml_attribute_value_raw_position(myhtml_tree_attr_t *attr);

/***********************************************************************************
 *
 * MyHTML_TOKEN_NODE
 *
 ***********************************************************************************/

/**
 * Get token node tag id
 *
 * @param[in] myhtml_token_node_t*
 *
 * @return myhtml_tag_id_t
 */
myhtml_tag_id_t
myhtml_token_node_tag_id(myhtml_token_node_t *token_node);

/**
 * Get raw position for Token Node in Incoming Buffer
 *
 * @example <[BEGIN]div[LENGTH] attr=lalala>
 *
 * @param[in] myhtml_token_node_t*
 *
 * @return myhtml_position_t
 */
myhtml_position_t
myhtml_token_node_raw_position(myhtml_token_node_t *token_node);

/**
 * Get element position for Token Node in Incoming Buffer
 *
 * @example [BEGIN]<div attr=lalala>[LENGTH]
 *
 * @param[in] myhtml_token_node_t*
 *
 * @return myhtml_position_t
 */
myhtml_position_t
myhtml_token_node_element_position(myhtml_token_node_t *token_node);

/**
 * Get first attribute of a token node
 *
 * @param[in] myhtml_token_node_t*
 *
 * @return myhtml_tree_attr_t* if exists, otherwise an NULL value
 */
myhtml_tree_attr_t*
myhtml_token_node_attribute_first(myhtml_token_node_t *token_node);

/**
 * Get last attribute of a token node
 *
 * @param[in] myhtml_token_node_t*
 *
 * @return myhtml_tree_attr_t* if exists, otherwise an NULL value
 */
myhtml_tree_attr_t*
myhtml_token_node_attribute_last(myhtml_token_node_t *token_node);

/**
 * Get text of a token node. Only for a MyHTML_TAG__TEXT or MyHTML_TAG__COMMENT tags
 *
 * @param[in] myhtml_token_node_t*
 * @param[out] optional, text length
 *
 * @return const char* if exists, otherwise an NULL value
 */
const char*
myhtml_token_node_text(myhtml_token_node_t *token_node, size_t *length);

/**
 * Get mycore_string_t object by token node
 *
 * @param[in] myhtml_token_node_t*
 *
 * @return mycore_string_t* if exists, otherwise an NULL value
 */
mycore_string_t*
myhtml_token_node_string(myhtml_token_node_t *token_node);

/**
 * Token node has closing flag?
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return true or false
 */
bool
myhtml_token_node_is_close(myhtml_token_node_t *token_node);

/**
 * Token node has self-closing flag?
 *
 * @param[in] myhtml_tree_node_t*
 *
 * @return true or false (1 or 0)
 */
bool
myhtml_token_node_is_close_self(myhtml_token_node_t *token_node);

/**
 * Wait for process token all parsing stage. Need if you use thread mode
 *
 * @param[in] myhtml_token_t*
 * @param[in] myhtml_token_node_t*
 */
void
myhtml_token_node_wait_for_done(myhtml_token_t* token, myhtml_token_node_t* node);

/***********************************************************************************
 *
 * MyHTML_TAG
 *
 ***********************************************************************************/

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
 * Get tag id by name
 *
 * @param[in] myhtml_tree_t*
 * @param[in] tag name
 * @param[in] tag name length
 *
 * @return tag id
 */
myhtml_tag_id_t
myhtml_tag_id_by_name(myhtml_tree_t* tree,
                      const char *tag_name, size_t length);

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
myhtml_collection_create(size_t size, mystatus_t *status);

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
 * @param[in] need nodes
 * @param[in] upto_length: count for up if nodes not exists 
 *            (current length + need + upto_length + 1)
 *
 * @return NULL if successful, otherwise an myhtml_collection_t* structure
 */
mystatus_t
myhtml_collection_check_size(myhtml_collection_t *collection, size_t need, size_t upto_length);

/***********************************************************************************
 *
 * MyHTML_ENCODING
 *
 ***********************************************************************************/

/**
 * Set character encoding for input stream
 *
 * @param[in] myhtml_tree_t*
 * @param[in] Input character encoding
 *
 */
void
myhtml_encoding_set(myhtml_tree_t* tree, myencoding_t encoding);

/**
 * Get character encoding for current stream
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myencoding_t
 */
myencoding_t
myhtml_encoding_get(myhtml_tree_t* tree);

/**
 * Convert Unicode Codepoint to UTF-8
 *
 * @param[in] Codepoint
 * @param[in] Data to set characters. Minimum data length is 1 bytes, maximum is 4 byte
 *   data length must be always available 4 bytes
 *
 * @return size character set
 */
size_t
myencoding_codepoint_to_ascii_utf_8(size_t codepoint, char *data);

/**
 * Convert Unicode Codepoint to UTF-16LE
 *
 * I advise not to use UTF-16! Use UTF-8 and be happy!
 *
 * @param[in] Codepoint
 * @param[in] Data to set characters. Data length is 2 or 4 bytes
 *   data length must be always available 4 bytes
 *
 * @return size character set
 */
size_t
myencoding_codepoint_to_ascii_utf_16(size_t codepoint, char *data);

/**
 * Detect character encoding
 *
 * Now available for detect UTF-8, UTF-16LE, UTF-16BE
 * and Russians: windows-1251,  koi8-r, iso-8859-5, x-mac-cyrillic, ibm866
 * Other in progress
 *
 * @param[in]  text
 * @param[in]  text length
 * @param[out] detected encoding
 *
 * @return true if encoding found, otherwise false
 */
bool
myencoding_detect(const char *text, size_t length, myencoding_t *encoding);

/**
 * Detect Russian character encoding
 *
 * Now available for detect windows-1251,  koi8-r, iso-8859-5, x-mac-cyrillic, ibm866
 *
 * @param[in]  text
 * @param[in]  text length
 * @param[out] detected encoding
 *
 * @return true if encoding found, otherwise false
 */
bool
myencoding_detect_russian(const char *text, size_t length, myencoding_t *encoding);

/**
 * Detect Unicode character encoding
 *
 * Now available for detect UTF-8, UTF-16LE, UTF-16BE
 *
 * @param[in]  text
 * @param[in]  text length
 * @param[out] detected encoding
 *
 * @return true if encoding found, otherwise false
 */
bool
myencoding_detect_unicode(const char *text, size_t length, myencoding_t *encoding);

/**
 * Detect Unicode character encoding by BOM
 *
 * Now available for detect UTF-8, UTF-16LE, UTF-16BE
 *
 * @param[in]  text
 * @param[in]  text length
 * @param[out] detected encoding
 *
 * @return true if encoding found, otherwise false
 */
bool
myencoding_detect_bom(const char *text, size_t length, myencoding_t *encoding);

/**
 * Detect Unicode character encoding by BOM. Cut BOM if will be found
 *
 * Now available for detect UTF-8, UTF-16LE, UTF-16BE
 *
 * @param[in]  text
 * @param[in]  text length
 * @param[out] detected encoding
 * @param[out] new text position
 * @param[out] new size position
 *
 * @return true if encoding found, otherwise false
 */
bool
myencoding_detect_and_cut_bom(const char *text, size_t length, myencoding_t *encoding,
                                   const char **new_text, size_t *new_size);

/**
 * Detect encoding by name
 * Names like: windows-1258 return MyENCODING_WINDOWS_1258
 *             cp1251 or windows-1251 return MyENCODING_WINDOWS_1251
 *
 * See https://encoding.spec.whatwg.org/#names-and-labels
 *
 * @param[in]  name
 * @param[in]  name length
 * @param[out] detected encoding
 *
 * @return true if encoding found, otherwise false
 */
bool
myencoding_by_name(const char *name, size_t length, myencoding_t *encoding);

/**
 * Get Encoding name by myencoding_t (by id)
 *
 * @param[in]  myencoding_t, encoding id
 * @param[out] return name length
 *
 * @return encoding name, otherwise NULL value
 */
const char*
myencoding_name_by_id(myencoding_t encoding, size_t *length);

/**
 * Detect encoding in meta tag (<meta ...>) before start parsing
 *
 * See https://html.spec.whatwg.org/multipage/syntax.html#prescan-a-byte-stream-to-determine-its-encoding
 *
 * @param[in]  html data bytes
 * @param[in]  html data length
 *
 * @return detected encoding if encoding found, otherwise MyENCODING_NOT_DETERMINED
 */
myencoding_t
myencoding_prescan_stream_to_determine_encoding(const char *data, size_t data_size);

/**
 * Extracting character encoding from string. Find "charset=" and see encoding. 
 * For example: "text/html; charset=windows-1251". Return MyENCODING_WINDOWS_1251
 *
 *
 * See https://html.spec.whatwg.org/multipage/infrastructure.html#algorithm-for-extracting-a-character-encoding-from-a-meta-element
 *
 * @param[in]  data
 * @param[in]  data length
 * @param[out] return encoding
 *
 * @return true if encoding found
 */
bool
myencoding_extracting_character_encoding_from_charset(const char *data, size_t data_size,
                                                           myencoding_t *encoding);

/**
 * Detect encoding in meta tag (<meta ...>) before start parsing and return found raw data
 *
 * See https://html.spec.whatwg.org/multipage/syntax.html#prescan-a-byte-stream-to-determine-its-encoding
 *
 * @param[in]  html data bytes
 * @param[in]  html data length
 * @param[out] return raw char data point for find encoding
 * @param[out] return raw char length
 *
 * @return detected encoding if encoding found, otherwise MyENCODING_NOT_DETERMINED
 */
myencoding_t
myencoding_prescan_stream_to_determine_encoding_with_found(const char *data, size_t data_size,
                                                           const char **found, size_t *found_length);

/**
 * Extracting character encoding from string. Find "charset=" and see encoding. Return found raw data.
 * For example: "text/html; charset=windows-1251". Return MyENCODING_WINDOWS_1251
 *
 *
 * See https://html.spec.whatwg.org/multipage/infrastructure.html#algorithm-for-extracting-a-character-encoding-from-a-meta-element
 *
 * @param[in]  data
 * @param[in]  data length
 * @param[out] return encoding
 * @param[out] return raw char data point for find encoding
 * @param[out] return raw char length
 *
 * @return true if encoding found
 */
bool
myencoding_extracting_character_encoding_from_charset_with_found(const char *data, size_t data_size,
                                                                 myencoding_t *encoding,
                                                                 const char **found, size_t *found_length);

/***********************************************************************************
 *
 * MyHTML_STRING
 *
 ***********************************************************************************/

/**
 * Init mycore_string_t structure
 *
 * @param[in] mchar_async_t*. It can be obtained from myhtml_tree_t object
 *  (see myhtml_tree_get_mchar function) or create manualy
 *  For each Tree creates its object, I recommend to use it (myhtml_tree_get_mchar).
 *
 * @param[in] node_id. For all threads (and Main thread) identifier that is unique.
 *  if created mchar_async_t object manually you know it, if not then take from the Tree 
 *  (see myhtml_tree_get_mchar_node_id)
 *
 * @param[in] mycore_string_t*. It can be obtained from myhtml_tree_node_t object
 *  (see myhtml_node_string function) or create manualy
 *
 * @param[in] data size. Set the size you want for char*
 *
 * @return char* of the size if successful, otherwise a NULL value
 */
char*
mycore_string_init(mchar_async_t *mchar, size_t node_id,
                   mycore_string_t* str, size_t size);

/**
 * Increase the current size for mycore_string_t object
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 * @param[in] data size. Set the new size you want for mycore_string_t object
 *
 * @return char* of the size if successful, otherwise a NULL value
 */
char*
mycore_string_realloc(mycore_string_t *str, size_t new_size);

/**
 * Clean mycore_string_t object. In reality, data length set to 0
 * Equivalently: mycore_string_length_set(str, 0);
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 */
void
mycore_string_clean(mycore_string_t* str);

/**
 * Clean mycore_string_t object. Equivalently: memset(str, 0, sizeof(mycore_string_t))
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 */
void
mycore_string_clean_all(mycore_string_t* str);

/**
 * Release all resources for mycore_string_t object
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 * @param[in] call free function for current object or not
 *
 * @return NULL if destroy_obj set true, otherwise a current mycore_string_t object
 */
mycore_string_t*
mycore_string_destroy(mycore_string_t* str, bool destroy_obj);

/**
 * Get data (char*) from a mycore_string_t object
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 *
 * @return char* if exists, otherwise a NULL value
 */
char*
mycore_string_data(mycore_string_t *str);

/**
 * Get data length from a mycore_string_t object
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 *
 * @return data length
 */
size_t
mycore_string_length(mycore_string_t *str);

/**
 * Get data size from a mycore_string_t object
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 *
 * @return data size
 */
size_t
mycore_string_size(mycore_string_t *str);

/**
 * Set data (char *) for a mycore_string_t object.
 *
 * Attention!!! Attention!!! Attention!!!
 *
 * You can assign only that it has been allocated from functions:
 * mycore_string_data_alloc
 * mycore_string_data_realloc
 * or obtained manually created from mchar_async_t object
 *
 * Attention!!! Do not try set chat* from allocated by malloc or realloc!!!
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 * @param[in] you data to want assign
 *
 * @return assigned data if successful, otherwise a NULL value
 */
char*
mycore_string_data_set(mycore_string_t *str, char *data);

/**
 * Set data size for a mycore_string_t object.
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 * @param[in] you size to want assign
 *
 * @return assigned size
 */
size_t
mycore_string_size_set(mycore_string_t *str, size_t size);

/**
 * Set data length for a mycore_string_t object.
 *
 * @param[in] mycore_string_t*. See description for mycore_string_init function
 * @param[in] you length to want assign
 *
 * @return assigned length
 */
size_t
mycore_string_length_set(mycore_string_t *str, size_t length);

/**
 * Allocate data (char*) from a mchar_async_t object
 *
 * @param[in] mchar_async_t*. See description for mycore_string_init function
 * @param[in] node id. See description for mycore_string_init function
 * @param[in] you size to want assign
 *
 * @return data if successful, otherwise a NULL value
 */
char*
mycore_string_data_alloc(mchar_async_t *mchar, size_t node_id, size_t size);

/**
 * Allocate data (char*) from a mchar_async_t object
 *
 * @param[in] mchar_async_t*. See description for mycore_string_init function
 * @param[in] node id. See description for mycore_string_init function
 * @param[in] old data
 * @param[in] how much data is copied from the old data to new data
 * @param[in] new size
 *
 * @return data if successful, otherwise a NULL value
 */
char*
mycore_string_data_realloc(mchar_async_t *mchar, size_t node_id,
                           char *data,  size_t len_to_copy, size_t size);

/**
 * Release allocated data
 *
 * @param[in] mchar_async_t*. See description for mycore_string_init function
 * @param[in] node id. See description for mycore_string_init function
 * @param[in] data to release
 *
 * @return data if successful, otherwise a NULL value
 */
void
mycore_string_data_free(mchar_async_t *mchar, size_t node_id, char *data);

/***********************************************************************************
 *
 * MyHTML_STRING_RAW
 *
 * All work with mycore_string_raw_t object occurs through 
 *    mycore_malloc (standart malloc), mycore_realloc (standart realloc),
 *    mycore_free (standart free).
 * 
 * You are free to change them on without fear that something will happen
 * You can call free for str_raw.data, or change str_raw.length = 0
 *
 ***********************************************************************************/

/**
 * Clean mycore_string_raw_t object. In reality, data length set to 0
 *
 * @param[in] mycore_string_raw_t*
 */
void
mycore_string_raw_clean(mycore_string_raw_t* str_raw);

/**
 * Full clean mycore_string_raw_t object.
 * Equivalently: memset(str_raw, 0, sizeof(mycore_string_raw_t))
 *
 * @param[in] mycore_string_raw_t*
 */
void
mycore_string_raw_clean_all(mycore_string_raw_t* str_raw);

/**
 * Free resources for mycore_string_raw_t object
 *
 * @param[in] mycore_string_raw_t*
 * @param[in] call free function for current object or not
 *
 * @return NULL if destroy_obj set true, otherwise a current mycore_string_raw_t object
 */
mycore_string_raw_t*
mycore_string_raw_destroy(mycore_string_raw_t* str_raw, bool destroy_obj);


/***********************************************************************************
 *
 * MyHTML_INCOMING
 *
 * @description
 * For example, three buffer:
 *   1) Data: "bebebe";        Prev: 0; Next: 2; Size: 6;  Length: 6;  Offset: 0
 *   2) Data: "lalala-lululu"; Prev: 1; Next: 3; Size: 13; Length: 13; Offset: 6
 *   3) Data: "huy";           Prev: 2; Next: 0; Size: 3;  Length: 1;  Offset: 19
 *
 ***********************************************************************************/

/**
 * Get Incoming Buffer by position
 *
 * @param[in] current mycore_incoming_buffer_t*
 * @param[in] begin position
 *
 * @return mycore_incoming_buffer_t if successful, otherwise a NULL value
 */
mycore_incoming_buffer_t*
mycore_incoming_buffer_find_by_position(mycore_incoming_buffer_t *inc_buf, size_t begin);

/**
 * Get data of Incoming Buffer
 *
 * @param[in] mycore_incoming_buffer_t*
 *
 * @return const char* if successful, otherwise a NULL value
 */
const char*
mycore_incoming_buffer_data(mycore_incoming_buffer_t *inc_buf);

/**
 * Get data length of Incoming Buffer
 *
 * @param[in] mycore_incoming_buffer_t*
 *
 * @return size_t
 */
size_t
mycore_incoming_buffer_length(mycore_incoming_buffer_t *inc_buf);

/**
 * Get data size of Incoming Buffer
 *
 * @param[in] mycore_incoming_buffer_t*
 *
 * @return size_t
 */
size_t
mycore_incoming_buffer_size(mycore_incoming_buffer_t *inc_buf);

/**
 * Get data offset of Incoming Buffer. Global position of begin Incoming Buffer.
 * See description for MyHTML_INCOMING title
 *
 * @param[in] mycore_incoming_buffer_t*
 *
 * @return size_t
 */
size_t
mycore_incoming_buffer_offset(mycore_incoming_buffer_t *inc_buf);

/**
 * Get Relative Position for Incoming Buffer.
 * Incoming Buffer should be prepared by mycore_incoming_buffer_find_by_position
 *
 * @param[in] mycore_incoming_buffer_t*
 * @param[in] global begin
 *
 * @return size_t
 */
size_t
mycore_incoming_buffer_relative_begin(mycore_incoming_buffer_t *inc_buf, size_t begin);

/**
 * This function returns number of available data by Incoming Buffer
 * Incoming buffer may be incomplete. See mycore_incoming_buffer_next
 *
 * @param[in] mycore_incoming_buffer_t*
 * @param[in] global begin
 *
 * @return size_t
 */
size_t
mycore_incoming_buffer_available_length(mycore_incoming_buffer_t *inc_buf,
                                        size_t relative_begin, size_t length);

/**
 * Get next buffer
 *
 * @param[in] mycore_incoming_buffer_t*
 *
 * @return mycore_incoming_buffer_t*
 */
mycore_incoming_buffer_t*
mycore_incoming_buffer_next(mycore_incoming_buffer_t *inc_buf);

/**
 * Get prev buffer
 *
 * @param[in] mycore_incoming_buffer_t*
 *
 * @return mycore_incoming_buffer_t*
 */
mycore_incoming_buffer_t*
mycore_incoming_buffer_prev(mycore_incoming_buffer_t *inc_buf);

/***********************************************************************************
 *
 * MyHTML_NAMESPACE
 *
 ***********************************************************************************/

/**
 * Get namespace text by namespace type (id)
 *
 * @param[in] myhtml_namespace_t
 * @param[out] optional, length of returned text
 *
 * @return text if successful, otherwise a NULL value
 */
const char*
myhtml_namespace_name_by_id(myhtml_namespace_t ns, size_t *length);

/**
 * Get namespace type (id) by namespace text
 *
 * @param[in] const char*, namespace text
 * @param[in] size of namespace text
 * @param[out] detected namespace type (id)
 *
 * @return true if detect, otherwise false
 */
bool
myhtml_namespace_id_by_name(const char *name, size_t length, myhtml_namespace_t *ns);

/***********************************************************************************
 *
 * MyHTML_CALLBACK
 *
 ***********************************************************************************/

/**
 * Get current callback for tokens before processing
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_callback_token_f
 */
myhtml_callback_token_f
myhtml_callback_before_token_done(myhtml_tree_t* tree);

/**
 * Get current callback for tokens after processing
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_callback_token_f
 */
myhtml_callback_token_f
myhtml_callback_after_token_done(myhtml_tree_t* tree);

/**
 * Set callback for tokens before processing
 *
 * Warning!
 * If you using thread mode parsing then this callback calls from thread (not Main thread)
 * If you build MyHTML without thread or using MyHTML_OPTIONS_PARSE_MODE_SINGLE for create myhtml_t object
 *  then this callback calls from Main thread
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_callback_token_f callback function
 */
void
myhtml_callback_before_token_done_set(myhtml_tree_t* tree, myhtml_callback_token_f func, void* ctx);

/**
 * Set callback for tokens after processing
 *
 * Warning!
 * If you using thread mode parsing then this callback calls from thread (not Main thread)
 * If you build MyHTML without thread or using MyHTML_OPTIONS_PARSE_MODE_SINGLE for create myhtml_t object
 *  then this callback calls from Main thread
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_callback_token_f callback function
 */
void
myhtml_callback_after_token_done_set(myhtml_tree_t* tree, myhtml_callback_token_f func, void* ctx);

/**
 * Get current callback for tree node after inserted
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_callback_tree_node_f
 */
myhtml_callback_tree_node_f
myhtml_callback_tree_node_insert(myhtml_tree_t* tree);

/**
 * Get current callback for tree node after removed
 *
 * @param[in] myhtml_tree_t*
 *
 * @return myhtml_callback_tree_node_f
 */
myhtml_callback_tree_node_f
myhtml_callback_tree_node_remove(myhtml_tree_t* tree);

/**
 * Set callback for tree node after inserted
 *
 * Warning!
 * If you using thread mode parsing then this callback calls from thread (not Main thread)
 * If you build MyHTML without thread or using MyHTML_OPTIONS_PARSE_MODE_SINGLE for create myhtml_t object
 *  then this callback calls from Main thread
 *
 * Warning!!!
 * If you well access to attributes or text for node and you using thread mode then 
 * you need wait for token processing done. See myhtml_token_node_wait_for_done
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_callback_tree_node_f callback function
 */
void
myhtml_callback_tree_node_insert_set(myhtml_tree_t* tree, myhtml_callback_tree_node_f func, void* ctx);

/**
 * Set callback for tree node after removed
 *
 * Warning!
 * If you using thread mode parsing then this callback calls from thread (not Main thread)
 * If you build MyHTML without thread or using MyHTML_OPTIONS_PARSE_MODE_SINGLE for create myhtml_t object
 *  then this callback calls from Main thread
 *
 * Warning!!!
 * If you well access to attributes or text for node and you using thread mode then
 * you need wait for token processing done. See myhtml_token_node_wait_for_done
 *
 * @param[in] myhtml_tree_t*
 * @param[in] myhtml_callback_tree_node_f callback function
 */
void
myhtml_callback_tree_node_remove_set(myhtml_tree_t* tree, myhtml_callback_tree_node_f func, void* ctx);

/***********************************************************************************
 *
 * MyHTML_UTILS
 *
 ***********************************************************************************/

/**
 * Compare two strings ignoring case
 *
 * @param[in] myhtml_collection_t*
 * @param[in] count of add nodes
 *
 * @return 0 if match, otherwise index of break position
 */
size_t
mycore_strcasecmp(const char* str1, const char* str2);

/**
 * Compare two strings ignoring case of the first n characters
 *
 * @param[in] myhtml_collection_t*
 * @param[in] count of add nodes
 *
 * @return 0 if match, otherwise index of break position
 */
size_t
mycore_strncasecmp(const char* str1, const char* str2, size_t size);

/***********************************************************************************
 *
 * MyHTML_SERIALIZATION
 *
 ***********************************************************************************/

/**
 * Tree fragment serialization 
 * The same as myhtml_serialization_tree_buffer function
 */
mystatus_t
myhtml_serialization(myhtml_tree_node_t* scope_node, mycore_string_raw_t* str);

/**
 * Only one tree node serialization
 * The same as myhtml_serialization_node_buffer function
 */
mystatus_t
myhtml_serialization_node(myhtml_tree_node_t* node, mycore_string_raw_t* str);

/**
 * Serialize tree to an output string
 *
 * @param[in] myhtml_tree_t*
 * @param[in] scope node
 * @param[in] mycore_string_raw_t*
 *
 * @return true if successful, otherwise false
 */
mystatus_t
myhtml_serialization_tree_buffer(myhtml_tree_node_t* scope_node, mycore_string_raw_t* str);

/**
 * Serialize node to an output string
 *
 * @param[in] myhtml_tree_t*
 * @param[in] node
 * @param[in] mycore_string_raw_t*
 *
 * @return true if successful, otherwise false
 */
mystatus_t
myhtml_serialization_node_buffer(myhtml_tree_node_t* node, mycore_string_raw_t* str);

/**
 * The serialize function for an entire tree
 *
 * @param[in] tree        the tree to be serialized
 * @param[in] scope_node  the scope_node
 * @param[in] callback    function that will be called for all strings that have to be printed
 * @param[in] ptr         user-supplied pointer
 *
 * @return true if successful, otherwise false
 */
mystatus_t
myhtml_serialization_tree_callback(myhtml_tree_node_t* scope_node,
                                   mycore_callback_serialize_f callback, void* ptr);

/**
 * The serialize function for a single node
 *
 * @param[in] tree        the tree to be serialized
 * @param[in] node        the node that is going to be serialized
 * @param[in] callback    function that will be called for all strings that have to be printed
 * @param[in] ptr         user-supplied pointer
 *
 * @return true if successful, otherwise false
 */
mystatus_t
myhtml_serialization_node_callback(myhtml_tree_node_t* node,
                                   mycore_callback_serialize_f callback, void* ptr);

/***********************************************************************************
 *
 * MyHTML_VERSION
 *
 ***********************************************************************************/

/**
 * Get current version
 *
 * @return myhtml_version_t
 */
myhtml_version_t
myhtml_version(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_api.h */

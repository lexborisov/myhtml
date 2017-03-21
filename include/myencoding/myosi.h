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

#ifndef MyENCODING_MYOSI_H
#define MyENCODING_MYOSI_H
#pragma once

#include <mycore/myosi.h>

#define MyENCODING_VERSION_MAJOR 0
#define MyENCODING_VERSION_MINOR 1
#define MyENCODING_VERSION_PATCH 0

#define MyENCODING_VERSION_STRING MyCORE_STR(MyENCODING_VERSION_MAJOR) MyCORE_STR(.) MyCORE_STR(MyENCODING_VERSION_MINOR) MyCORE_STR(.) MyCORE_STR(MyENCODING_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif

// encoding
// https://encoding.spec.whatwg.org/#the-encoding
// https://encoding.spec.whatwg.org/#legacy-single-byte-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-chinese-(simplified)-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-chinese-(traditional)-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-japanese-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-korean-encodings
// https://encoding.spec.whatwg.org/#legacy-miscellaneous-encodings

enum myencoding_list {
    MyENCODING_DEFAULT        = 0x00,
    //MyENCODING_AUTO           = 0x01,  // future
    MyENCODING_NOT_DETERMINED = 0x02,
    MyENCODING_UTF_8          = 0x00,  // default encoding
    MyENCODING_UTF_16LE       = 0x04,
    MyENCODING_UTF_16BE       = 0x05,
    MyENCODING_X_USER_DEFINED = 0x06,
    MyENCODING_BIG5           = 0x07,
    MyENCODING_EUC_JP         = 0x08,
    MyENCODING_EUC_KR         = 0x09,
    MyENCODING_GB18030        = 0x0a,
    MyENCODING_GBK            = 0x0b,
    MyENCODING_IBM866         = 0x0c,
    MyENCODING_ISO_2022_JP    = 0x0d,
    MyENCODING_ISO_8859_10    = 0x0e,
    MyENCODING_ISO_8859_13    = 0x0f,
    MyENCODING_ISO_8859_14    = 0x10,
    MyENCODING_ISO_8859_15    = 0x11,
    MyENCODING_ISO_8859_16    = 0x12,
    MyENCODING_ISO_8859_2     = 0x13,
    MyENCODING_ISO_8859_3     = 0x14,
    MyENCODING_ISO_8859_4     = 0x15,
    MyENCODING_ISO_8859_5     = 0x16,
    MyENCODING_ISO_8859_6     = 0x17,
    MyENCODING_ISO_8859_7     = 0x18,
    MyENCODING_ISO_8859_8     = 0x19,
    MyENCODING_ISO_8859_8_I   = 0x1a,
    MyENCODING_KOI8_R         = 0x1b,
    MyENCODING_KOI8_U         = 0x1c,
    MyENCODING_MACINTOSH      = 0x1d,
    MyENCODING_SHIFT_JIS      = 0x1e,
    MyENCODING_WINDOWS_1250   = 0x1f,
    MyENCODING_WINDOWS_1251   = 0x20,
    MyENCODING_WINDOWS_1252   = 0x21,
    MyENCODING_WINDOWS_1253   = 0x22,
    MyENCODING_WINDOWS_1254   = 0x23,
    MyENCODING_WINDOWS_1255   = 0x24,
    MyENCODING_WINDOWS_1256   = 0x25,
    MyENCODING_WINDOWS_1257   = 0x26,
    MyENCODING_WINDOWS_1258   = 0x27,
    MyENCODING_WINDOWS_874    = 0x28,
    MyENCODING_X_MAC_CYRILLIC = 0x29,
    MyENCODING_LAST_ENTRY     = 0x2a
}
typedef myencoding_t;

enum myencoding_status {
    MyENCODING_STATUS_OK       = 0x0000,
    MyENCODING_STATUS_ERROR    = 0x0001,
    MyENCODING_STATUS_CONTINUE = 0x0002,
    MyENCODING_STATUS_DONE     = 0x0004
}
typedef myencoding_status_t;

typedef struct myencoding_result myencoding_result_t;
typedef struct myencoding_trigram myencoding_trigram_t;
typedef struct myencoding_trigram_result myencoding_trigram_result_t;
typedef struct myencoding_unicode_result myencoding_unicode_result_t;
typedef struct myencoding_detect_name_entry myencoding_detect_name_entry_t;
typedef struct myencoding_detect_attr myencoding_detect_attr_t;
typedef struct myencoding_entry_name_index myencoding_entry_name_index_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyENCODING_MYOSI_H */


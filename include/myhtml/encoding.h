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

#ifndef MyHTML_ENCODING_H
#define MyHTML_ENCODING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"

enum myhtml_encoding_status {
    MyHTML_ENCODING_STATUS_OK       = 0x00,
    MyHTML_ENCODING_STATUS_ERROR    = 0x01,
    MyHTML_ENCODING_STATUS_CONTINUE = 0x02,
    MyHTML_ENCODING_STATUS_DONE     = 0x04
};

// https://encoding.spec.whatwg.org/#the-encoding
// https://encoding.spec.whatwg.org/#legacy-single-byte-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-chinese-(simplified)-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-chinese-(traditional)-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-japanese-encodings
// https://encoding.spec.whatwg.org/#legacy-multi-byte-korean-encodings
// https://encoding.spec.whatwg.org/#legacy-miscellaneous-encodings

enum myhtml_encoding_list {
    MyHTML_ENCODING_UNDEF          = 0x00,
    MyHTML_ENCODING_UTF_8          = 0x01,
    MyHTML_ENCODING_BIG5           = 0x02,
    MyHTML_ENCODING_EUC_KR         = 0x03,
    MyHTML_ENCODING_GB18030        = 0x04,
    MyHTML_ENCODING_IBM866         = 0x05,
    MyHTML_ENCODING_ISO_8859_10    = 0x06,
    MyHTML_ENCODING_ISO_8859_13    = 0x07,
    MyHTML_ENCODING_ISO_8859_14    = 0x08,
    MyHTML_ENCODING_ISO_8859_15    = 0x09,
    MyHTML_ENCODING_ISO_8859_16    = 0x0a,
    MyHTML_ENCODING_ISO_8859_2     = 0x0b,
    MyHTML_ENCODING_ISO_8859_3     = 0x0c,
    MyHTML_ENCODING_ISO_8859_4     = 0x0d,
    MyHTML_ENCODING_ISO_8859_5     = 0x0e,
    MyHTML_ENCODING_ISO_8859_6     = 0x0f,
    MyHTML_ENCODING_ISO_8859_7     = 0x10,
    MyHTML_ENCODING_ISO_8859_8     = 0x11,
    MyHTML_ENCODING_JIS0208        = 0x12,
    MyHTML_ENCODING_JIS0212        = 0x13,
    MyHTML_ENCODING_KOI8_R         = 0x14,
    MyHTML_ENCODING_KOI8_U         = 0x15,
    MyHTML_ENCODING_MACINTOSH      = 0x16,
    MyHTML_ENCODING_WINDOWS_1250   = 0x17,
    MyHTML_ENCODING_WINDOWS_1251   = 0x18,
    MyHTML_ENCODING_WINDOWS_1252   = 0x19,
    MyHTML_ENCODING_WINDOWS_1253   = 0x1a,
    MyHTML_ENCODING_WINDOWS_1254   = 0x1b,
    MyHTML_ENCODING_WINDOWS_1255   = 0x1c,
    MyHTML_ENCODING_WINDOWS_1256   = 0x1d,
    MyHTML_ENCODING_WINDOWS_1257   = 0x1e,
    MyHTML_ENCODING_WINDOWS_1258   = 0x1f,
    MyHTML_ENCODING_WINDOWS_874    = 0x20,
    MyHTML_ENCODING_X_MAC_CYRILLIC = 0x21,
    MyHTML_ENCODING_LAST_ENTRY     = 0x22
};

struct myhtml_encoding_result {
    unsigned long first;
    unsigned long second;
    unsigned long third;
    unsigned long result;
    unsigned long result_aux;
    unsigned long flag;
}
typedef myhtml_encoding_result_t;

enum myhtml_encoding_status myhtml_encoding_decode_utf_8(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_ibm866(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_2(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_3(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_4(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_5(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_6(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_7(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_8(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_8_i(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_10(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_13(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_14(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_15(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_8859_16(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_koi8_r(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_koi8_u(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_macintosh(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_874(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_1250(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_1251(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_1252(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_1253(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_1254(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_1255(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_1256(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_1257(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_windows_1258(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_x_mac_cyrillic(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_gbk(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_gb18030(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_big5(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_euc_jp(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_iso_2022_jp(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_shift_jis(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_euc_kr(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_shared_utf_16(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_utf_16be(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_utf_16le(unsigned const char data, myhtml_encoding_result_t *res);
enum myhtml_encoding_status myhtml_encoding_decode_x_user_defined(unsigned const char data, myhtml_encoding_result_t *res);

size_t myhtml_encoding_dec_to_char(unsigned long value, unsigned char *bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* encoding_h */

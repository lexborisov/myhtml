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

#ifndef MyHTML_ENCODING_H
#define MyHTML_ENCODING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/utils.h"

enum myhtml_encoding_status {
    MyHTML_ENCODING_STATUS_OK       = 0x00,
    MyHTML_ENCODING_STATUS_ERROR    = 0x01,
    MyHTML_ENCODING_STATUS_CONTINUE = 0x02,
    MyHTML_ENCODING_STATUS_DONE     = 0x04
}
typedef myhtml_encoding_status_t;

struct myhtml_encoding_result {
    unsigned long first;
    unsigned long second;
    unsigned long third;
    unsigned long result;
    unsigned long result_aux;
    unsigned long flag;
}
typedef myhtml_encoding_result_t;

struct myhtml_encoding_trigram {
    const unsigned char trigram[3];
    size_t value;
}
typedef myhtml_encoding_trigram_t;

struct myhtml_encoding_trigram_result {
    size_t count;
    size_t value;
}
typedef myhtml_encoding_trigram_result_t;

struct myhtml_encoding_unicode_result {
    size_t count_ascii;
    size_t count_good;
    size_t count_bad;
}
typedef myhtml_encoding_unicode_result_t;

struct myhtml_encoding_detect_name_entry {
    const char* name;
    size_t name_length;
    const char* label;
    size_t label_length;
    
    myhtml_encoding_t encoding;
    
    size_t next;
    size_t curr;
}
typedef myhtml_encoding_detect_name_entry_t;

typedef myhtml_encoding_status_t (*myhtml_encoding_custom_f)(unsigned const char data, myhtml_encoding_result_t *res);

myhtml_encoding_custom_f myhtml_encoding_get_function_by_id(myhtml_encoding_t idx);

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

size_t myhtml_encoding_codepoint_to_ascii_utf_8(size_t codepoint, char *data);
size_t myhtml_encoding_codepoint_to_lowercase_ascii_utf_8(size_t codepoint, char *data);
size_t myhtml_encoding_codepoint_to_ascii_utf_16(size_t codepoint, char *data);

void myhtml_encoding_result_clean(myhtml_encoding_result_t *res);

bool myhtml_encoding_detect(const char *text, size_t length, myhtml_encoding_t *encoding);
bool myhtml_encoding_detect_russian(const char *text, size_t length, myhtml_encoding_t *encoding);
bool myhtml_encoding_detect_unicode(const char *text, size_t length, myhtml_encoding_t *encoding);
bool myhtml_encoding_detect_bom(const char *text, size_t length, myhtml_encoding_t *encoding);
bool myhtml_encoding_detect_and_cut_bom(const char *text, size_t length, myhtml_encoding_t *encoding, const char **new_text, size_t *new_size);

const myhtml_encoding_detect_name_entry_t * myhtml_encoding_name_entry_by_name(const char* name, size_t length);
bool myhtml_encoding_by_name(const char *name, size_t length, myhtml_encoding_t *encoding);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* encoding_h */

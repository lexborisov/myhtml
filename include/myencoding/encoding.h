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

#ifndef MyENCODING_ENCODING_H
#define MyENCODING_ENCODING_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include <myencoding/myosi.h>
#include <mycore/utils.h>
#include <mycore/mystring.h>

struct myencoding_result {
    unsigned long first;
    unsigned long second;
    unsigned long third;
    unsigned long result;
    unsigned long result_aux;
    unsigned long flag;
};

struct myencoding_trigram {
    const unsigned char trigram[3];
    size_t value;
};

struct myencoding_trigram_result {
    size_t count;
    size_t value;
};

struct myencoding_unicode_result {
    size_t count_ascii;
    size_t count_good;
    size_t count_bad;
};

struct myencoding_detect_name_entry {
    const char* name;
    size_t name_length;
    const char* label;
    size_t label_length;
    
    myencoding_t encoding;
    
    size_t next;
    size_t curr;
};

struct myencoding_detect_attr {
    size_t key_begin;
    size_t key_length;
    size_t value_begin;
    size_t value_length;
    
    myencoding_detect_attr_t *next;
};
    
struct myencoding_entry_name_index {
    const char *name;
    size_t length;
};

typedef myencoding_status_t (*myencoding_custom_f)(unsigned const char data, myencoding_result_t *res);

myencoding_custom_f myencoding_get_function_by_id(myencoding_t idx);

myencoding_status_t myencoding_decode_utf_8(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_ibm866(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_2(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_3(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_4(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_5(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_6(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_7(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_8(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_8_i(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_10(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_13(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_14(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_15(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_8859_16(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_koi8_r(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_koi8_u(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_macintosh(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_874(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_1250(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_1251(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_1252(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_1253(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_1254(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_1255(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_1256(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_1257(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_windows_1258(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_x_mac_cyrillic(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_gbk(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_gb18030(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_big5(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_euc_jp(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_iso_2022_jp(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_shift_jis(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_euc_kr(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_shared_utf_16(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_utf_16be(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_utf_16le(unsigned const char data, myencoding_result_t *res);
myencoding_status_t myencoding_decode_x_user_defined(unsigned const char data, myencoding_result_t *res);

size_t myencoding_codepoint_ascii_length(size_t codepoint);
size_t myencoding_ascii_utf_8_length(const unsigned char data);

size_t myencoding_codepoint_to_ascii_utf_8(size_t codepoint, char *data);
size_t myencoding_codepoint_to_lowercase_ascii_utf_8(size_t codepoint, char *data);
size_t myencoding_codepoint_to_ascii_utf_16(size_t codepoint, char *data);
size_t myencoding_ascii_utf_8_to_codepoint(const unsigned char* data, size_t* codepoint);

void myencoding_result_clean(myencoding_result_t *res);

bool myencoding_detect(const char *text, size_t length, myencoding_t *encoding);
bool myencoding_detect_russian(const char *text, size_t length, myencoding_t *encoding);
bool myencoding_detect_unicode(const char *text, size_t length, myencoding_t *encoding);
bool myencoding_detect_bom(const char *text, size_t length, myencoding_t *encoding);
bool myencoding_detect_and_cut_bom(const char *text, size_t length, myencoding_t *encoding, const char **new_text, size_t *new_size);

size_t myencoding_convert_to_ascii_utf_8(mycore_string_raw_t* raw_str, const char* buff, size_t length, myencoding_t encoding);

const myencoding_detect_name_entry_t * myencoding_name_entry_by_name(const char* name, size_t length);
bool myencoding_by_name(const char *name, size_t length, myencoding_t *encoding);
const char * myencoding_name_by_id(myencoding_t encoding, size_t *length);

bool myencoding_extracting_character_encoding_from_charset(const char *data, size_t data_size, myencoding_t *encoding);
myencoding_t myencoding_prescan_stream_to_determine_encoding(const char *data, size_t data_size);

bool myencoding_extracting_character_encoding_from_charset_with_found(const char *data, size_t data_size, myencoding_t *encoding, const char **found, size_t *found_length);
myencoding_t myencoding_prescan_stream_to_determine_encoding_with_found(const char *data, size_t data_size, const char **found, size_t *found_length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyENCODING_ENCODING_H */

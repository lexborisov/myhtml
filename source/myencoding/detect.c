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

#include "myencoding/encoding.h"
#include "myencoding/detect_resource.h"
#include "mycore/utils/resources.h"

myencoding_trigram_result_t myencoding_detect_by_trigram(unsigned const char *u_text, size_t length,
                                                                   const myencoding_trigram_t *list, size_t list_length,
                                                                   size_t max_sum_for_break)
{
    myencoding_trigram_result_t res = {0, 0};
    
    for (size_t i = 0; i < (length - 3); i++) {
        if(u_text[i] > 127)
        {
            for (size_t j = 0; j < list_length; j++)
            {
                if(memcmp(list[j].trigram, &u_text[i], 3) == 0) {
                    res.value += list[j].value;
                    res.count++;
                    
                    if(res.value >= max_sum_for_break)
                        i = length;
                    
                    break;
                }
            }
        }
    }
    
    return res;
}

bool myencoding_detect_russian_has_end(myencoding_trigram_result_t *res, size_t min_count, size_t min_value)
{
    if(res->value >= min_value || res->count >= min_count)
        return true;
    
    return false;
}

bool myencoding_detect_unicode_has_end(myencoding_unicode_result_t *res, size_t max_bad_percent)
{
    if(res->count_good == 0) {
        if(res->count_bad)
            return false;
        
        return true;
    }
    else if(res->count_bad == 0)
        return true;
    
    size_t percent_bad = (res->count_bad * 100) / res->count_good;
    if(percent_bad < max_bad_percent)
        return true;
    
    return false;
}

myencoding_unicode_result_t myencoding_detect_utf_8(unsigned const char *u_text, size_t length)
{
    size_t i = 0;
    myencoding_unicode_result_t res = {0, 0, 0};
    
    while(i < length)
    {
        if((u_text[i] & 0x80) == 0x00) {
            i++;
            res.count_ascii++;
        }
        else if((u_text[i] & 0xE0) == 0xC0) {
            i += 2;
            
            if(i >= length)
                break;
            
            if(((u_text[i - 1] & 0x80) && (u_text[i - 1] & 0x40) == 0))
                res.count_good++;
            else
                res.count_bad++;
        }
        else if((u_text[i] & 0xF0) == 0xE0) {
            i += 3;
            
            if(i >= length)
                break;
            
            if(((u_text[i - 1] & 0x80) && (u_text[i - 1] & 0x40) == 0) &&
               ((u_text[i - 2] & 0x80) && (u_text[i - 2] & 0x40) == 0)) {
                res.count_good++;
            }
            else
                res.count_bad++;
        }
        else if((u_text[i] & 0xF8) == 0xF0) {
            i += 4;
            
            if(i >= length)
                break;
            
            if(((u_text[i - 1] & 0x80) && (u_text[i - 1] & 0x40) == 0) &&
               ((u_text[i - 2] & 0x80) && (u_text[i - 2] & 0x40) == 0) &&
               ((u_text[i - 3] & 0x80) && (u_text[i - 3] & 0x40) == 0)) {
                res.count_good++;
            }
            else
                res.count_bad++;
        }
        else {
            i++;
            res.count_bad++;
        }
    }
    
    return res;
}

myencoding_unicode_result_t myencoding_detect_utf_16(unsigned const char *u_text, size_t length)
{
    size_t i = 0;
    myencoding_unicode_result_t res = {0, 0, 0};
    
    while(i < length)
    {
        if(u_text[i] == 0x00) {
            if((i % 2) == 0) {
                i++;
                
                if(u_text[i] > 0x1F && u_text[i] < 0x7F)
                    res.count_bad++;
            }
            else {
                if(u_text[(i - 1)] > 0x1F && u_text[(i - 1)] < 0x7F)
                    res.count_good++;
                
                i++;
            }
        }
        else
            i++;
    }
    
    return res;
}

bool myencoding_detect_bom(const char *text, size_t length, myencoding_t *encoding)
{
    unsigned const char *u_text = (unsigned const char*)text;
    
    if(length > 2) {
        if(u_text[0] == 0xEF &&
           u_text[1] == 0xBB &&
           u_text[2] == 0xBF)
        {
            *encoding = MyENCODING_UTF_8;
            return true;
        }
    }
    
    if(length > 1) {
        if(u_text[0] == 0xFE && u_text[1] == 0xFF) {
            *encoding = MyENCODING_UTF_16BE;
            return true;
        }
        
        if(u_text[0] == 0xFF && u_text[1] == 0xFE) {
            *encoding = MyENCODING_UTF_16LE;
            return true;
        }
    }
    
//  //for UTF-32
//    if(length > 3) {
//        if(u_text[0] == 0x00 &&
//           u_text[1] == 0x00 &&
//           u_text[2] == 0xFE &&
//           u_text[3] == 0xFF)
//        {
//            *encoding = MyENCODING_UTF_32BE;
//            return true;
//        }
//        
//        if(u_text[0] == 0xFF &&
//           u_text[1] == 0xFE &&
//           u_text[2] == 0x00 &&
//           u_text[3] == 0x00)
//        {
//            *encoding = MyENCODING_UTF_32LE;
//            return true;
//        }
//    }
    
    return false;
}

bool myencoding_detect_and_cut_bom(const char *text, size_t length, myencoding_t *encoding, const char **new_text, size_t *new_size)
{
    if(myencoding_detect_bom(text, length, encoding))
    {
        if(*encoding == MyENCODING_UTF_8) {
            *new_text = &text[3];
            *new_size = length - 3;
        }
        else {
            *new_text = &text[2];
            *new_size = length - 2;
        }
        
        return true;
    }
    
    return false;
}

bool myencoding_detect_unicode(const char *text, size_t length, myencoding_t *encoding)
{
    unsigned const char *u_text = (unsigned const char*)text;
    *encoding = MyENCODING_DEFAULT;
    
    myencoding_unicode_result_t res = myencoding_detect_utf_16(u_text, length);
    
    if(res.count_bad == 0 && res.count_good >= 3) {
        *encoding = MyENCODING_UTF_16LE;
        return true;
    }
    else if(res.count_bad >= 3 && res.count_good == 0) {
        *encoding = MyENCODING_UTF_16BE;
        return true;
    }
    
    res = myencoding_detect_utf_8(u_text, length);
    if(myencoding_detect_unicode_has_end(&res, 10)) {
        *encoding = MyENCODING_UTF_8;
        return true;
    }
    
    return false;
}

bool myencoding_detect_russian(const char *text, size_t length, myencoding_t *encoding)
{
    unsigned const char *u_text = (unsigned const char*)text;
    
    size_t min_count = 50;
    size_t min_value = 100000;
    size_t max_value = 0;
    
    *encoding = MyENCODING_DEFAULT;
    
    myencoding_trigram_result_t
    res = myencoding_detect_by_trigram(u_text, length, myencoding_detect_trigrams_index_windows_1251, 1000, min_value);
    if(myencoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyENCODING_WINDOWS_1251;
        return true;
    }
    
    max_value = res.value;
    if(max_value) {
        *encoding = MyENCODING_WINDOWS_1251;
    }
    
    res = myencoding_detect_by_trigram(u_text, length, myencoding_detect_trigrams_index_koi8_r, 1000, min_value);
    if(myencoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyENCODING_KOI8_R;
        return true;
    }
    
    if(max_value < res.value) {
        *encoding = MyENCODING_KOI8_R;
        max_value = res.value;
    }
    
    res = myencoding_detect_by_trigram(u_text, length, myencoding_detect_trigrams_index_iso_8859_5, 1000, min_value);
    if(myencoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyENCODING_ISO_8859_5;
        return true;
    }
    
    if(max_value < res.value) {
        *encoding = MyENCODING_ISO_8859_5;
        max_value = res.value;
    }
    
    res = myencoding_detect_by_trigram(u_text, length, myencoding_detect_trigrams_index_x_mac_cyrillic, 1000, min_value);
    if(myencoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyENCODING_X_MAC_CYRILLIC;
        return true;
    }
    
    if(max_value < res.value) {
        *encoding = MyENCODING_X_MAC_CYRILLIC;
        max_value = res.value;
    }
    
    res = myencoding_detect_by_trigram(u_text, length, myencoding_detect_trigrams_index_ibm866, 1000, min_value);
    if(myencoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyENCODING_IBM866;
        return true;
    }
    
    if(max_value < res.value) {
        *encoding = MyENCODING_IBM866;
    }
    
    return false;
}

bool myencoding_detect(const char *text, size_t length, myencoding_t *encoding)
{
    *encoding = MyENCODING_DEFAULT;
    
    if(myencoding_detect_unicode(text, length, encoding))
        return true;
    
    if(myencoding_detect_russian(text, length, encoding))
        return true;
    
    return false;
}

const myencoding_detect_name_entry_t * myencoding_name_entry_by_name(const char* name, size_t length)
{
    size_t idx = ((mycore_string_chars_lowercase_map[ (const unsigned char)name[0] ] *
                   mycore_string_chars_lowercase_map[ (const unsigned char)name[(length - 1)] ] *
                   length)
                  % MyENCODING_DETECT_NAME_STATIC_SIZE) + 1;
    
    while (myencoding_detect_name_entry_static_list_index[idx].label)
    {
        if(myencoding_detect_name_entry_static_list_index[idx].label_length == length) {
            if(mycore_strncasecmp(myencoding_detect_name_entry_static_list_index[idx].label, name, length) == 0)
                return &myencoding_detect_name_entry_static_list_index[idx];
            
            if(myencoding_detect_name_entry_static_list_index[idx].next)
                idx = myencoding_detect_name_entry_static_list_index[idx].next;
            else
                return NULL;
        }
        else if(myencoding_detect_name_entry_static_list_index[idx].label_length > length) {
            return NULL;
        }
        else {
            idx = myencoding_detect_name_entry_static_list_index[idx].next;
        }
    }
    
    return NULL;
}

bool myencoding_by_name(const char *name, size_t length, myencoding_t *encoding)
{
    const myencoding_detect_name_entry_t *entry = myencoding_name_entry_by_name(name, length);
    
    if(entry) {
        if(encoding)
            *encoding = entry->encoding;
        
        return true;
    }
    
    return false;
}

const char * myencoding_name_by_id(myencoding_t encoding, size_t *length)
{
    if(encoding >= MyENCODING_LAST_ENTRY) {
        if(length) {
            *length = 0;
        }
        
        return NULL;
    }
    
    const myencoding_entry_name_index_t *entry = &myencoding_entry_name_index_static_list_index[encoding];
    
    if(length) {
        *length = entry->length;
    }
    
    return entry->name;
}

/*
  When an algorithm requires a user agent to prescan a byte stream to determine its encoding, 
  given some defined end condition, then it must run the following steps.
  These steps either abort unsuccessfully or return a character encoding. 
  If at any point during these steps (including during instances of the get an attribute algorithm invoked by this one) 
  the user agent either runs out of bytes (meaning the position pointer created in the first step below goes beyond the end of the byte stream obtained so far) 
  or reaches its end condition, then abort the prescan a byte stream to determine its encoding algorithm unsuccessfully.
*/
bool myencoding_extracting_character_encoding_from_charset(const char *data, size_t data_size, myencoding_t *encoding)
{
    return myencoding_extracting_character_encoding_from_charset_with_found(data, data_size, encoding, NULL, NULL);
}

bool myencoding_extracting_character_encoding_from_charset_with_found(const char *data, size_t data_size, myencoding_t *encoding, const char **found, size_t *found_length)
{
    *encoding = MyENCODING_NOT_DETERMINED;
    
    if(found)
        *found = NULL;
    if(found_length)
        *found_length = 0;
    
    /* 1 */
    size_t length = 0;
    size_t charset_length = strlen("charset");
    
    bool is_get_pos = false;
    const unsigned char *udata = (const unsigned char *)data;
    
    /* 2 */
    while((length + charset_length) < data_size) {
        if(mycore_ustrcasecmp_without_checks_by_secondary((const unsigned char*)"charset", &udata[length]))
        {
            length += charset_length;
            
            /* 2 */
            while(length < data_size) {
                if(udata[length] != 0x09 && udata[length] != 0x0A && udata[length] != 0x0C &&
                   udata[length] != 0x0D && udata[length] != 0x20)
                {
                    break;
                }
                
                length++;
            }
            
            /* 4 */
            if(udata[length] == 0x3D) { /* EQUALS SIGN (=) */
                is_get_pos = true;
                
                length++;
                break;
            }
        }
        
        length++;
    }
    
    if(is_get_pos == false || length >= data_size)
        return false;
    
    /* 5 */
    while(length < data_size) {
        if(udata[length] != 0x09 && udata[length] != 0x0A && udata[length] != 0x0C &&
           udata[length] != 0x0D && udata[length] != 0x20)
        {
            break;
        }
        
        length++;
    }
    
    if(length >= data_size)
        return false;
    
    /* 6 */
    /* " */
    if(udata[length] == 0x22)
    {
        length++;
        size_t begin = length;
        
        while(length < data_size) {
            if(udata[length] == 0x22)
            {
                if(found)
                    *found = &data[begin];
                if(found_length)
                    *found_length = (length - begin);
                
                return myencoding_by_name(&data[begin], (length - begin), encoding);
            }
            
            length++;
        }
        
        return false;
    }
    
    /* ' */
    if(udata[length] == 0x27)
    {
        length++;
        size_t begin = length;
        
        while(length < data_size) {
            if(udata[length] == 0x27)
            {
                if(found)
                    *found = &data[begin];
                if(found_length)
                    *found_length = (length - begin);
                
                return myencoding_by_name(&data[begin], (length - begin), encoding);
            }
            
            length++;
        }
        
        return false;
    }
    
    /* other */
    while(length < data_size) {
        if(udata[length] != 0x09 && udata[length] != 0x0A && udata[length] != 0x0C &&
           udata[length] != 0x0D && udata[length] != 0x20)
        {
            size_t begin = length;
            
            while(length < data_size) {
                /* SEMICOLON character (;) */
                if(udata[length] == 0x3B)
                {
                    if(found)
                        *found = &data[begin];
                    if(found_length)
                        *found_length = (length - begin);
                    
                    return myencoding_by_name(&data[begin], (length - begin), encoding);
                }
                
                length++;
            }
            
            if(found)
                *found = &data[begin];
            if(found_length)
                *found_length = (length - begin);
            
            return myencoding_by_name(&data[begin], (length - begin), encoding);
        }
        
        length++;
    }
    
    return false;
}

bool myencoding_prescan_stream_to_determine_encoding_get_attr_spaces(const unsigned char *udata, size_t *data_length, size_t data_size, myencoding_detect_attr_t *attr)
{
    size_t length = *data_length;
    
    /* set position */
    attr->key_length = length - attr->key_begin;
    
    /* 6 */
    while(length < data_size) {
        if(udata[length] != 0x09 && udata[length] != 0x0A && udata[length] != 0x0C &&
           udata[length] != 0x0D && udata[length] != 0x20 && udata[length] != 0x2F)
        {
            break;
        }
        
        length++;
    }
    
    if(length >= data_size) {
        *data_length = length;
        return false;
    }
    
    /* 7 */
    if(udata[length] != 0x3D) {
        *data_length = length;
        return false;
    }
    
    /* 8 */
    *data_length = (length + 1);
    return true;
}

size_t myencoding_prescan_stream_to_determine_encoding_get_attr_value(const unsigned char *udata, size_t length, size_t data_size, myencoding_detect_attr_t *attr, bool *it_last)
{
    /* 9 */
    while(length < data_size) {
        if(udata[length] != 0x09 && udata[length] != 0x0A && udata[length] != 0x0C &&
           udata[length] != 0x0D && udata[length] != 0x20)
        {
            break;
        }
        
        length++;
    }
    
    if(length >= data_size) {
        *it_last = true;
        return length;
    }
    
    /* 10 */
    switch (udata[length]) {
        case 0x22: /* (ASCII ") */
            length++;
            attr->value_begin = length;
            
            while(length < data_size) {
                if(udata[length] == 0x22)
                {
                    attr->value_length = length - attr->value_begin;
                    return (length + 1);
                }
                
                length++;
            }
            
            break;
            
        case 0x27: /* (ASCII ') */
            length++;
            attr->value_begin = length;
            
            while(length < data_size) {
                if(udata[length] == 0x27)
                {
                    attr->value_length = length - attr->value_begin;
                    return (length + 1);
                }
                
                length++;
            }
            
            break;
            
        case 0x3E: /* (ASCII >) */
            *it_last = true;
            return (length + 1);
            
        default:
            attr->value_begin = length;
            
            while(length < data_size) {
                if(udata[length] == 0x09 || udata[length] == 0x0A || udata[length] == 0x0C ||
                   udata[length] == 0x0D || udata[length] == 0x20 || udata[length] == 0x3E)
                {
                    attr->value_length = length - attr->value_begin;
                    return (length + 1);
                }
                
                length++;
            }
            
            break;
    }
    
    attr->value_length = length - attr->value_begin;
    return length;
}

size_t myencoding_prescan_stream_to_determine_encoding_get_attr(const unsigned char *udata, size_t length, size_t data_size, myencoding_detect_attr_t *attr, bool *it_last)
{
    memset(attr, 0, sizeof(myencoding_detect_attr_t));
    
    /*
     If the byte at position is one of 0x09 (ASCII TAB), 0x0A (ASCII LF), 0x0C (ASCII FF), 0x0D (ASCII CR),
     0x20 (ASCII space), or 0x2F (ASCII /) then advance position to the next byte and redo this step.
     */
    /* 1 */
    while(length < data_size) {
        if(udata[length] != 0x09 && udata[length] != 0x0A && udata[length] != 0x0C &&
           udata[length] != 0x0D && udata[length] != 0x20 && udata[length] != 0x2F)
        {
            break;
        }
        
        length++;
    }
    
    if(length >= data_size) {
        *it_last = true;
        return length;
    }
    
    /* 2 */
    if(udata[length] == 0x3E) { /* (ASCII >) */
        *it_last = true;
        return (length + 1);
    }
    
    attr->key_begin = length;
    
    /* 3, 4 */
    while(length < data_size) {
        switch (udata[length]) {
            case 0x3D: /* (ASCII =) */
                if(attr->key_begin != (length - 1)) {
                    attr->key_length = length - attr->key_begin;
                    
                    length++;
                    return myencoding_prescan_stream_to_determine_encoding_get_attr_value(udata, length, data_size, attr, it_last);
                }
            
            /* fall through */
                
            case 0x09: /* (ASCII TAB)   */
            case 0x0A: /* (ASCII LF)    */
            case 0x0C: /* (ASCII FF)    */
            case 0x0D: /* (ASCII CR)    */
            case 0x20: /* (ASCII space) */
                length++;
                
                if(myencoding_prescan_stream_to_determine_encoding_get_attr_spaces(udata, &length, data_size, attr) == false) {
                    *it_last = true;
                    return length;
                }
                
                return myencoding_prescan_stream_to_determine_encoding_get_attr_value(udata, length, data_size, attr, it_last);
                
            case 0x2F: /* (ASCII /)     */
            case 0x3E: /* (ASCII >)     */
                *it_last = true;
                attr->key_length = length - attr->key_begin;
                
                return (length + 1);
                
            default:
                break;
        }
        
        length++;
    }
    
    if(myencoding_prescan_stream_to_determine_encoding_get_attr_spaces(udata, &length, data_size, attr) == false) {
        *it_last = true;
        return length;
    }
    
    return myencoding_prescan_stream_to_determine_encoding_get_attr_value(udata, length, data_size, attr, it_last);
}

bool myencoding_prescan_stream_to_determine_encoding_check_meta(const unsigned char *udata, size_t *length, size_t data_size, myencoding_t *encoding, const char **found, size_t *found_length)
{
    myencoding_detect_attr_t attr;
    
    bool got_pragma = false;
    bool it_last = false;
    
    unsigned int need_pragma = 0; /* 0 = NULL, 1 = false, 2 = true */
    
    /*
      http-equiv = 1
      content = 2
      charset = 4
     */
    /* If the attribute's name is already in attribute list, then return to the step labeled attributes. */
    size_t is_exists = 0;
    
    while(*length < data_size) {
        *length = myencoding_prescan_stream_to_determine_encoding_get_attr(udata, *length, data_size, &attr, &it_last);
        
        /* 9 */
        if(attr.key_length == strlen("http-equiv") &&
           mycore_ustrcasecmp_without_checks_by_secondary((const unsigned char*)"http-equiv", &udata[ attr.key_begin ]))
        {
            if((is_exists & 1) == 0) {
                is_exists |= 1;
                
                if(attr.value_length == strlen("content-type") &&
                   mycore_ustrcasecmp_without_checks_by_secondary((const unsigned char*)"content-type", &udata[ attr.value_begin ]))
                {
                    got_pragma = true;
                }
            }
        }
        else if(attr.key_length == strlen("content") &&
                mycore_ustrcasecmp_without_checks_by_secondary((const unsigned char*)"content", &udata[ attr.key_begin ]))
        {
            if((is_exists & 2) == 0) {
                is_exists |= 2;
                
                if(myencoding_extracting_character_encoding_from_charset_with_found((const char*)(&udata[ attr.value_begin ]), attr.value_length, encoding, found, found_length)) {
                    need_pragma = 2;
                }
            }
        }
        else if(attr.key_length == strlen("charset") &&
                mycore_ustrcasecmp_without_checks_by_secondary((const unsigned char*)"charset", &udata[ attr.key_begin ]))
        {
            if((is_exists & 4) == 0) {
                is_exists |= 4;
                
                if(found)
                    *found = (const char*)(&udata[ attr.value_begin ]);
                if(found_length)
                    *found_length = attr.value_length;
                
                myencoding_by_name((const char*)(&udata[ attr.value_begin ]), attr.value_length, encoding);
                need_pragma = 1;
            }
        }
        
        if(it_last)
            break;
    }
    
    /* 11, 12, 13 */
    if(need_pragma == 0 || (need_pragma == 2 && got_pragma == false)) {
        *encoding = MyENCODING_NOT_DETERMINED;
        return false;
    }
    
    /* 14 */
    if(*encoding == MyENCODING_UTF_16BE || *encoding == MyENCODING_UTF_16LE) {
        *encoding = MyENCODING_UTF_8;
    }
    
    /* 15 */
    if(*encoding == MyENCODING_X_USER_DEFINED) {
        *encoding = MyENCODING_WINDOWS_1252;
    }
    
    /* 16 */
    return true;
}

size_t myencoding_prescan_stream_to_determine_encoding_skip_name(const unsigned char *udata, size_t length, size_t data_size)
{
    while(length < data_size) {
        if(udata[length] != 0x09 && udata[length] != 0x0A && udata[length] != 0x0C &&
           udata[length] != 0x0D && udata[length] != 0x20)
        {
            break;
        }
        
        length++;
    }
    
    if(length >= data_size)
        return length;
    
    if(udata[length] == 0x3E) {
        return (length + 1);
    }
    
    myencoding_detect_attr_t attr;
    bool it_last = false;
    
    while(length < data_size) {
        length = myencoding_prescan_stream_to_determine_encoding_get_attr(udata, length, data_size, &attr, &it_last);
        
        if(it_last) {
            return length;
        }
    }
    
    return length;
}

size_t myencoding_prescan_stream_to_determine_encoding_skip_other(const unsigned char *udata, size_t length, size_t data_size)
{
    if(udata[length] == 0x2F) { /* / */
        length++;
        
        if(length >= data_size)
            return length;
        
        if(mycore_tokenizer_chars_map[ udata[length] ] == MyCORE_STRING_MAP_CHAR_A_Z_a_z) {
            return myencoding_prescan_stream_to_determine_encoding_skip_name(udata, length, data_size);
        }
        
        while(length < data_size) {
            if(udata[length] != 0x3E) {
                return (length + 1);
            }
            
            length++;
        }
        
        return length;
    }
    else if(udata[length] == 0x21) { /* ! */
        length++;
        
        if((length + 2) < data_size && udata[length] == 0x2D && udata[(length+1)] == 0x2D) {
            while(length < data_size) {
                if(udata[length] != 0x3E) {
                    if(udata[(length - 1)] == 0x2D && udata[(length - 2)] == 0x2D)
                        return (length + 1);
                }
                
                length++;
            }
            
            return length;
        }
        
        while(length < data_size) {
            if(udata[length] != 0x3E) {
                return (length + 1);
            }
            
            length++;
        }
        
        return length;
    }
    else if(udata[length] == 0x3F) { /* ? */
        length++;
        
        while(length < data_size) {
            if(udata[length] != 0x3E) {
                return (length + 1);
            }
            
            length++;
        }
        
        return length;
    }
    
    
    return myencoding_prescan_stream_to_determine_encoding_skip_name(udata, length, data_size);
}

myencoding_t myencoding_prescan_stream_to_determine_encoding(const char *data, size_t data_size)
{
    return myencoding_prescan_stream_to_determine_encoding_with_found(data, data_size, NULL, NULL);
}

myencoding_t myencoding_prescan_stream_to_determine_encoding_with_found(const char *data, size_t data_size, const char **found, size_t *found_length)
{
    const unsigned char* udata = (const unsigned char*)data;
    myencoding_t encoding = MyENCODING_NOT_DETERMINED;
    
    if(found)
        *found = NULL;
    if(found_length)
        *found_length = 0;
    
    size_t i = 0;
    while(i < data_size) {
        /* 0x3C = '<' */
        if(data[i] == 0x3C)
        {
            if((i + 5) >= data_size)
                return encoding;
            
            i++;
            
            switch (data[i]) {
                    /*
                     A sequence of bytes starting with:
                     0x3C, 0x4D or 0x6D, 0x45 or 0x65, 0x54 or 0x74, 0x41 or 0x61,
                     and one of 0x09, 0x0A, 0x0C, 0x0D, 0x20, 0x2F
                     (case-insensitive ASCII '<meta' followed by a space or slash)
                     */
                case 0x4D:
                case 0x6D:
                    if(mycore_ustrcasecmp_without_checks_by_secondary((const unsigned char*)"meta", &udata[i])) {
                        i += 4;
                        
                        if(udata[i] == 0x09 || udata[i] == 0x0A || udata[i] == 0x0C ||
                           udata[i] == 0x0D || udata[i] == 0x20 || udata[i] == 0x2F)
                        {
                            i++;
                            
                            if(myencoding_prescan_stream_to_determine_encoding_check_meta(udata, &i, data_size, &encoding, found, found_length))
                                return encoding;
                        }
                    }
                    
                    break;
                    
                default:
                    i = myencoding_prescan_stream_to_determine_encoding_skip_other(udata, i, data_size);
                    break;
            }
        }
        else {
            i++;
        }
    }
    
    return encoding;
}



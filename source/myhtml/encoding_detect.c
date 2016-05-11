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

#include "myhtml/encoding.h"
#include "myhtml/encoding_detect_resource.h"
#include "myhtml/utils/resources.h"

myhtml_encoding_trigram_result_t myhtml_encoding_detect_by_trigram(unsigned const char *u_text, size_t length,
                                                                   const myhtml_encoding_trigram_t *list, size_t list_length,
                                                                   size_t max_sum_for_break)
{
    myhtml_encoding_trigram_result_t res = {0, 0};
    
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

bool myhtml_encoding_detect_russian_has_end(myhtml_encoding_trigram_result_t *res, size_t min_count, size_t min_value)
{
    if(res->value >= min_value || res->count >= min_count)
        return true;
    
    return false;
}

bool myhtml_encoding_detect_unicode_has_end(myhtml_encoding_unicode_result_t *res, size_t max_bad_percent)
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

myhtml_encoding_unicode_result_t myhtml_encoding_detect_utf_8(unsigned const char *u_text, size_t length)
{
    size_t i = 0;
    myhtml_encoding_unicode_result_t res = {0, 0, 0};
    
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

myhtml_encoding_unicode_result_t myhtml_encoding_detect_utf_16(unsigned const char *u_text, size_t length)
{
    size_t i = 0;
    myhtml_encoding_unicode_result_t res = {0, 0, 0};
    
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

bool myhtml_encoding_detect_bom(const char *text, size_t length, myhtml_encoding_t *encoding)
{
    unsigned const char *u_text = (unsigned const char*)text;
    
    if(length > 2) {
        if(u_text[0] == 0xEF &&
           u_text[1] == 0xBB &&
           u_text[2] == 0xBF)
        {
            *encoding = MyHTML_ENCODING_UTF_8;
            return true;
        }
    }
    
    if(length > 1) {
        if(u_text[0] == 0xFE && u_text[1] == 0xFF) {
            *encoding = MyHTML_ENCODING_UTF_16BE;
            return true;
        }
        
        if(u_text[0] == 0xFF && u_text[1] == 0xFE) {
            *encoding = MyHTML_ENCODING_UTF_16LE;
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
//            *encoding = MyHTML_ENCODING_UTF_32BE;
//            return true;
//        }
//        
//        if(u_text[0] == 0xFF &&
//           u_text[1] == 0xFE &&
//           u_text[2] == 0x00 &&
//           u_text[3] == 0x00)
//        {
//            *encoding = MyHTML_ENCODING_UTF_32LE;
//            return true;
//        }
//    }
    
    return false;
}

bool myhtml_encoding_detect_and_cut_bom(const char *text, size_t length, myhtml_encoding_t *encoding, const char **new_text, size_t *new_size)
{
    if(myhtml_encoding_detect_bom(text, length, encoding))
    {
        if(*encoding == MyHTML_ENCODING_UTF_8) {
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

bool myhtml_encoding_detect_unicode(const char *text, size_t length, myhtml_encoding_t *encoding)
{
    unsigned const char *u_text = (unsigned const char*)text;
    *encoding = MyHTML_ENCODING_DEFAULT;
    
    myhtml_encoding_unicode_result_t res = myhtml_encoding_detect_utf_16(u_text, length);
    
    if(res.count_bad == 0 && res.count_good >= 3) {
        *encoding = MyHTML_ENCODING_UTF_16LE;
        return true;
    }
    else if(res.count_bad >= 3 && res.count_good == 0) {
        *encoding = MyHTML_ENCODING_UTF_16BE;
        return true;
    }
    
    res = myhtml_encoding_detect_utf_8(u_text, length);
    if(myhtml_encoding_detect_unicode_has_end(&res, 10)) {
        *encoding = MyHTML_ENCODING_UTF_8;
        return true;
    }
    
    return false;
}

bool myhtml_encoding_detect_russian(const char *text, size_t length, myhtml_encoding_t *encoding)
{
    unsigned const char *u_text = (unsigned const char*)text;
    
    size_t min_count = 50;
    size_t min_value = 100000;
    size_t max_value = 0;
    
    *encoding = MyHTML_ENCODING_DEFAULT;
    
    myhtml_encoding_trigram_result_t
    res = myhtml_encoding_detect_by_trigram(u_text, length, myhtml_encoding_detect_trigrams_index_windows_1251, 1000, min_value);
    if(myhtml_encoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyHTML_ENCODING_WINDOWS_1251;
        return true;
    }
    
    max_value = res.value;
    if(max_value) {
        *encoding = MyHTML_ENCODING_WINDOWS_1251;
    }
    
    res = myhtml_encoding_detect_by_trigram(u_text, length, myhtml_encoding_detect_trigrams_index_koi8_r, 1000, min_value);
    if(myhtml_encoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyHTML_ENCODING_KOI8_R;
        return true;
    }
    
    if(max_value < res.value) {
        *encoding = MyHTML_ENCODING_KOI8_R;
        max_value = res.value;
    }
    
    res = myhtml_encoding_detect_by_trigram(u_text, length, myhtml_encoding_detect_trigrams_index_iso_8859_5, 1000, min_value);
    if(myhtml_encoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyHTML_ENCODING_ISO_8859_5;
        return true;
    }
    
    if(max_value < res.value) {
        *encoding = MyHTML_ENCODING_ISO_8859_5;
        max_value = res.value;
    }
    
    res = myhtml_encoding_detect_by_trigram(u_text, length, myhtml_encoding_detect_trigrams_index_x_mac_cyrillic, 1000, min_value);
    if(myhtml_encoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyHTML_ENCODING_X_MAC_CYRILLIC;
        return true;
    }
    
    if(max_value < res.value) {
        *encoding = MyHTML_ENCODING_X_MAC_CYRILLIC;
        max_value = res.value;
    }
    
    res = myhtml_encoding_detect_by_trigram(u_text, length, myhtml_encoding_detect_trigrams_index_ibm866, 1000, min_value);
    if(myhtml_encoding_detect_russian_has_end(&res, min_count, min_value)) {
        *encoding = MyHTML_ENCODING_IBM866;
        return true;
    }
    
    if(max_value < res.value) {
        *encoding = MyHTML_ENCODING_IBM866;
    }
    
    return false;
}

bool myhtml_encoding_detect(const char *text, size_t length, myhtml_encoding_t *encoding)
{
    *encoding = MyHTML_ENCODING_DEFAULT;
    
    if(myhtml_encoding_detect_unicode(text, length, encoding))
        return true;
    
    if(myhtml_encoding_detect_russian(text, length, encoding))
        return true;
    
    return false;
}

const myhtml_encoding_detect_name_entry_t * myhtml_encoding_name_entry_by_name(const char* name, size_t length)
{
    size_t idx = ((myhtml_string_chars_lowercase_map[ (const unsigned char)name[0] ] *
                   myhtml_string_chars_lowercase_map[ (const unsigned char)name[(length - 1)] ] *
                   length)
                  % MyHTML_ENCODING_DETECT_NAME_STATIC_SIZE) + 1;
    
    while (myhtml_encoding_detect_name_entry_static_list_index[idx].label)
    {
        if(myhtml_encoding_detect_name_entry_static_list_index[idx].label_length == length) {
            if(myhtml_strncasecmp(myhtml_encoding_detect_name_entry_static_list_index[idx].label, name, length) == 0)
                return &myhtml_encoding_detect_name_entry_static_list_index[idx];
            
            if(myhtml_encoding_detect_name_entry_static_list_index[idx].next)
                idx = myhtml_encoding_detect_name_entry_static_list_index[idx].next;
            else
                return NULL;
        }
        else if(myhtml_encoding_detect_name_entry_static_list_index[idx].label_length > length) {
            return NULL;
        }
        else {
            idx = myhtml_encoding_detect_name_entry_static_list_index[idx].next;
        }
    }
    
    return NULL;
}

bool myhtml_encoding_by_name(const char *name, size_t length, myhtml_encoding_t *encoding)
{
    const myhtml_encoding_detect_name_entry_t *entry = myhtml_encoding_name_entry_by_name(name, length);
    
    if(entry) {
        if(encoding)
            *encoding = entry->encoding;
        
        return true;
    }
    
    return false;
}






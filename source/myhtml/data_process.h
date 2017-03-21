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

#ifndef MyHTML_DATA_PROCESS_H
#define MyHTML_DATA_PROCESS_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/charef.h"
#include "myhtml/mystring.h"

struct myhtml_data_process_entry {
    /* current state for process data */
    myhtml_data_process_state_f state;
    
    /* for encodings */
    myencoding_t encoding;
    myencoding_result_t res;
    
    /* temp */
    size_t tmp_str_pos_proc;
    size_t tmp_str_pos;
    size_t tmp_num;
    
    /* &lt; current result */
    charef_entry_result_t charef_res;
    
    /* settings */
    bool is_attributes;
    bool emit_null_char;
};

void myhtml_data_process_entry_clean(myhtml_data_process_entry_t* proc_entry);

void myhtml_data_process(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t size);
void myhtml_data_process_end(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str);

size_t myhtml_data_process_state_data(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size);
size_t myhtml_data_process_state_ampersand(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size);
size_t myhtml_data_process_state_ampersand_data(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size);
size_t myhtml_data_process_state_ampersand_hash(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size);
size_t myhtml_data_process_state_ampersand_hash_data(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char *data, size_t offset, size_t size);
size_t myhtml_data_process_state_ampersand_hash_x_data(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str, const char* data, size_t offset, size_t size);
void   myhtml_data_process_state_end(myhtml_data_process_entry_t* proc_entry, mycore_string_t* str);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyHTML_DATA_PROCESS_H */


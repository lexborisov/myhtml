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
#include "myencoding/resource.h"
#include "mycore/utils/resources.h"

myencoding_custom_f myencoding_get_function_by_id(myencoding_t idx)
{
    return myencoding_function_index[idx];
}

myencoding_status_t myencoding_decode_utf_8(unsigned const char data, myencoding_result_t *res)
{
    // res->first  -- lower boundary
    // res->second -- upper boundary
    // res->result -- code point
    // res->third  -- bytes seen
    // res->flag   -- bytes needed
    
    if(res->flag == 0)
    {
        if(data <= 0x7F) {
            res->result = data;
            return MyENCODING_STATUS_OK;
        }
        else if(data >= 0xC2 && data <= 0xDF) {
            res->flag = 1;
            res->result = data - 0xC0;
        }
        else if(data >= 0xE0 && data <= 0xEF) {
            if(data == 0xE0) {
                res->first = 0xA0;
            }
            else if(data == 0xED){
                res->second = 0x9F;
            }
            
            res->flag = 2;
            res->result = data - 0xE0;
        }
        else if(data >= 0xF0 && data <= 0xF4) {
            if(data == 0xF0) {
                res->first = 0x90;
            }
            else if(data == 0xF4){
                res->second = 0x8F;
            }
            
            res->flag = 2;
            res->result = data - 0xF0;
        }
        else
            return MyENCODING_STATUS_ERROR;
        
        res->result = res->result << (6 * res->flag);
        return MyENCODING_STATUS_CONTINUE;
    }
    
    if(data < res->first && data > res->second)
    {
        res->result = 0x00;
        res->flag   = 0x00;
        res->third  = 0x00;
        res->first  = 0x80;
        res->second = 0xBF;
        
        return MyENCODING_STATUS_DONE|MyENCODING_STATUS_ERROR;
    }
    
    res->first  = 0x80;
    res->second = 0xBF;
    
    res->third++;
    res->result += (unsigned long)(data - 0x80) << (6 * (res->flag - res->third));
    
    if(res->third != res->flag)
        return MyENCODING_STATUS_CONTINUE;
    
    res->flag  = 0x00;
    res->third = 0x00;
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_ibm866(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_ibm866[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_2(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_2[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_3(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_3[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_4(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_4[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_5(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_5[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_6(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_6[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_7(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_7[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_8(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_8[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_8_i(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_8[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_10(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_10[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_13(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_13[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_14(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_14[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_15(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_15[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_iso_8859_16(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_iso_8859_16[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_koi8_r(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_koi8_r[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_koi8_u(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_koi8_u[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_macintosh(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_macintosh[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_874(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_874[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_1250(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_1250[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_1251(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_1251[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_1252(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_1252[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_1253(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_1253[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_1254(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_1254[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_1255(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_1255[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_1256(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_1256[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_1257(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_1257[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_windows_1258(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_windows_1258[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_x_mac_cyrillic(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        (res->result = data);
    else
        (res->result = myencoding_map_x_mac_cyrillic[(data - 0x80)]);
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_gbk(unsigned const char data, myencoding_result_t *res)
{
    return myencoding_decode_gb18030(data, res);
}

unsigned long myencoding_index_gb18030_ranges_code_point(unsigned long pointer)
{
    // step 1
    if((pointer > 39419 && pointer < 189000) || pointer > 1237575)
        return 0;
    
    // step 2
    if(pointer == 7457)
        return 0xe7c7;
    
    // step 3
    unsigned long offset = sizeof(myencoding_map_gb18030_ranges) / (sizeof(unsigned long) * 2);
    
    unsigned long code_point_offset = 0;
    while (offset) {
        offset--;
        
        if(myencoding_map_gb18030_ranges[offset][0] == pointer ||
           myencoding_map_gb18030_ranges[offset][0] < pointer)
        {
            code_point_offset = myencoding_map_gb18030_ranges[offset][1];
            break;
        }
    }
    
    // step 4
    return (code_point_offset + pointer - offset);
}

myencoding_status_t myencoding_decode_gb18030(unsigned const char data, myencoding_result_t *res)
{
    if(res->third) {
        if(data >= 0x30 && data <= 0x39) {
            res->result = myencoding_index_gb18030_ranges_code_point((((res->first - 0x81) * 10 + res->second - 0x30) *
                                                                          126 + res->third - 0x81) * 10 + data - 0x30);
        }
        
        res->first  = 0;
        res->second = 0;
        res->third  = 0;
        
        if(res->result)
            return MyENCODING_STATUS_OK;
        
        return MyENCODING_STATUS_ERROR;
    }
    else if(res->second) {
        if(data >= 0x81 && data <= 0xFE) {
            res->third = data;
            return MyENCODING_STATUS_CONTINUE;
        }
        
        res->first = 0;
        res->second = 0;
        
        return MyENCODING_STATUS_ERROR;
    }
    else if(res->first) {
        if(data >= 0x30 && data <= 0x39) {
            res->second = data;
            return MyENCODING_STATUS_CONTINUE;
        }
        
        unsigned long lead = res->first, pointer = 0x00;
        res->first = 0x00;
        
        unsigned char offset;
        
        if(data < 0x7F)
            offset = 0x40;
        else
            offset = 0x41;
        
        if((data >= 0x40 && data <= 0x7E) ||
           (data >= 0x80 && data <= 0xFE))
        {
            pointer = (lead - 0x81) * 190 + (data - offset);
        }
        else {
            return MyENCODING_STATUS_ERROR;
        }
        
        res->result = myencoding_map_gb18030[pointer];
        return MyENCODING_STATUS_OK;
    }
    
    if(data <= 0x7F) {
        res->result = data;
        return MyENCODING_STATUS_OK;
    }
    
    if(data == 0x80) {
        res->result = 0x20ac;
        return MyENCODING_STATUS_OK;
    }
    
    if(data >= 0x81 && data <= 0xFE) {
        res->first = data;
        return MyENCODING_STATUS_CONTINUE;
    }
    
    return MyENCODING_STATUS_ERROR;
}

myencoding_status_t myencoding_decode_big5(unsigned const char data, myencoding_result_t *res)
{
    if(res->first)
    {
        unsigned long lead = res->first;
        unsigned long pointer = 0x00;
        unsigned long offset;
        
        res->first = 0x00;
        
        if(data < 0x7F)
            offset = 0x40;
        else
            offset = 0x62;
        
        if((data >= 0x40 && data <= 0x7E) ||
           (data >= 0xA1 && data <= 0xFE))
        {
            pointer = (lead - 0x81) * 157 + (data - offset);
        }
        
        switch (pointer) {
            case 1133:
                // U+00CA U+0304
                res->result     = 0x00ca;
                res->result_aux = 0x0304;
                return MyENCODING_STATUS_OK;
            case 1135:
                // U+00CA U+030C
                res->result     = 0x00ca;
                res->result_aux = 0x030c;
                return MyENCODING_STATUS_OK;
            case 1164:
                // U+00EA U+0304
                res->result     = 0x00ea;
                res->result_aux = 0x0304;
                return MyENCODING_STATUS_OK;
            case 1166:
                // U+00EA U+030C
                res->result     = 0x00ea;
                res->result_aux = 0x030c;
                return MyENCODING_STATUS_OK;
            default:
                break;
        }
        
        if(pointer == 0)
            return MyENCODING_STATUS_ERROR;
        
        res->result = myencoding_map_big5[pointer];
        res->result_aux = 0;
        
        return MyENCODING_STATUS_OK;
    }
    
    if(data <= 0x7F) {
        res->result = data;
        return MyENCODING_STATUS_OK;
    }
    
    if(data >= 0x81 && data <= 0xFE) {
        res->first = data;
        return MyENCODING_STATUS_CONTINUE;
    }
    
    return MyENCODING_STATUS_ERROR;
}

myencoding_status_t myencoding_decode_euc_jp(unsigned const char data, myencoding_result_t *res)
{
    if(res->first == 0x8E && (data >= 0xA1 && data <= 0xDF)) {
        res->first = 0x00;
        res->result = 0xFF61 + data - 0xA1;
        
        return MyENCODING_STATUS_OK;
    }
    else if(res->first == 0x8F && (data >= 0xA1 && data <= 0xFE)) {
        res->flag = 1;
        res->first = data;
        
        return MyENCODING_STATUS_CONTINUE;
    }
    else if(res->first)
    {
        unsigned long lead = res->first;
        res->first = 0x00;
        
        if((lead >= 0xA1 && lead <= 0xFE) &&
           (data >= 0xA1 && data <= 0xFE))
        {
            unsigned long idx = (lead - 0xA1) * 94 + data - 0xA1;
            if(res->flag) {
                res->result = myencoding_map_jis0212[idx];
            }
            else {
                res->result = myencoding_map_jis0208[idx];
            }
            
            res->flag = 0;
            
            if(res->result == 0)
                return MyENCODING_STATUS_ERROR;
            
            return MyENCODING_STATUS_OK;
        }
    }
    
    if(data <= 0x7F) {
        res->result = data;
        return MyENCODING_STATUS_OK;
    }
    
    if((data >= 0x8E && data <= 0x8F) ||
       (data >= 0xA1 && data <= 0xFE))
    {
        res->first = data;
        return MyENCODING_STATUS_CONTINUE;
    }
    
    return MyENCODING_STATUS_ERROR;
}

myencoding_status_t myencoding_decode_iso_2022_jp(unsigned const char data, myencoding_result_t *res)
{
    // res->first  -- lead
    // res->second -- state
    // res->third  -- output state
    // res->flag   -- output flag
    
    switch (res->second) {
        case 0: // ASCII
        {
            if(data == 0x1B) {
                res->second = 6;
                return MyENCODING_STATUS_CONTINUE;
            }
            else if((data <= 0x7F) &&
                    data != 0x0E && data != 0x0F && data != 0x1B)
            {
                res->flag   = 0;
                res->result = data;
                return MyENCODING_STATUS_OK;
            }
            
            res->flag = 0;
            return MyENCODING_STATUS_ERROR;
        }
            
        case 1: // Roman
        {
            if(data == 0x1B) {
                res->second = 6;
                return MyENCODING_STATUS_CONTINUE;
            }
            else if(data == 0x5C) {
                res->flag   = 0;
                res->result = 0x00A5;
                return MyENCODING_STATUS_OK;
            }
            else if(data == 0x7E) {
                res->flag   = 0;
                res->result = 0x203E;
                return MyENCODING_STATUS_OK;
            }
            else if((data <= 0x7F) &&
                    data != 0x0E && data != 0x0F &&
                    data != 0x1B && data != 0x5C &&
                    data != 0x7E)
            {
                res->flag   = 0;
                res->result = data;
                return MyENCODING_STATUS_OK;
            }
            
            res->flag = 0;
            return MyENCODING_STATUS_ERROR;
        }
            
        case 3: // Katakana
        {
            if(data == 0x1B) {
                res->second = 6;
                return MyENCODING_STATUS_CONTINUE;
            }
            else if(data >= 0x21 && data <= 0x5F) {
                res->flag   = 0;
                res->result = 0xFF61 + data - 0x21;
                
                return MyENCODING_STATUS_OK;
            }
            
            res->flag = 0;
            return MyENCODING_STATUS_ERROR;
        }
            
        case 4: // Lead byte
        {
            if(data == 0x1B) {
                res->second = 6;
                return MyENCODING_STATUS_CONTINUE;
            }
            else if(data >= 0x21 && data <= 0x7E) {
                res->flag   = 0;
                res->first  = data;
                res->second = 5;
                
                return MyENCODING_STATUS_CONTINUE;
            }
            
            res->flag = 0;
            return MyENCODING_STATUS_ERROR;
        }
            
        case 5: // Trail byte
        {
            if(data == 0x1B) {
                res->second = 6;
                return MyENCODING_STATUS_CONTINUE|MyENCODING_STATUS_ERROR;
            }
            else if(data >= 0x21 && data <= 0x7E) {
                res->second = 4;
                
                unsigned long pointer = (res->first - 0x21) * 94 + data - 0x21;
                res->result = myencoding_map_jis0208[pointer];
                
                if(res->result == 0)
                    return MyENCODING_STATUS_ERROR;
                
                return MyENCODING_STATUS_OK;
            }
            
            res->second = 4;
            return MyENCODING_STATUS_ERROR;
        }
            
        case 6: // Escape start
        {
            if(data == 0x24 || data == 0x28) {
                res->first = data;
                res->second = 7;
                
                return MyENCODING_STATUS_CONTINUE;
            }
            
            res->flag   = 0;
            res->second = res->third;
            
            return MyENCODING_STATUS_ERROR;
        }
            
        case 7: // Escape
        {
            unsigned long lead = res->first;
            res->first = 0x00;
            
            res->second = 0x00;
            
            if(lead == 0x28 && data == 0x42) {
                res->second = 0;
            }
            else if(lead == 0x28 && data == 0x4A) {
                res->second = 1;
            }
            else if(lead == 0x28 && data == 0x49) {
                res->second = 2;
            }
            else if(lead == 0x24 && (data == 0x40 || data == 0x42)) {
                res->second = 4;
            }
            
            if(res->second)
            {
                res->third = res->second;
                
                unsigned long output_flag = res->flag;
                res->flag = 1;
                
                if(output_flag)
                    return MyENCODING_STATUS_ERROR;
                
                return MyENCODING_STATUS_CONTINUE;
            }
            
            res->flag = 0;
            res->second = res->third;
            
            return MyENCODING_STATUS_ERROR;
        }
            
        default:
            break;
    }
    
    return 0;
}

myencoding_status_t myencoding_decode_shift_jis(unsigned const char data, myencoding_result_t *res)
{
    // res->first -- lead
    
    if(res->first)
    {
        unsigned long offset;
        unsigned long lead_offset;
        
        if(data < 0x7F)
            offset = 0x40;
        else
            offset = 0x41;
        
        if(res->first < 0xA0)
            lead_offset = 0x81;
        else
            lead_offset = 0xC1;
        
        unsigned long pointer = 0x00;
        if((data >= 0x40 && data <= 0x7E) ||
           (data >= 0x80 && data <= 0xFC))
        {
            pointer = (res->first - lead_offset) * 188 + data - offset;
        }
        
        if(pointer)
            res->result = myencoding_map_jis0208[pointer];
        else
            res->result = 0x00;
        
        if(res->result == 0x00 && (pointer >= 8836 && pointer <= 10528)) {
            res->result = 0xE000 + pointer - 8836;
        }
        
        if(res->result)
            return MyENCODING_STATUS_OK;
        
        return MyENCODING_STATUS_ERROR;
    }
    
    if((data <= 0x7F) || data == 0x80) {
        res->result = data;
        return MyENCODING_STATUS_OK;
    }
    
    if(data >= 0xA1 && data <= 0xDF) {
        res->result = 0xFF61 + data - 0xA1;
        return MyENCODING_STATUS_OK;
    }
    
    if((data >= 0x81 && data <= 0x9F) ||
       (data >= 0xE0 && data <= 0xFC)) {
        res->first = data;
        return MyENCODING_STATUS_CONTINUE;
    }
    
    return MyENCODING_STATUS_ERROR;
}

myencoding_status_t myencoding_decode_euc_kr(unsigned const char data, myencoding_result_t *res)
{
    // res->first -- lead
    
    if(res->first)
    {
        unsigned long lead = res->first, pointer = 0x00;
        res->first = 0x00;
        
        if(data >= 0x41 && data <= 0xFE) {
            pointer = (lead - 0x81) * 190 + (data - 0x41);
        }
        
        if(pointer)
            res->result = myencoding_map_euc_kr[pointer];
        
        if(res->result)
            return MyENCODING_STATUS_OK;
        
        return MyENCODING_STATUS_ERROR;
    }
    
    if(data <= 0x7F) {
        res->result = data;
        return MyENCODING_STATUS_OK;
    }
    
    if(data >= 0x81 && data <= 0xFE) {
        res->first = data;
        return MyENCODING_STATUS_CONTINUE;
    }
    
    return MyENCODING_STATUS_ERROR;
}

myencoding_status_t myencoding_decode_shared_utf_16(unsigned const char data, myencoding_result_t *res)
{
    // res->first  -- lead
    // res->second -- lead surrogate
    // res->flag   -- flag
    
    if(res->first == 0x00) {
        res->first = data;
        return MyENCODING_STATUS_CONTINUE;
    }
    
    unsigned long code_unit;
    if(res->flag)
        (code_unit = (res->first << 8) + data);
    else
        (code_unit = (unsigned long)(data << 8) + res->first);
    
    res->first = 0x00;
    
    if(res->second) {
        unsigned long lead_surrogate = res->second;
        res->second = 0x00;
        
        if(code_unit >= 0xDC00 && code_unit <= 0xDFFF) {
            res->result = 0x10000 + ((lead_surrogate - 0xD800) << 10) + (code_unit - 0xDC00);
            return MyENCODING_STATUS_OK;
        }
        
        unsigned char byte1 = (unsigned char)(code_unit >> 8);
        unsigned char byte2 = (unsigned char)(code_unit & 0x00FF);
        
        if(res->flag) {
            res->result     = byte1;
            res->result_aux = byte2;
        }
        else {
            res->result     = byte2;
            res->result_aux = byte1;
        }
        
        return MyENCODING_STATUS_DONE|MyENCODING_STATUS_ERROR;
    }
    
    if(code_unit >= 0xD800 && code_unit <= 0xDBFF) {
        res->second = code_unit;
        return MyENCODING_STATUS_CONTINUE;
    }
    
    if(code_unit >= 0xDC00 && code_unit <= 0xDFFF) {
        return MyENCODING_STATUS_ERROR;
    }
    
    res->result = code_unit;
    
    return MyENCODING_STATUS_OK;
}

myencoding_status_t myencoding_decode_utf_16be(unsigned const char data, myencoding_result_t *res)
{
    if(res->flag == 0)
        res->flag = 1;
    
    return myencoding_decode_shared_utf_16(data, res);
}

myencoding_status_t myencoding_decode_utf_16le(unsigned const char data, myencoding_result_t *res)
{
    if(res->flag)
        res->flag = 0;
    
    return myencoding_decode_shared_utf_16(data, res);
}

myencoding_status_t myencoding_decode_x_user_defined(unsigned const char data, myencoding_result_t *res)
{
    if(data <= 0x7F)
        res->result = data;
    else
        res->result = 0xF780 + data - 0x80;
    
    return MyENCODING_STATUS_OK;
}

void myencoding_result_clean(myencoding_result_t *res)
{
    memset(res, 0, sizeof(myencoding_result_t));
}

size_t myencoding_codepoint_ascii_length(size_t codepoint)
{
    if (codepoint <= 0x0000007F) {
        return 1;
    }
    else if (codepoint <= 0x000007FF) {
        return 2;
    }
    else if (codepoint <= 0x0000FFFF) {
        return 3;
    }
    else if (codepoint <= 0x001FFFFF) {
        return 4;
    }
    /* not uses in unicode */
    else if (codepoint <= 0x03FFFFFF) {
        return 5;
    }
    else if (codepoint <= 0x7FFFFFFF) {
        return 6;
    }
    
    return 0;
}

size_t myencoding_ascii_utf_8_length(const unsigned char data)
{
    if (data < 0x80){
        return 1;
    }
    else if ((data & 0xe0) == 0xc0) {
        return 2;
    }
    else if ((data & 0xf0) == 0xe0) {
        return 3;
    }
    else if ((data & 0xf8) == 0xf0) {
        return 4;
    }
    else if ((data & 0xfc) == 0xf8) {
        return 5;
    }
    else if ((data & 0xff) == 0xfc) {
        return 6;
    }
    
    return 0;
}

size_t myencoding_codepoint_to_ascii_utf_8(size_t codepoint, char *data)
{
    /* 0x80 -- 10xxxxxx */
    /* 0xC0 -- 110xxxxx */
    /* 0xE0 -- 1110xxxx */
    /* 0xF0 -- 11110xxx */
    /* 0xF8 -- 111110xx */
    /* 0xFC -- 1111110x */
    
    if (codepoint <= 0x0000007F) {
        /* 0xxxxxxx */
        data[0] = (char)codepoint;
        return 1;
    }
    else if (codepoint <= 0x000007FF) {
        /* 110xxxxx 10xxxxxx */
        data[0] = (char)(0xC0 | (codepoint >> 6  ));
        data[1] = (char)(0x80 | (codepoint & 0x3F));
        
        return 2;
    }
    else if (codepoint <= 0x0000FFFF) {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        data[0] = (char)(0xE0 | ((codepoint >> 12)));
        data[1] = (char)(0x80 | ((codepoint >> 6 ) & 0x3F));
        data[2] = (char)(0x80 | ( codepoint & 0x3F));
        
        return 3;
    }
    else if (codepoint <= 0x001FFFFF) {
        /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        data[0] = (char)(0xF0 | ( codepoint >> 18));
        data[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        data[2] = (char)(0x80 | ((codepoint >> 6 ) & 0x3F));
        data[3] = (char)(0x80 | ( codepoint & 0x3F));
        
        return 4;
    }
    /* not uses in unicode */
    //    else if (codepoint <= 0x03FFFFFF) {
    //        /* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
    //        data[0] = 0xF8 | ( codepoint >> 24);
    //        data[1] = 0x80 | ((codepoint >> 18) & 0x3F);
    //        data[2] = 0x80 | ((codepoint >> 12) & 0x3F);
    //        data[3] = 0x80 | ((codepoint >> 6 ) & 0x3F);
    //        data[4] = 0x80 | ( codepoint & 0x3F);
    //
    //        return 5;
    //    }
    //    else if (codepoint <= 0x7FFFFFFF) {
    //        /* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
    //        data[0] = 0xFC | ( codepoint >> 30);
    //        data[1] = 0x80 | ((codepoint >> 24) & 0x3F);
    //        data[2] = 0x80 | ((codepoint >> 18) & 0x3F);
    //        data[3] = 0x80 | ((codepoint >> 12) & 0x3F);
    //        data[4] = 0x80 | ((codepoint >> 6 ) & 0x3F);
    //        data[5] = 0x80 | ( codepoint & 0x3F);
    //
    //        return 6;
    //    }
    
    return 0;
}

size_t myencoding_codepoint_to_lowercase_ascii_utf_8(size_t codepoint, char *data)
{
    /* 0x80 -- 10xxxxxx */
    /* 0xC0 -- 110xxxxx */
    /* 0xE0 -- 1110xxxx */
    /* 0xF0 -- 11110xxx */
    /* 0xF8 -- 111110xx */
    /* 0xFC -- 1111110x */
    
    if (codepoint <= 0x0000007F) {
        /* 0xxxxxxx */
        data[0] = (char)mycore_string_chars_lowercase_map[ codepoint ];
        return 1;
    }
    else if (codepoint <= 0x000007FF) {
        /* 110xxxxx 10xxxxxx */
        data[0] = (char)(0xC0 | (codepoint >> 6  ));
        data[1] = (char)(0x80 | (codepoint & 0x3F));
        
        return 2;
    }
    else if (codepoint <= 0x0000FFFF) {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        data[0] = (char)(0xE0 | ((codepoint >> 12)));
        data[1] = (char)(0x80 | ((codepoint >> 6 ) & 0x3F));
        data[2] = (char)(0x80 | ( codepoint & 0x3F));
        
        return 3;
    }
    else if (codepoint <= 0x001FFFFF) {
        /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        data[0] = (char)(0xF0 | ( codepoint >> 18));
        data[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        data[2] = (char)(0x80 | ((codepoint >> 6 ) & 0x3F));
        data[3] = (char)(0x80 | ( codepoint & 0x3F));
        
        return 4;
    }
    /* not uses in unicode */
    //    else if (codepoint <= 0x03FFFFFF) {
    //        /* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
    //        data[0] = 0xF8 | ( codepoint >> 24);
    //        data[1] = 0x80 | ((codepoint >> 18) & 0x3F);
    //        data[2] = 0x80 | ((codepoint >> 12) & 0x3F);
    //        data[3] = 0x80 | ((codepoint >> 6 ) & 0x3F);
    //        data[4] = 0x80 | ( codepoint & 0x3F);
    //
    //        return 5;
    //    }
    //    else if (codepoint <= 0x7FFFFFFF) {
    //        /* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
    //        data[0] = 0xFC | ( codepoint >> 30);
    //        data[1] = 0x80 | ((codepoint >> 24) & 0x3F);
    //        data[2] = 0x80 | ((codepoint >> 18) & 0x3F);
    //        data[3] = 0x80 | ((codepoint >> 12) & 0x3F);
    //        data[4] = 0x80 | ((codepoint >> 6 ) & 0x3F);
    //        data[5] = 0x80 | ( codepoint & 0x3F);
    //
    //        return 6;
    //    }
    
    return 0;
}

size_t myencoding_ascii_utf_8_to_codepoint(const unsigned char* data, size_t* codepoint)
{
    if (*data < 0x80){
        /* 0xxxxxxx */
        *codepoint = (size_t)*data;
        return 1;
    }
    else if ((*data & 0xe0) == 0xc0) {
        /* 110xxxxx 10xxxxxx */
        *codepoint  = (data[0] ^ (0xC0 & data[0])) << 6;
        *codepoint |= (data[1] ^ (0x80 & data[1]));
        
        return 2;
    }
    else if ((*data & 0xf0) == 0xe0) {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        *codepoint  = (data[0] ^ (0xE0 & data[0])) << 12;
        *codepoint |= (data[1] ^ (0x80 & data[1])) << 6;
        *codepoint |= (data[2] ^ (0x80 & data[2]));
        
        return 3;
    }
    else if ((*data & 0xf8) == 0xf0) {
        /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        *codepoint  = (data[0] ^ (0xF0 & data[0])) << 18;
        *codepoint |= (data[1] ^ (0x80 & data[1])) << 12;
        *codepoint |= (data[2] ^ (0x80 & data[2])) << 6;
        *codepoint |= (data[3] ^ (0x80 & data[3]));
        
        return 4;
    }
    
    return 0;
}

size_t myencoding_codepoint_to_ascii_utf_16(size_t codepoint, char *data)
{
    if((codepoint >> 16)) {
        codepoint -= 0x10000;
        
        size_t high = 0xD800 | (codepoint >> 10);
        size_t low = 0xDC00 | (codepoint & 0x3FF);
        
        data[0] = (char)(high >> 8);
        data[1] = (char)high;
        data[2] = (char)(low >> 8);
        data[3] = (char)low;
        
        return 4;
    }
    
    data[0] = (char)(codepoint >> 8);
    data[1] = (char)codepoint;
    
    return 2;
}

size_t myencoding_convert_to_ascii_utf_8(mycore_string_raw_t* raw_str, const char* buff, size_t length, myencoding_t encoding)
{
    if(raw_str->data == NULL) {
        raw_str->size   = length + 1;
        raw_str->length = 0;
        raw_str->data   = mycore_malloc(sizeof(char) * raw_str->size);
        
        if(raw_str->data == NULL)
            return 0;
    }
    
    myencoding_result_t res = {0};
    
    unsigned const char* u_buff = (unsigned const char*)buff;
    const myencoding_custom_f func = myencoding_get_function_by_id(encoding);
    
    size_t i;
    for (i = 0; i < length; i++)
    {
        if(func(u_buff[i], &res) == MyENCODING_STATUS_OK) {
            if((raw_str->length + 6) >= raw_str->size) {
                size_t new_size = raw_str->length + 6 + (length / 2);
                char *new_data  = mycore_realloc(raw_str->data, sizeof(char) * new_size);
                
                if(new_data == NULL) {
                    return 0;
                }
                
                raw_str->data = new_data;
                raw_str->size = new_size;
            }
            
            raw_str->length += myencoding_codepoint_to_ascii_utf_8(res.result, &raw_str->data[raw_str->length]);
        }
    }
    
    return i;
}



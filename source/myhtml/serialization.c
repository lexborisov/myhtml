/*
 Copyright (C) 2016 Alexander Borisov
 
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

#include "myhtml/serialization.h"

bool myhtml_serialization_append(myhtml_string_raw_t* str, const char* src_data, size_t length);
bool myhtml_serialization_append_attr(myhtml_string_raw_t* str, const char* src_data, size_t length);
bool myhtml_serialization_append_raw(myhtml_string_raw_t* str, const char* src_data, size_t length);

bool myhtml_serialization_attributes(myhtml_tree_t* tree, myhtml_tree_attr_t* attr, myhtml_string_raw_t* str);
bool myhtml_serialization_node_append_text_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_string_raw_t* str);
bool myhtml_serialization_node_append_close(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_string_raw_t* str);

bool myhtml_serialization(myhtml_tree_t* tree, myhtml_tree_node_t* scope_node, myhtml_string_raw_t* str)
{
    if(str == NULL)
        return false;
    
    if(str->data == NULL) {
        str->size   = 4098 * 5;
        str->length = 0;
        str->data   = (char*)myhtml_malloc(str->size * sizeof(char));
        
        if(str->data == NULL) {
            str->size = 0;
            return false;
        }
    }
    
    myhtml_tree_node_t* node = scope_node;
    
    if(node == tree->document) {
        if(tree->document)
            node = tree->document->child;
        else {
            myhtml_free(str->data);
            memset(str, 0, sizeof(myhtml_string_raw_t));
            return false;
        }
    }
    
    while(node) {
        if(myhtml_serialization_node(tree, node, str) == false) {
            myhtml_free(str->data);
            memset(str, 0, sizeof(myhtml_string_raw_t));
            return false;
        }
        
        if(node->child)
            node = node->child;
        else {
            while(node != scope_node && node->next == NULL) {
                myhtml_serialization_node_append_close(tree, node, str);
                node = node->parent;
            }
            
            if(node == scope_node) {
                if(node != tree->document) {
                    if(myhtml_serialization_node_append_close(tree, node, str) == false) {
                        myhtml_free(str->data);
                        memset(str, 0, sizeof(myhtml_string_raw_t));
                        return false;
                    }
                }
                
                break;
            }
            
            if(myhtml_serialization_node_append_close(tree, node, str) == false) {
                myhtml_free(str->data);
                memset(str, 0, sizeof(myhtml_string_raw_t));
                return false;
            }
            
            node = node->next;
        }
    }
    
    return true;
}

bool myhtml_serialization_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_string_raw_t* str)
{
    if(str == NULL)
        return false;
    
    if(str->data == NULL) {
        str->size   = 2048;
        str->length = 0;
        str->data   = (char*)myhtml_malloc(str->size * sizeof(char));
        
        if(str->data == NULL) {
            str->size = 0;
            return false;
        }
    }
    
    switch (node->tag_id) {
        case MyHTML_TAG__TEXT: {
            if(myhtml_serialization_node_append_text_node(tree, node, str) == false)
                return false;
            
            break;
        }
        case MyHTML_TAG__COMMENT: {
            if(myhtml_serialization_append_raw(str, "<!--", 4) == false)
                return false;
            
            if(node->token && node->token->str.data) {
                if(myhtml_serialization_append_raw(str, node->token->str.data, node->token->str.length) == false)
                    return false;
            }
            
            if(myhtml_serialization_append_raw(str, "-->", 3) == false)
                return false;
            
            break;
        }
        case MyHTML_TAG__DOCTYPE: {
            if(myhtml_serialization_append_raw(str, "<!DOCTYPE", 9) == false)
                return false;
            
            if(node->token) {
                myhtml_tree_attr_t* attr = node->token->attr_first;
                
                if(attr->key.data && attr->key.length) {
                    if(myhtml_serialization_append_raw(str, " ", 1) == false)
                        return false;
                    
                    if(myhtml_serialization_append_raw(str, attr->key.data, attr->key.length) == false)
                        return false;
                }
            }
            
            if(myhtml_serialization_append_raw(str, ">", 1) == false)
                return false;
            
            break;
        }
        default: {
            size_t length;
            const char *tag = myhtml_tag_name_by_id(tree, node->tag_id, &length);
            
            if(myhtml_serialization_append_raw(str, "<", 1) == false)
                return false;
            
            if(myhtml_serialization_append_raw(str, tag, length) == false)
                return false;
            
            if(node->token) {
                if(myhtml_serialization_attributes(tree, node->token->attr_first, str) == false)
                    return false;
            }
            
            if(myhtml_serialization_append_raw(str, ">", 1) == false)
                return false;
            
            break;
        }
    }
    
    return true;
}

bool myhtml_serialization_attributes(myhtml_tree_t* tree, myhtml_tree_attr_t* attr, myhtml_string_raw_t* str)
{
    while(attr) {
        if(myhtml_serialization_append_raw(str, " ", 1) == false)
            return false;
        
        switch (attr->ns) {
            case MyHTML_NAMESPACE_XML: {
                if(myhtml_serialization_append_raw(str, "xml:", 4) == false)
                    return false;
                
                break;
            }
            case MyHTML_NAMESPACE_XMLNS: {
                /*
                    If the attribute is in the XMLNS namespace and the attribute's local name is not xmlns
                    The attribute's serialized name is the string "xmlns:" followed by the attribute's local name.
                 */
                if(attr->key.data && attr->key.length == 5 && myhtml_strcmp(attr->key.data, "xmlns")) {
                    if(myhtml_serialization_append_raw(str, "xmlns:", 6) == false)
                        return false;
                }
                
                break;
            }
            case MyHTML_NAMESPACE_XLINK: {
                if(myhtml_serialization_append_raw(str, "xlink:", 6) == false)
                    return false;
                
                break;
            }
            default:
                break;
        }
        
        size_t length;
        const char *data = myhtml_attribute_key(attr, &length);
        if(data) {
            if(myhtml_serialization_append_raw(str, data, length) == false)
                return false;
        }
        
        if(myhtml_serialization_append_raw(str, "=\"", 2) == false)
            return false;
        
        data = myhtml_attribute_value(attr, &length);
        if(data) {
            if(myhtml_serialization_append_attr(str, data, length) == false)
                return false;
        }
        
        if(myhtml_serialization_append_raw(str, "\"", 1) == false)
            return false;
        
        attr = attr->next;
    }
    
    return true;
}

bool myhtml_serialization_node_append_close(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_string_raw_t* str)
{
    if(node->tag_id != MyHTML_TAG__TEXT &&
       node->tag_id != MyHTML_TAG__COMMENT &&
       node->tag_id != MyHTML_TAG__DOCTYPE)
    {
        size_t length;
        const char *tag = myhtml_tag_name_by_id(tree, node->tag_id, &length);
        
        if(myhtml_serialization_append_raw(str, "</", 2) == false)
            return false;
        
        if(myhtml_serialization_append_raw(str, tag, length) == false)
            return false;
        
        if(myhtml_serialization_append_raw(str, ">", 1) == false)
            return false;
    }
    
    return true;
}

bool myhtml_serialization_node_append_text_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_string_raw_t* str)
{
    if(node->token == NULL || node->token->str.data == NULL)
        return str;
    
    if(node->parent == NULL)
        return myhtml_serialization_append(str, node->token->str.data, node->token->str.length);
    
    switch (node->parent->tag_id) {
        case MyHTML_TAG_STYLE:
        case MyHTML_TAG_SCRIPT:
        case MyHTML_TAG_XMP:
        case MyHTML_TAG_IFRAME:
        case MyHTML_TAG_NOEMBED:
        case MyHTML_TAG_NOFRAMES:
        case MyHTML_TAG_PLAINTEXT:
            if(myhtml_serialization_append_raw(str, node->token->str.data, node->token->str.length) == false)
                return false;
            
            break;
            
        default:
            if(myhtml_serialization_append(str, node->token->str.data, node->token->str.length) == false)
                return false;
            
            break;
    }
    
    return true;
}

bool myhtml_serialization_append_raw(myhtml_string_raw_t* str, const char* src_data, size_t length)
{
    myhtml_serialization_realloc_if_need((length + 1))
    strncpy(&str->data[ str->length ], src_data, length);
    
    str->length += length;
    str->data[ str->length ] = '\0';
    
    return true;
}

bool myhtml_serialization_append(myhtml_string_raw_t* str, const char* src_data, size_t length)
{
    myhtml_serialization_realloc_if_need(length)
    
    for(size_t i = 0; i < length; i++) {
        if(src_data[i] == '&') {
            myhtml_serialization_realloc_if_need(6)
            
            strncpy(&str->data[str->length], "&amp;", 5);
            str->length += 5;
        }
        else if(src_data[i] == '<') {
            myhtml_serialization_realloc_if_need(5)
            
            strncpy(&str->data[str->length], "&lt;", 4);
            str->length += 4;
        }
        else if(src_data[i] == '>') {
            myhtml_serialization_realloc_if_need(5)
            
            strncpy(&str->data[str->length], "&gt;", 4);
            str->length += 4;
        }
        else if((unsigned char)src_data[i] == 0xC2) {
            i++;
            
            if(i >= length) {
                str->data[str->length] = (unsigned char)0xC2;
                
                str->length++;
                myhtml_serialization_realloc_if_need(1)
                
                break;
            }
            
            if((unsigned char)src_data[i] == 0xA0) {
                myhtml_serialization_realloc_if_need(7)
                
                strncpy(&str->data[str->length], "&nbsp;", 6);
                str->length += 6;
            }
            else {
                str->data[str->length] = src_data[i];
                
                str->length++;
                myhtml_serialization_realloc_if_need(1)
            }
        }
        else {
            str->data[str->length] = src_data[i];
            
            str->length++;
            myhtml_serialization_realloc_if_need(1)
        }
    }
    
    str->data[ str->length ] = '\0';
    
    return true;
}

bool myhtml_serialization_append_attr(myhtml_string_raw_t* str, const char* src_data, size_t length)
{
    myhtml_serialization_realloc_if_need(length)
    
    for(size_t i = 0; i < length; i++) {
        if(src_data[i] == '&') {
            myhtml_serialization_realloc_if_need(6)
            
            strncpy(&str->data[str->length], "&amp;", 5);
            str->length += 5;
        }
        else if(src_data[i] == '"') {
            myhtml_serialization_realloc_if_need(7)
            
            strncpy(&str->data[str->length], "&quot;", 6);
            str->length += 6;
        }
        else if((unsigned char)src_data[i] == 0xC2) {
            i++;
            
            if(i >= length) {
                str->data[str->length] = (unsigned char)0xC2;
                
                str->length++;
                myhtml_serialization_realloc_if_need(1)
                
                break;
            }
            
            if((unsigned char)src_data[i] == 0xA0) {
                myhtml_serialization_realloc_if_need(7)
                
                strncpy(&str->data[str->length], "&nbsp;", 6);
                str->length += 6;
            }
            else {
                str->data[str->length] = src_data[i];
                
                str->length++;
                myhtml_serialization_realloc_if_need(1)
            }
        }
        else {
            str->data[str->length] = src_data[i];
            
            str->length++;
            myhtml_serialization_realloc_if_need(1)
        }
    }
    
    str->data[ str->length ] = '\0';
    
    return true;
}



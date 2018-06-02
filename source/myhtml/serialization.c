/*
 Copyright (C) 2016-2017 Alexander Borisov
 
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
 Author: https://github.com/EmielBruijntjes (Emiel Bruijntjes)
*/

#include "myhtml/serialization.h"

/**
 *  Forward declaration of all the functions that are used inside this module
 */
static mystatus_t myhtml_serialization_append(const char* str, size_t size, mycore_callback_serialize_f callback, void *ptr);
static mystatus_t myhtml_serialization_append_attr(const char* str, size_t length, mycore_callback_serialize_f callback, void *ptr);
static mystatus_t myhtml_serialization_attributes(myhtml_tree_t* tree, myhtml_tree_attr_t* attr, mycore_callback_serialize_f callback, void *ptr);
static mystatus_t myhtml_serialization_node_append_text_node(myhtml_tree_node_t* node, mycore_callback_serialize_f callback, void *ptr);
static mystatus_t myhtml_serialization_node_append_close(myhtml_tree_node_t* node, mycore_callback_serialize_f callback, void *ptr);

/**
 *  See the function myhtml_serialization_tree_buffer
 */
mystatus_t myhtml_serialization(myhtml_tree_node_t* scope_node, mycore_string_raw_t* str)
{
    return myhtml_serialization_tree_buffer(scope_node, str);
}

/**
 *  See the function myhtml_serialization_node_buffer
 */
mystatus_t myhtml_serialization_node(myhtml_tree_node_t* node, mycore_string_raw_t* str)
{
    return myhtml_serialization_node_buffer(node, str);
}

/**
 *  The serialize function for an entire tree
 *  @param  tree        the tree to be serialized
 *  @param  scope_node  the scope_node 
 *  @param  callback    function that will be called for all strings that have to be printed
 *  @param  ptr         user-supplied pointer
 *  @return bool
 */
mystatus_t myhtml_serialization_tree_callback(myhtml_tree_node_t* scope_node, mycore_callback_serialize_f callback, void *ptr)
{
    myhtml_tree_node_t* node = scope_node;
    
    /*
     Document node it is a root node of tree, it is not a tag element.
     
     For HTML, the root element will always be <HTML> tag.
     It is always present in the HTML document and it always a root of HTML document.
     */
    
    if(scope_node && scope_node->tree && scope_node->tree->document == scope_node) {
        node = scope_node->child;
    }
    
    while(node) {
        if(myhtml_serialization_node_callback(node, callback, ptr))
            return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
        
        if(node->child)
            node = node->child;
        else {
            while(node != scope_node && node->next == NULL) {
                if(myhtml_serialization_node_append_close(node, callback, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                
                node = node->parent;
            }
            
            if(node == scope_node) {
                if(node != node->tree->document) {
                    if(myhtml_serialization_node_append_close(node, callback, ptr))
                        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                }
                
                break;
            }
            
            if(myhtml_serialization_node_append_close(node, callback, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            node = node->next;
        }
    }
    
    return MyCORE_STATUS_OK;
}

/**
 *  The serialize function for a single node
 *  @param  tree        the tree to be serialized
 *  @param  node        the node that is going to be serialized 
 *  @param  callback    function that will be called for all strings that have to be printed
 *  @param  ptr         user-supplied pointer
 *  @return bool
 */
mystatus_t myhtml_serialization_node_callback(myhtml_tree_node_t* node, mycore_callback_serialize_f callback, void *ptr)
{
    switch (node->tag_id) {
        case MyHTML_TAG__TEXT: {
            if(myhtml_serialization_node_append_text_node(node, callback, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            break;
        }
        case MyHTML_TAG__COMMENT: {
            if(callback("<!--", 4, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            if(node->token && node->token->str.data) {
                if(callback(node->token->str.data, node->token->str.length, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            }
            
            if(callback("-->", 3, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            break;
        }
        case MyHTML_TAG__DOCTYPE: {
            if(callback("<!DOCTYPE", 9, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            if(callback(" ", 1, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            if(node->token) {
                myhtml_tree_attr_t* attr = node->token->attr_first;
                
                if(attr && attr->key.data && attr->key.length) {
                    if(callback(attr->key.data, attr->key.length, ptr))
                        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                    
                    attr = attr->next;
                    
                    if(attr) {
                        myhtml_tree_attr_t *system_id = NULL, *public_id = NULL;
                        
                        if(attr->value.length == 6) {
                            if(mycore_strcasecmp(attr->value.data, "SYSTEM") == 0) {
                                system_id = attr->next;
                            } else if(mycore_strcasecmp(attr->value.data, "PUBLIC") == 0) {
                                public_id = attr->next;
                                system_id = public_id ? public_id->next : NULL;
                            }
                        }
                        
                        if(public_id && public_id->value.length > 0) {
                            if(callback(" PUBLIC \"", 9, ptr))
                                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                            
                            if(callback(public_id->value.data, public_id->value.length, ptr))
                                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                            
                            if(callback("\"", 1, ptr))
                                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                        }
                        
                        if(system_id && system_id->value.length > 0) {
                            if(!public_id || public_id->value.length == 0) {
                                if(callback(" SYSTEM", 7, ptr))
                                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                            }
                            
                            if(callback(" \"", 2, ptr))
                                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                            
                            if(system_id->value.data && system_id->value.length) {
                                if(callback(system_id->value.data, system_id->value.length, ptr))
                                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                            }
                            
                            if(callback("\"", 1, ptr))
                                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                        }
                    }
                }
            }
            
            if(callback(">", 1, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            break;
        }
        default: {
            size_t length;
            const char *tag = myhtml_tag_name_by_id(node->tree, node->tag_id, &length);

            if(callback("<", 1, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            if(callback(tag, length, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            if(node->token) {
                if(myhtml_serialization_attributes(node->tree, node->token->attr_first, callback, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            }
            
            if(callback(">", 1, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            break;
        }
    }
    
    return MyCORE_STATUS_OK;
}

/**
 *  Internal function to process attributes
 *  @param  tree
 *  @param  attr
 *  @param  callback
 *  @param  ptr
 */
mystatus_t myhtml_serialization_attributes(myhtml_tree_t* tree, myhtml_tree_attr_t* attr, mycore_callback_serialize_f callback, void* ptr)
{
    while(attr) {
        if(callback(" ", 1, ptr))
            return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
        
        switch (attr->ns) {
            case MyHTML_NAMESPACE_XML:
                if(callback("xml:", 4, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                
                break;
            case MyHTML_NAMESPACE_XMLNS: {
                /*
                    If the attribute is in the XMLNS namespace and the attribute's local name is not xmlns
                    The attribute's serialized name is the string "xmlns:" followed by the attribute's local name.
                 */
                if(attr->key.data && attr->key.length == 5 && mycore_strcmp(attr->key.data, "xmlns")) {
                    if(callback("xmlns:", 6, ptr))
                        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                }
                
                break;
            }
            case MyHTML_NAMESPACE_XLINK: {
                if(callback("xlink:", 6, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                
                break;
            }
            default:
                break;
        }
        
        
        size_t length;
        const char *data = myhtml_attribute_key(attr, &length);
        if(data) {
            if(callback(data, length, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
        }
        if(callback("=\"", 2, ptr))
            return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
        
        data = myhtml_attribute_value(attr, &length);
        if(data) {
            if(myhtml_serialization_append_attr(data, length, callback, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
        }
        if(callback("\"", 1, ptr))
            return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
        
        attr = attr->next;
    }
    
    return MyCORE_STATUS_OK;
}

/**
 *  Internal function to process a close tag
 *  @param  tree
 *  @param  node
 *  @param  callback
 *  @param  ptr
 */
mystatus_t myhtml_serialization_node_append_close(myhtml_tree_node_t* node, mycore_callback_serialize_f callback, void* ptr)
{
    switch (node->tag_id) {
        case MyHTML_TAG__TEXT:
        case MyHTML_TAG__COMMENT:
        case MyHTML_TAG__DOCTYPE:
            return MyCORE_STATUS_OK;
        
        // https://html.spec.whatwg.org/multipage/parsing.html#serialising-html-fragments
        case MyHTML_TAG_AREA:
        case MyHTML_TAG_BASE:
        case MyHTML_TAG_BGSOUND:
        case MyHTML_TAG_BR:
        case MyHTML_TAG_COL:
        case MyHTML_TAG_EMBED:
        case MyHTML_TAG_FRAME:
        case MyHTML_TAG_HR:
        case MyHTML_TAG_IMG:
        case MyHTML_TAG_INPUT:
        case MyHTML_TAG_KEYGEN:
        case MyHTML_TAG_LINK:
        case MyHTML_TAG_META:
        case MyHTML_TAG_PARAM:
        case MyHTML_TAG_SOURCE:
        case MyHTML_TAG_TRACK:
        case MyHTML_TAG_WBR:
            if(node->ns == MyHTML_NAMESPACE_HTML)
                return MyCORE_STATUS_OK;
            break;
    }
    
    size_t length;
    const char *tag = myhtml_tag_name_by_id(node->tree, node->tag_id, &length);
    
    if(callback("</", 2, ptr))
        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    
    if(callback(tag, length, ptr))
        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    
    if(callback(">", 1, ptr))
        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    
    return MyCORE_STATUS_OK;
}

/**
 *  Internal function to process a text node
 *  @param  tree
 *  @param  node
 *  @param  callback
 *  @param  ptr
 */
mystatus_t myhtml_serialization_node_append_text_node(myhtml_tree_node_t* node, mycore_callback_serialize_f callback, void* ptr)
{
    if(node->token == NULL || node->token->str.data == NULL) return MyCORE_STATUS_OK;
    
    if(node->parent == NULL) {
        if(myhtml_serialization_append(node->token->str.data, node->token->str.length, callback, ptr))
            return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
        
        return MyCORE_STATUS_OK;
    }
    
    switch (node->parent->tag_id) {
        case MyHTML_TAG_STYLE:
        case MyHTML_TAG_SCRIPT:
        case MyHTML_TAG_XMP:
        case MyHTML_TAG_IFRAME:
        case MyHTML_TAG_NOEMBED:
        case MyHTML_TAG_NOFRAMES:
        case MyHTML_TAG_PLAINTEXT:
            if(callback(node->token->str.data, node->token->str.length, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            break;
        default:
            if(myhtml_serialization_append(node->token->str.data, node->token->str.length, callback, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            break;
    }
    
    return MyCORE_STATUS_OK;
}

/**
 *  Internal method to process a buffer that could contain to-be-quoted output
 *  @param  data
 *  @param  size
 *  @param  callback
 *  @param  ptr
 */
mystatus_t myhtml_serialization_append(const char *data, size_t size, mycore_callback_serialize_f callback, void* ptr)
{
    // number of chars not yet displayed
    size_t notwritten = 0;
    
    // iterate over the buffer
    for (size_t i = 0; i < size; ++i) {
        switch ((unsigned char)data[i]) {
        case '&':
            if(notwritten) {
                if(callback(data + i - notwritten, notwritten, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            }
            
            if(callback("&amp;", 5, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            notwritten = 0;
            break;
        case '<':
            if(notwritten) {
                if(callback(data + i - notwritten, notwritten, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            }
            
            if(callback("&lt;", 4, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            notwritten = 0;
            break;
        case '>':
            if(notwritten) {
                if(callback(data + i - notwritten, notwritten, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            }
            
            if(callback("&gt;", 4, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            notwritten = 0;
            break;
        case 0xA0:
            if(i > 0 && (unsigned char)(data[(i - 1)]) == 0xC2) {
                if(notwritten) {
                    if(callback(data + i - notwritten, (notwritten - 1), ptr))
                        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                }
                
                if(callback("&nbsp;", 6, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                
                notwritten = 0;
            }
            else {
                ++notwritten;
            }
            
            break;
        default:
            ++notwritten;
            break;
        }
    }
    
    if(notwritten)
        if(callback(data + size - notwritten, notwritten, ptr))
            return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    
    return MyCORE_STATUS_OK;
}

/**
 *  Internal method to process an attribute
 *  @param  data
 *  @param  size
 *  @param  callback
 *  @param  ptr
 */
mystatus_t myhtml_serialization_append_attr(const char* data, size_t size, mycore_callback_serialize_f callback, void* ptr)
{
    // number of chars not yet displayed
    size_t notwritten = 0;
    
    // iterate over the buffer
    for (size_t i = 0; i < size; ++i) {
        switch ((unsigned char)data[i]) {
        case '&':
            if(notwritten) {
                if(callback(data + i - notwritten, notwritten, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            }
            
            if(callback("&amp;", 5, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            notwritten = 0;
            break;
        case '"':
            if(notwritten) {
                if(callback(data + i - notwritten, notwritten, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            }
            
            if(callback("&quot;", 6, ptr))
                return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
            
            notwritten = 0;
            break;
        case 0xA0:
            if(i > 0 && (unsigned char)(data[(i - 1)]) == 0xC2) {
                if(notwritten) {
                    if(callback(data + i - notwritten, (notwritten - 1), ptr))
                        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                }
                
                if(callback("&nbsp;", 6, ptr))
                    return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
                
                notwritten = 0;
            }
            else {
                ++notwritten;
            }
            
            break;
        default:
            ++notwritten;
            break;
        }
    }
    
    if(notwritten) {
        if(callback(data + size - notwritten, notwritten, ptr))
            return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    }
    
    return MyCORE_STATUS_OK;
}

/**
 *  Reallocate the buffer
 *  @param  str         the buffer to reallocate
 *  @param  size        new size
 */
mystatus_t myhtml_serialization_reallocate(mycore_string_raw_t *str, size_t size)
{
    // construct a buffer
    char *data = (char*)mycore_realloc(str->data, size * sizeof(char));

    // was it ok?
    if(data == NULL) {
        memset(str, 0, sizeof(mycore_string_raw_t));
        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    }
    else {
        // reallocation succeeded
        str->data = data;
        str->size = size;
    }
    
    return MyCORE_STATUS_OK;
}

/**
 *  Implementation of the mycore_callback_serialize_f function for internal
 *  use that concatenats everything to a string
 *  @param  data
 *  @param  size
 */
mystatus_t myhtml_serialization_concatenate(const char* data, size_t length, void *ptr)
{
    mycore_string_raw_t *str = (mycore_string_raw_t*)ptr;
    
    // do we still have enough size in the output buffer?
    if ((length + str->length) >= str->size) {
        if(myhtml_serialization_reallocate(str, length + str->length + 4096))
            return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    }
    
    // copy data
    strncpy(&str->data[ str->length ], data, length);
    
    // update counters
    str->length += length;
    str->data[ str->length ] = '\0';
    
    return MyCORE_STATUS_OK;
}

/**
 *  Serialize tree to an output string
 *  @param  tree
 *  @param  scope_node
 *  @param  str
 *  @return bool
 */
mystatus_t myhtml_serialization_tree_buffer(myhtml_tree_node_t* scope_node, mycore_string_raw_t* str) {

    // we need an output variable
    if(str == NULL) return false;
    
    // allocate space that is most likely enough for the output
    if(str->data == NULL) {
        str->size   = 4098 * 5;
        str->length = 0;
        str->data   = (char*)mycore_malloc(str->size * sizeof(char));
        
        if(str->data == NULL) {
            str->size = 0;
            return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }
    
    return myhtml_serialization_tree_callback(scope_node, myhtml_serialization_concatenate, str);
}

/**
 *  Serialize node to an output string
 *  @param  tree
 *  @param  node
 *  @param  str
 *  @return bool
 */
mystatus_t myhtml_serialization_node_buffer(myhtml_tree_node_t* node, mycore_string_raw_t* str) {

    // we need an output variable
    if(str == NULL) return false;
    
    // allocate space that is most likely enough for the output
    if(str->data == NULL) {
        str->size   = 2048;
        str->length = 0;
        str->data   = (char*)mycore_malloc(str->size * sizeof(char));
        
        if(str->data == NULL) {
            str->size = 0;
            return false;
        }
    }
    
    return myhtml_serialization_node_callback(node, myhtml_serialization_concatenate, str);
}


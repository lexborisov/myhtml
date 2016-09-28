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

#include <setjmp.h>
#include "myhtml/serialization.h"

/**
 *  Forward declaration of all the functions that are used inside this module
 */
static void process_buffer(const char* str, size_t size, myhtml_callback_serialize_f callback, void *ptr);
static void process_attr(const char* str, size_t length, myhtml_callback_serialize_f callback, void *ptr);
static void process_attributes(myhtml_tree_t* tree, myhtml_tree_attr_t* attr, myhtml_callback_serialize_f callback, void *ptr);
static void process_text(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_callback_serialize_f callback, void *ptr);
static void process_close(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_callback_serialize_f callback, void *ptr);

/**
 *  The serialize function for an entire tree
 *  @param  tree        the tree to be serialized
 *  @param  scope_node  the scope_node 
 *  @param  flags       serialization flags
 *  @param  callback    function that will be called for all strings that have to be printed
 *  @param  ptr         user-supplied pointer
 *  @return bool
 */
bool myhtml_serialize(myhtml_tree_t* tree, myhtml_tree_node_t* scope_node, myhtml_tree_serialize_flags_t flags, myhtml_callback_serialize_f callback, void *ptr)
{
    myhtml_tree_node_t* node = scope_node;
    
    if(node == tree->document) {
        if (!tree->document) return false;
        node = tree->document->child;
    }
    
    while(node) {
        if(!myhtml_serialize_node(tree, node, flags, callback, ptr)) return false;
        
        if(node->child)
            node = node->child;
        else {
            while(node != scope_node && node->next == NULL) {
                process_close(tree, node, callback, ptr);
                node = node->parent;
            }
            
            if(node == scope_node) {
                if(node != tree->document) process_close(tree, node, callback, ptr);
                break;
            }
            
            process_close(tree, node, callback, ptr);
            node = node->next;
        }
    }
    
    return true;
}

/**
 *  The serialize function for a single node
 *  @param  tree        the tree to be serialized
 *  @param  node        the node that is going to be serialized 
 *  @param  flags       serialization flags
 *  @param  callback    function that will be called for all strings that have to be printed
 *  @param  ptr         user-supplied pointer
 *  @return bool
 */
bool myhtml_serialize_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_tree_serialize_flags_t flags, myhtml_callback_serialize_f callback, void *ptr)
{
    switch (node->tag_id) {
        case MyHTML_TAG__TEXT: {
            process_text(tree, node, callback, ptr);
            break;
        }
        case MyHTML_TAG__COMMENT: {
            callback("<!--", 4, ptr);
            if(node->token && node->token->str.data) callback(node->token->str.data, node->token->str.length, ptr);
            callback("-->", 3, ptr);
            break;
        }
        case MyHTML_TAG__DOCTYPE: {
            callback("<!DOCTYPE", 9, ptr);
            
            if(node->token) {
                myhtml_tree_attr_t* attr = node->token->attr_first;
                
                if(attr->key.data && attr->key.length) {
                    callback(" ", 1, ptr);
                    callback(attr->key.data, attr->key.length, ptr);
                }
            }
            callback(">", 1, ptr);
            break;
        }
        default: {
            size_t length;
            const char *tag = myhtml_tag_name_by_id(tree, node->tag_id, &length);

            callback("<", 1, ptr);
            callback(tag, length, ptr);
            if(node->token) process_attributes(tree, node->token->attr_first, callback, ptr);
            callback(">", 1, ptr);
            break;
        }
    }
    
    return true;
}

/**
 *  Internal function to process attributes
 *  @param  tree
 *  @param  attr
 *  @param  callback
 *  @param  ptr
 */
static void process_attributes(myhtml_tree_t* tree, myhtml_tree_attr_t* attr, myhtml_callback_serialize_f callback, void* ptr)
{
    while(attr) {
        callback(" ", 1, ptr);
        
        switch (attr->ns) {
            case MyHTML_NAMESPACE_XML:
                callback("xml:", 4, ptr);
                break;
            case MyHTML_NAMESPACE_XMLNS: {
                /*
                    If the attribute is in the XMLNS namespace and the attribute's local name is not xmlns
                    The attribute's serialized name is the string "xmlns:" followed by the attribute's local name.
                 */
                if(attr->key.data && attr->key.length == 5 && myhtml_strcmp(attr->key.data, "xmlns")) {
                    callback("xmlns:", 6, callback);
                }
                
                break;
            }
            case MyHTML_NAMESPACE_XLINK: {
                callback("xlink:", 6, callback);
                
                break;
            }
            default:
                break;
        }
        
        
        size_t length;
        const char *data = myhtml_attribute_key(attr, &length);
        if(data) callback(data, length, ptr);
        callback("=\"", 2, ptr);
        
        data = myhtml_attribute_value(attr, &length);
        if(data) process_attr(data, length, callback, ptr);
        callback("\"", 1, ptr);
        attr = attr->next;
    }
}

/**
 *  Internal function to process a close tag
 *  @param  tree
 *  @param  node
 *  @param  callback
 *  @param  ptr
 */
static void process_close(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_callback_serialize_f callback, void* ptr)
{
    if(node->tag_id != MyHTML_TAG__TEXT &&
       node->tag_id != MyHTML_TAG__COMMENT &&
       node->tag_id != MyHTML_TAG__DOCTYPE)
    {
        size_t length;
        const char *tag = myhtml_tag_name_by_id(tree, node->tag_id, &length);
        
        callback("</", 2, ptr);
        callback(tag, length, ptr);
        callback(">", 1, ptr);
    }
}

/**
 *  Internal function to process a text node
 *  @param  tree
 *  @param  node
 *  @param  callback
 *  @param  ptr
 */
static void process_text(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_callback_serialize_f callback, void* ptr)
{
    if(node->token == NULL || node->token->str.data == NULL) return;
    
    if(node->parent == NULL) return process_buffer(node->token->str.data, node->token->str.length, callback, ptr);
    
    switch (node->parent->tag_id) {
        case MyHTML_TAG_STYLE:
        case MyHTML_TAG_SCRIPT:
        case MyHTML_TAG_XMP:
        case MyHTML_TAG_IFRAME:
        case MyHTML_TAG_NOEMBED:
        case MyHTML_TAG_NOFRAMES:
        case MyHTML_TAG_PLAINTEXT:
            callback(node->token->str.data, node->token->str.length, ptr);
            break;
        default:
            process_buffer(node->token->str.data, node->token->str.length, callback, ptr);
            break;
    }
}

/**
 *  Internal method to process a buffer that could contain to-be-quoted output
 *  @param  data
 *  @param  size
 *  @param  callback
 *  @param  ptr
 */
static void process_buffer(const char *data, size_t size, myhtml_callback_serialize_f callback, void* ptr)
{
    // number of chars not yet displayed
    size_t notwritten = 0;
    
    // iterate over the buffer
    for (size_t i = 0; i < size; ++i) {
        switch ((unsigned char)data[i]) {
        case '&':
            if (notwritten) callback(data + i - notwritten, notwritten, ptr);
            callback("&amp;", 5, ptr);
            notwritten = 0;
            break;
        case '<':
            if (notwritten) callback(data + i - notwritten, notwritten, ptr);
            callback("&lt;", 4, ptr);
            notwritten = 0;
            break;
        case '>':
            if (notwritten) callback(data + i - notwritten, notwritten, ptr);
            callback("&gt;", 4, ptr);
            notwritten = 0;
            break;
        case 0xA0:
            if (notwritten) callback(data + i - notwritten, notwritten, ptr);
            callback("&nbsp;", 6, ptr);
            notwritten = 0;
            break;
        default:
            ++notwritten;
            break;
        }
    }
    
    if (notwritten) callback(data + size - notwritten, notwritten, ptr);
}

/**
 *  Internal method to process an attribute
 *  @param  data
 *  @param  size
 *  @param  callback
 *  @param  ptr
 */
static void process_attr(const char* data, size_t size, myhtml_callback_serialize_f callback, void* ptr)
{
    // number of chars not yet displayed
    size_t notwritten = 0;
    
    // iterate over the buffer
    for (size_t i = 0; i < size; ++i) {
        switch ((unsigned char)data[i]) {
        case '&':
            if (notwritten) callback(data + i - notwritten, notwritten, ptr);
            callback("&amp;", 5, ptr);
            notwritten = 0;
            break;
        case '"':
            if (notwritten) callback(data + i - notwritten, notwritten, ptr);
            callback("&quot;", 6, ptr);
            notwritten = 0;
            break;
        case 0xA0:
            if (notwritten) callback(data + i - notwritten, notwritten, ptr);
            callback("&nbsp;", 6, NULL);
            notwritten = 0;
            break;
        default:
            ++notwritten;
            break;
        }
    }
    
    if (notwritten) callback(data + size - notwritten, notwritten, ptr);
}

/**
 *  Because we want to leap out of the algorithm if we're halfway through
 *  serializing, we use a longjmp() call to jump back to the public APP
 *  @var jmp_buf
 */
static jmp_buf leap;

/**
 *  Reallocate the buffer
 *  @param  str         the buffer to reallocate
 *  @param  size        new size
 */
static void reallocate(myhtml_string_raw_t *str, size_t size)
{
    // construct a buffer
    char *data = (char*)myhtml_realloc(str->data, size * sizeof(char));

    // was it ok?
    if (data == NULL) {
        
        // allocation failed, reset the string object
        myhtml_free(str->data);
        memset(str, 0, sizeof(myhtml_string_raw_t));
        
        // leap back to the source of the serialization algorithm
        longjmp(leap, 1);
    }
    else {
        
        // reallocation succeeded
        str->data = data;
        str->size = size;
    }
}

/**
 *  Implementation of the myhtml_callback_serialize_f function for internal
 *  use that concatenats everything to a string
 *  @param  data
 *  @param  size
 */
static void concatenate(const char* data, size_t length, void *ptr)
{
    // get the string back
    myhtml_string_raw_t* str = (myhtml_string_raw_t *)ptr;
    
    // do we still have enough size in the output buffer?
    if ((length + str->length) >= str->size) reallocate(str, length + str->length + 4096);
    
    // copy data
    strncpy(&str->data[ str->length ], data, length);
    
    // update counters
    str->length += length;
    str->data[ str->length ] = '\0';
}

/**
 *  Serialize tree to an output string
 *  @param  tree
 *  @param  scope_node
 *  @param  str
 *  @return bool
 */
bool myhtml_serialization(myhtml_tree_t* tree, myhtml_tree_node_t* scope_node, myhtml_string_raw_t* str) {

    // we need an output variable
    if(str == NULL) return false;
    
    // allocate space that is most likely enough for the output
    if(str->data == NULL) {
        str->size   = 4098 * 5;
        str->length = 0;
        str->data   = (char*)myhtml_malloc(str->size * sizeof(char));
        
        if(str->data == NULL) {
            str->size = 0;
            return false;
        }
    }
    
    // if allocation halfway the algorithm fails, we want to leap back
    if (setjmp(leap) == 0)
    {
        // serialize the entire tree
        return myhtml_serialize(tree, scope_node, MyHTML_TREE_SERIALIZE_FLAGS_FULL, concatenate, str);
    }
    else
    {
        // the serialization algorithm failed because of a memory-allocation failure
        return false;
    }
}

/**
 *  Serialize node to an output string
 *  @param  tree
 *  @param  node
 *  @param  str
 *  @return bool
 */
bool myhtml_serialization_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_string_raw_t* str) {

    // we need an output variable
    if(str == NULL) return false;
    
    // allocate space that is most likely enough for the output
    if(str->data == NULL) {
        str->size   = 2048;
        str->length = 0;
        str->data   = (char*)myhtml_malloc(str->size * sizeof(char));
        
        if(str->data == NULL) {
            str->size = 0;
            return false;
        }
    }

    // if allocation halfway the algorithm fails, we want to leap back
    if (setjmp(leap) == 0)
    {
        // pass on
        return myhtml_serialize_node(tree, node, MyHTML_TREE_SERIALIZE_FLAGS_FULL, concatenate, str);
    }
    else
    {
        // the serialization algorithm failed because of a memory-allocation failure
        return false;
    }
}


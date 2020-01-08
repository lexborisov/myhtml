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

#include "myhtml/token.h"

// all key size == value size
static const myhtml_token_replacement_entry_t myhtml_token_attr_svg_replacement[] = {
    {"attributename", 13, "attributeName", 13},
    {"attributetype", 13, "attributeType", 13},
    {"basefrequency", 13, "baseFrequency", 13},
    {"baseprofile", 11, "baseProfile", 11},
    {"calcmode", 8, "calcMode", 8},
    {"clippathunits", 13, "clipPathUnits", 13},
    {"diffuseconstant", 15, "diffuseConstant", 15},
    {"edgemode", 8, "edgeMode", 8},
    {"filterunits", 11, "filterUnits", 11},
    {"glyphref", 8, "glyphRef", 8},
    {"gradienttransform", 17, "gradientTransform", 17},
    {"gradientunits", 13, "gradientUnits", 13},
    {"kernelmatrix", 12, "kernelMatrix", 12},
    {"kernelunitlength", 16, "kernelUnitLength", 16},
    {"keypoints", 9, "keyPoints", 9},
    {"keysplines", 10, "keySplines", 10},
    {"keytimes", 8, "keyTimes", 8},
    {"lengthadjust", 12, "lengthAdjust", 12},
    {"limitingconeangle", 17, "limitingConeAngle", 17},
    {"markerheight", 12, "markerHeight", 12},
    {"markerunits", 11, "markerUnits", 11},
    {"markerwidth", 11, "markerWidth", 11},
    {"maskcontentunits", 16, "maskContentUnits", 16},
    {"maskunits", 9, "maskUnits", 9},
    {"numoctaves", 10, "numOctaves", 10},
    {"pathlength", 10, "pathLength", 10},
    {"patterncontentunits", 19, "patternContentUnits", 19},
    {"patterntransform", 16, "patternTransform", 16},
    {"patternunits", 12, "patternUnits", 12},
    {"pointsatx", 9, "pointsAtX", 9},
    {"pointsaty", 9, "pointsAtY", 9},
    {"pointsatz", 9, "pointsAtZ", 9},
    {"preservealpha", 13, "preserveAlpha", 13},
    {"preserveaspectratio", 19, "preserveAspectRatio", 19},
    {"primitiveunits", 14, "primitiveUnits", 14},
    {"refx", 4, "refX", 4},
    {"refy", 4, "refY", 4},
    {"repeatcount", 11, "repeatCount", 11},
    {"repeatdur", 9, "repeatDur", 9},
    {"requiredextensions", 18, "requiredExtensions", 18},
    {"requiredfeatures", 16, "requiredFeatures", 16},
    {"specularconstant", 16, "specularConstant", 16},
    {"specularexponent", 16, "specularExponent", 16},
    {"spreadmethod", 12, "spreadMethod", 12},
    {"startoffset", 11, "startOffset", 11},
    {"stddeviation", 12, "stdDeviation", 12},
    {"stitchtiles", 11, "stitchTiles", 11},
    {"surfacescale", 12, "surfaceScale", 12},
    {"systemlanguage", 14, "systemLanguage", 14},
    {"tablevalues", 11, "tableValues", 11},
    {"targetx", 7, "targetX", 7},
    {"targety", 7, "targetY", 7},
    {"textlength", 10, "textLength", 10},
    {"viewbox", 7, "viewBox", 7},
    {"viewtarget", 10, "viewTarget", 10},
    {"xchannelselector", 16, "xChannelSelector", 16},
    {"ychannelselector", 16, "yChannelSelector", 16},
    {"zoomandpan", 10, "zoomAndPan", 10}
};

// all key size > value size
static const myhtml_token_namespace_replacement_t myhtml_token_attr_namespace_replacement[] = {
    {"xlink:actuate", 13, "actuate", 7, MyHTML_NAMESPACE_XLINK},
    {"xlink:arcrole", 13, "arcrole", 7, MyHTML_NAMESPACE_XLINK},
    {"xlink:href", 10, "href", 4, MyHTML_NAMESPACE_XLINK},
    {"xlink:role", 10, "role", 4, MyHTML_NAMESPACE_XLINK},
    {"xlink:show", 10, "show", 4, MyHTML_NAMESPACE_XLINK},
    {"xlink:title", 11, "title", 5, MyHTML_NAMESPACE_XLINK},
    {"xlink:type", 10, "type", 4, MyHTML_NAMESPACE_XLINK},
    {"xml:lang", 8, "lang", 4, MyHTML_NAMESPACE_XML},
    {"xml:space", 9, "space", 5, MyHTML_NAMESPACE_XML},
    {"xmlns", 5, "xmlns", 5, MyHTML_NAMESPACE_XMLNS},
    {"xmlns:xlink", 11, "xlink", 5, MyHTML_NAMESPACE_XMLNS}
};

myhtml_token_t * myhtml_token_create(myhtml_tree_t* tree, size_t size)
{
    if(size == 0)
        size = 4096;
    
    myhtml_token_t* token = (myhtml_token_t*)mycore_malloc(sizeof(myhtml_token_t));
    
    if(token == NULL)
        return NULL;
    
    token->nodes_obj = mcobject_async_create();
    
    if(token->nodes_obj == NULL) {
        mycore_free(token);
        return NULL;
    }
    
    token->attr_obj = mcobject_async_create();
    
    if(token->attr_obj == NULL) {
        mycore_free(token->nodes_obj);
        mycore_free(token);
        
        return NULL;
    }
    
    mcobject_async_init(token->nodes_obj, 128, size, sizeof(myhtml_token_node_t));
    mcobject_async_init(token->attr_obj, 128, size, sizeof(myhtml_token_attr_t));
    
    token->mcasync_token_id  = mcobject_async_node_add(token->nodes_obj, NULL);
    token->mcasync_attr_id   = mcobject_async_node_add(token->attr_obj, NULL);
    
    token->tree = tree;
    
    return token;
}

void myhtml_token_clean(myhtml_token_t* token)
{
    mcobject_async_node_clean(token->nodes_obj, token->mcasync_token_id);
    mcobject_async_node_clean(token->attr_obj, token->mcasync_attr_id);
}

void myhtml_token_clean_all(myhtml_token_t* token)
{
    mcobject_async_clean(token->nodes_obj);
    mcobject_async_clean(token->attr_obj);
}

myhtml_token_t * myhtml_token_destroy(myhtml_token_t* token)
{
    if(token == NULL)
        return NULL;
    
    if(token->nodes_obj)
        token->nodes_obj = mcobject_async_destroy(token->nodes_obj, 1);
    
    if(token->attr_obj)
        token->attr_obj = mcobject_async_destroy(token->attr_obj, 1);
    
    mycore_free(token);
    
    return NULL;
}

myhtml_token_node_t * myhtml_token_node_create(myhtml_token_t* token, size_t async_node_id)
{
    myhtml_token_node_t *token_node = (myhtml_token_node_t*)mcobject_async_malloc(token->nodes_obj, async_node_id, NULL);
    if(token_node == NULL)
        return NULL;
    
    myhtml_token_node_clean(token_node);
    return token_node;
}

void myhtml_token_node_clean(myhtml_token_node_t* node)
{
    memset(node, 0, sizeof(myhtml_token_node_t));
    node->type = MyHTML_TOKEN_TYPE_OPEN|MyHTML_TOKEN_TYPE_WHITESPACE;
    
    mycore_string_clean_all(&node->str);
}

myhtml_token_attr_t * myhtml_token_attr_create(myhtml_token_t* token, size_t async_node_id)
{
    myhtml_token_attr_t *attr_node = mcobject_async_malloc(token->attr_obj, async_node_id, NULL);
    if(attr_node == NULL)
        return NULL;
    
    myhtml_token_attr_clean(attr_node);
    return attr_node;
}

void myhtml_token_attr_clean(myhtml_token_attr_t* attr)
{
    memset(attr, 0, sizeof(myhtml_token_attr_t));
    attr->ns = MyHTML_NAMESPACE_HTML;
    
    mycore_string_clean_all(&attr->key);
    mycore_string_clean_all(&attr->value);
}

myhtml_tag_id_t myhtml_token_node_tag_id(myhtml_token_node_t *token_node)
{
    return token_node->tag_id;
}

myhtml_position_t myhtml_token_node_raw_position(myhtml_token_node_t *token_node)
{
    if(token_node)
        return (myhtml_position_t){token_node->raw_begin, token_node->raw_length};
    
    return (myhtml_position_t){0, 0};
}

myhtml_position_t myhtml_token_node_element_position(myhtml_token_node_t *token_node)
{
    if(token_node)
        return (myhtml_position_t){token_node->element_begin, token_node->element_length};
    
    return (myhtml_position_t){0, 0};
}

myhtml_tree_attr_t * myhtml_token_node_attribute_first(myhtml_token_node_t *token_node)
{
    return token_node->attr_first;
}

myhtml_tree_attr_t * myhtml_token_node_attribute_last(myhtml_token_node_t *token_node)
{
    return token_node->attr_last;
}

const char * myhtml_token_node_text(myhtml_token_node_t *token_node, size_t *length)
{
    if(length)
        *length = token_node->str.length;
    
    return token_node->str.data;
}

mycore_string_t * myhtml_token_node_string(myhtml_token_node_t *token_node)
{
    return &token_node->str;
}

bool myhtml_token_node_is_close(myhtml_token_node_t *token_node)
{
    return (token_node->type & MyHTML_TOKEN_TYPE_CLOSE);
}

bool myhtml_token_node_is_close_self(myhtml_token_node_t *token_node)
{
    return (token_node->type & MyHTML_TOKEN_TYPE_CLOSE_SELF);
}

void myhtml_token_node_wait_for_done(myhtml_token_t* token, myhtml_token_node_t* node)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    while((node->type & MyHTML_TOKEN_TYPE_DONE) == 0) {mythread_nanosleep_sleep(token->tree->myhtml->thread_stream->timespec);}
#endif
}

void myhtml_token_set_done(myhtml_token_node_t* node)
{
    node->type |= MyHTML_TOKEN_TYPE_DONE;
}

myhtml_token_node_t * myhtml_token_node_clone(myhtml_token_t* token, myhtml_token_node_t* node, size_t token_thread_idx, size_t attr_thread_idx)
{
    if(node == NULL)
        return NULL;
    
    myhtml_tree_t* tree = token->tree;
    myhtml_token_node_t* new_node = myhtml_token_node_create(token, token_thread_idx);
    
    if(new_node == NULL)
        return NULL;
    
    new_node->tag_id         = node->tag_id;
    new_node->type           = node->type;
    new_node->attr_first     = NULL;
    new_node->attr_last      = NULL;
    new_node->raw_begin      = node->raw_begin;
    new_node->raw_length     = node->raw_length;
    new_node->element_begin  = node->element_begin;
    new_node->element_length = node->element_length;
    
    if(node->str.length) {
        mycore_string_init(tree->mchar, tree->mchar_node_id, &new_node->str, node->str.length + 1);
        mycore_string_append(&new_node->str, node->str.data, node->str.length);
    } else {
        mycore_string_clean_all(&new_node->str);
    }
    
    myhtml_token_node_attr_copy(token, node, new_node, attr_thread_idx);
    
    return new_node;
}

void myhtml_token_node_text_append(myhtml_token_t* token, myhtml_token_node_t* dest, const char* text, size_t text_len)
{
    mycore_string_init(token->tree->mchar, token->tree->mchar_node_id, &dest->str, (text_len + 2));
    
    mycore_string_t* string = &dest->str;
    mycore_string_append(string, text, text_len);
}

myhtml_token_attr_t * myhtml_token_node_attr_append(myhtml_token_t* token, myhtml_token_node_t* dest,
                                   const char* key, size_t key_len,
                                   const char* value, size_t value_len, size_t thread_idx)
{
    myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx, NULL);
    new_attr->next = 0;
    
    if(key_len) {
        mycore_string_init(token->tree->mchar, token->tree->mchar_node_id, &new_attr->key, (key_len + 1));
        mycore_string_append_lowercase(&new_attr->key, key, key_len);
    }
    else
        mycore_string_clean_all(&new_attr->key);
    
    if(value_len) {
        mycore_string_init(token->tree->mchar, token->tree->mchar_node_id, &new_attr->value, (value_len + 1));
        mycore_string_append(&new_attr->value, value, value_len);
    }
    else
        mycore_string_clean_all(&new_attr->value);
    
    if(dest->attr_first == NULL) {
        new_attr->prev = 0;
        
        dest->attr_first = new_attr;
        dest->attr_last = new_attr;
    }
    else {
        dest->attr_last->next = new_attr;
        new_attr->prev = dest->attr_last;
        
        dest->attr_last = new_attr;
    }
    
    new_attr->ns = MyHTML_NAMESPACE_HTML;
    
    return new_attr;
}

myhtml_token_attr_t * myhtml_token_node_attr_append_with_convert_encoding(myhtml_token_t* token, myhtml_token_node_t* dest,
                                                                          const char* key, size_t key_len,
                                                                          const char* value, size_t value_len,
                                                                          size_t thread_idx, myencoding_t encoding)
{
    myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx, NULL);
    new_attr->next = 0;
    
    if(key_len) {
        mycore_string_init(token->tree->mchar, token->tree->mchar_node_id, &new_attr->key, (key_len + 1));
        
        if(encoding == MyENCODING_UTF_8)
            mycore_string_append_lowercase(&new_attr->key, key, key_len);
        else
            myencoding_string_append_lowercase_ascii(&new_attr->key, key, key_len, encoding);
    }
    else
        mycore_string_clean_all(&new_attr->key);
    
    if(value_len) {
        mycore_string_init(token->tree->mchar, token->tree->mchar_node_id, &new_attr->value, (value_len + 1));
        
        if(encoding == MyENCODING_UTF_8)
            mycore_string_append(&new_attr->value, value, value_len);
        else
            myencoding_string_append(&new_attr->value, value, value_len, encoding);
    }
    else
        mycore_string_clean_all(&new_attr->value);
    
    if(dest->attr_first == NULL) {
        new_attr->prev = 0;
        
        dest->attr_first = new_attr;
        dest->attr_last = new_attr;
    }
    else {
        dest->attr_last->next = new_attr;
        new_attr->prev = dest->attr_last;
        
        dest->attr_last = new_attr;
    }
    
    new_attr->ns = MyHTML_NAMESPACE_HTML;
    
    return new_attr;
}

void myhtml_token_node_attr_copy_with_check(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_node_t* dest, size_t thread_idx)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        if(attr->key.length && myhtml_token_attr_by_name(dest, attr->key.data, attr->key.length) == NULL) {
            myhtml_token_attr_copy(token, attr, dest, thread_idx);
        }
        
        attr = attr->next;
    }
}

void myhtml_token_node_attr_copy(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_node_t* dest, size_t thread_idx)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        myhtml_token_attr_copy(token, attr, dest, thread_idx);
        attr = attr->next;
    }
}

bool myhtml_token_attr_copy(myhtml_token_t* token, myhtml_token_attr_t* attr, myhtml_token_node_t* dest, size_t thread_idx)
{
    myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx, NULL);
    new_attr->next = 0;
    
    if(attr->key.length) {
        mycore_string_init(token->tree->mchar, token->tree->mchar_node_id, &new_attr->key, (attr->key.length + 1));
        mycore_string_append_lowercase(&new_attr->key, attr->key.data, attr->key.length);
    }
    else
        mycore_string_clean_all(&new_attr->key);
    
    if(attr->value.length) {
        mycore_string_init(token->tree->mchar, token->tree->mchar_node_id, &new_attr->value, (attr->value.length + 1));
        mycore_string_append(&new_attr->value, attr->value.data, attr->value.length);
    }
    else
        mycore_string_clean_all(&new_attr->value);
    
    if(dest->attr_first == NULL) {
        new_attr->prev = 0;
        
        dest->attr_first = new_attr;
        dest->attr_last = new_attr;
    }
    else {
        dest->attr_last->next = new_attr;
        new_attr->prev = dest->attr_last;
        
        dest->attr_last = new_attr;
    }
    
    new_attr->ns = attr->ns;
    
    return true;
}

myhtml_token_attr_t * myhtml_token_attr_match(myhtml_token_t* token, myhtml_token_node_t* target,
                                              const char* key, size_t key_size, const char* value, size_t value_size)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        if(attr->key.length == key_size && attr->value.length == value_size)
        {
            if((mycore_strcmp(attr->key.data, key) == 0)) {
                if((mycore_strcmp(attr->value.data, value) == 0))
                    return attr;
                else
                    return NULL;
            }
        }
        
        attr = attr->next;
    }
    
    return NULL;
}

myhtml_token_attr_t * myhtml_token_attr_match_case(myhtml_token_t* token, myhtml_token_node_t* target,
                                              const char* key, size_t key_size, const char* value, size_t value_size)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        if(attr->key.length == key_size && attr->value.length == value_size)
        {
            if((mycore_strcmp(attr->key.data, key) == 0)) {
                if((mycore_strcasecmp(attr->value.data, value) == 0))
                    return attr;
                else
                    return NULL;
            }
        }
        
        attr = attr->next;
    }
    
    return NULL;
}

void myhtml_token_adjust_mathml_attributes(myhtml_token_node_t* target)
{
    myhtml_token_attr_t* attr = myhtml_token_attr_by_name(target, "definitionurl", 13);
    
    if(attr) {
        memcpy(attr->key.data, "definitionURL", 13);
    }
}

void _myhtml_token_create_copy_srt(myhtml_token_t* token, const char* from, size_t from_size, char** to)
{
    *to = mchar_async_malloc(token->tree->mchar, token->tree->mchar_node_id, (from_size + 2));
    mycore_string_raw_copy(*to, from, from_size);
}

void myhtml_token_strict_doctype_by_token(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_tree_doctype_t* return_doctype)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    if(attr && attr->key.length) {
        _myhtml_token_create_copy_srt(token, attr->key.data, attr->key.length, &return_doctype->attr_name);
        
        if(mycore_strcmp("html", return_doctype->attr_name))
            return_doctype->is_html = false;
        else
            return_doctype->is_html = true;
    }
    else {
        return_doctype->is_html = false;
        
        _myhtml_token_create_copy_srt(token, "\0", 1, &return_doctype->attr_name);
        
        if(return_doctype->attr_public)
            mycore_free(return_doctype->attr_public);
        return_doctype->attr_public = NULL;
        
        if(return_doctype->attr_system)
            mycore_free(return_doctype->attr_system);
        return_doctype->attr_system = NULL;
        
        return;
    }
    
    attr = attr->next;
    
    if(attr && attr->value.length)
    {
        if(mycore_strcasecmp(attr->value.data, "PUBLIC") == 0)
        {
            // try see public
            attr = attr->next;
            
            if(attr && attr->value.length) {
                _myhtml_token_create_copy_srt(token, attr->value.data, attr->value.length, &return_doctype->attr_public);
                
                // try see system
                attr = attr->next;
                
                if(attr && attr->value.length)
                    _myhtml_token_create_copy_srt(token, attr->value.data, attr->value.length, &return_doctype->attr_system);
                else {
                    if(return_doctype->attr_system)
                        mycore_free(return_doctype->attr_system);
                    
                    _myhtml_token_create_copy_srt(token, "\0", 1, &return_doctype->attr_system);
                }
            }
            else {
                if(return_doctype->attr_public)
                    mycore_free(return_doctype->attr_public);
                return_doctype->attr_public = NULL;
                
                if(return_doctype->attr_system)
                    mycore_free(return_doctype->attr_system);
                return_doctype->attr_system = NULL;
            }
        }
        else if(mycore_strncasecmp(attr->value.data, "SYSTEM", attr->value.length) == 0)
        {
            attr = attr->next;
            
            if(attr && attr->value.length) {
                _myhtml_token_create_copy_srt(token, "\0", 1, &return_doctype->attr_public);
                _myhtml_token_create_copy_srt(token, attr->value.data, attr->value.length, &return_doctype->attr_system);
            }
            else {
                if(return_doctype->attr_public)
                    mycore_free(return_doctype->attr_public);
                return_doctype->attr_public = NULL;
                
                if(return_doctype->attr_system)
                    mycore_free(return_doctype->attr_system);
                return_doctype->attr_system = NULL;
            }
        }
        else {
            if(return_doctype->attr_public)
                mycore_free(return_doctype->attr_public);
            return_doctype->attr_public = NULL;
            
            if(return_doctype->attr_system)
                mycore_free(return_doctype->attr_system);
            return_doctype->attr_system = NULL;
        }
    }
}

bool myhtml_token_doctype_check_html_4_0(myhtml_tree_doctype_t* return_doctype)
{
    return mycore_strcmp(return_doctype->attr_public, "-//W3C//DTD HTML 4.0//EN") &&
    (return_doctype->attr_system == NULL || mycore_strcmp(return_doctype->attr_system, "http://www.w3.org/TR/REC-html40/strict.dtd"));
}

bool myhtml_token_doctype_check_html_4_01(myhtml_tree_doctype_t* return_doctype)
{
    return mycore_strcmp(return_doctype->attr_public, "-//W3C//DTD HTML 4.01//EN") &&
    (return_doctype->attr_system == NULL || mycore_strcmp(return_doctype->attr_system, "http://www.w3.org/TR/html4/strict.dtd"));
}

bool myhtml_token_doctype_check_xhtml_1_0(myhtml_tree_doctype_t* return_doctype)
{
    if(return_doctype->attr_system == NULL)
        return true;
    
    return mycore_strcmp(return_doctype->attr_public, "-//W3C//DTD XHTML 1.0 Strict//EN") &&
    mycore_strcmp(return_doctype->attr_system, "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd");
}

bool myhtml_token_doctype_check_xhtml_1_1(myhtml_tree_doctype_t* return_doctype)
{
    if(return_doctype->attr_system == NULL)
        return true;
    
    return mycore_strcmp(return_doctype->attr_public, "-//W3C//DTD XHTML 1.1//EN") &&
    mycore_strcmp(return_doctype->attr_system, "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd");
}

bool myhtml_token_release_and_check_doctype_attributes(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_tree_doctype_t* return_doctype)
{
    if(return_doctype == NULL)
        return false;
    
    myhtml_token_strict_doctype_by_token(token, target, return_doctype);
    
    if(return_doctype->attr_name == NULL)
        return false;
    
    if((return_doctype->is_html ||
       return_doctype->attr_public ||
       (return_doctype->attr_system && mycore_strcmp(return_doctype->attr_system, "about:legacy-compat"))))
    {
        if(return_doctype->attr_public == NULL)
            return false;
        
        if(return_doctype->is_html &&
           myhtml_token_doctype_check_html_4_0(return_doctype) &&
           myhtml_token_doctype_check_html_4_01(return_doctype) &&
           myhtml_token_doctype_check_xhtml_1_0(return_doctype) &&
           myhtml_token_doctype_check_xhtml_1_1(return_doctype))
        {
            return false;
        }
    }
    
    return true;
}

void myhtml_token_adjust_svg_attributes(myhtml_token_node_t* target)
{
    size_t count = sizeof(myhtml_token_attr_svg_replacement) / sizeof(myhtml_token_replacement_entry_t);
    
    for (size_t i = 0; i < count; i++)
    {
        myhtml_token_attr_t* attr = myhtml_token_attr_by_name(target, myhtml_token_attr_svg_replacement[i].from,
                                                              myhtml_token_attr_svg_replacement[i].from_size);
        
        if(attr) {
            mycore_string_clean(&attr->key);
            mycore_string_append(&attr->key, myhtml_token_attr_svg_replacement[i].to,
                                 myhtml_token_attr_svg_replacement[i].to_size);
        }
    }
}

void myhtml_token_adjust_foreign_attributes(myhtml_token_node_t* target)
{
    size_t count = sizeof(myhtml_token_attr_namespace_replacement) / sizeof(myhtml_token_namespace_replacement_t);
    
    for (size_t i = 0; i < count; i++)
    {
        myhtml_token_attr_t* attr = myhtml_token_attr_by_name(target, myhtml_token_attr_namespace_replacement[i].from,
                                                              myhtml_token_attr_namespace_replacement[i].from_size);
        
        if(attr) {
            mycore_string_clean(&attr->key);
            mycore_string_append(&attr->key, myhtml_token_attr_namespace_replacement[i].to,
                                 myhtml_token_attr_namespace_replacement[i].to_size);
            
            attr->ns = myhtml_token_attr_namespace_replacement[i].ns;
        }
    }
}

bool myhtml_token_attr_compare(myhtml_token_node_t* target, myhtml_token_node_t* dest)
{
    if(target == NULL || dest == NULL)
        return false;
    
    myhtml_token_attr_t* target_attr = target->attr_first;
    myhtml_token_attr_t* dest_attr   = dest->attr_first;
    
    while (target_attr && dest_attr)
    {
        if(target_attr->key.length == dest_attr->key.length &&
           target_attr->value.length == dest_attr->value.length)
        {
            if(mycore_strcmp(target_attr->key.data, dest_attr->key.data) != 0)
                break;
            
            if(mycore_strcasecmp(target_attr->value.data, dest_attr->value.data) != 0)
                break;
        }
        else
            break;
        
        target_attr = target_attr->next;
        dest_attr   = dest_attr->next;
    }
    
    if(target_attr == NULL && dest_attr == NULL)
        return true;
    
    return false;
}

myhtml_token_attr_t * myhtml_token_attr_by_name(myhtml_token_node_t* node, const char* name, size_t name_length)
{
    myhtml_token_attr_t* attr = node->attr_first;
    
    while (attr)
    {
        if(name_length == attr->key.length) {
            if(mycore_strcmp(attr->key.data, name) == 0)
                break;
        }
        
        attr = attr->next;
    }
    
    return attr;
}

void myhtml_token_delete(myhtml_token_t* token, myhtml_token_node_t* node)
{
    if(node->str.data && node->str.mchar) {
        mchar_async_free(node->str.mchar, node->str.node_idx, node->str.data);
    }
    
    mcobject_async_free(token->nodes_obj, node);
}

void myhtml_token_attr_delete_all(myhtml_token_t* token, myhtml_token_node_t* node)
{
    myhtml_token_attr_t* attr = node->attr_first;
    
    while (attr)
    {
        if(attr->key.data && attr->key.mchar) {
            mchar_async_free(attr->key.mchar, attr->key.node_idx, attr->key.data);
        }
        
        if(attr->value.data && attr->value.mchar) {
            mchar_async_free(attr->value.mchar, attr->value.node_idx, attr->value.data);
        }
        
        attr = attr->next;
    }
}

myhtml_token_attr_t * myhtml_token_attr_remove(myhtml_token_node_t* node, myhtml_token_attr_t* attr)
{
    if(attr)
    {
        if(attr->prev) {
            attr->prev->next = attr->next;
        }
        else {
            node->attr_first = attr->next;
        }
        
        if(attr->next) {
            attr->next->prev = attr->prev;
        }
        else {
            node->attr_last = attr->prev;
        }
        
        attr->next = NULL;
        attr->prev = NULL;
    }
    
    return attr;
}

myhtml_token_attr_t * myhtml_token_attr_remove_by_name(myhtml_token_node_t* node, const char* name, size_t name_length)
{
    return myhtml_token_attr_remove(node, myhtml_token_attr_by_name(node, name, name_length));
}

myhtml_token_node_t * myhtml_token_merged_two_token_string(myhtml_tree_t* tree, myhtml_token_node_t* token_to, myhtml_token_node_t* token_from, bool cp_reverse)
{
    myhtml_token_node_wait_for_done(tree->token, token_to);
    myhtml_token_node_wait_for_done(tree->token, token_from);
    
    mycore_string_t *string1 = &token_to->str;
    mycore_string_t *string2 = &token_from->str;
    
    token_to->raw_begin  = 0;
    token_to->raw_length = 0;
    
    if(token_to->str.node_idx == tree->mchar_node_id)
    {
        if(cp_reverse) {
            //mycore_string_copy(string2, &string_base);
        }
        else {
            mycore_string_copy(string1, string2);
        }
        
        return token_to;
    }
    if(token_from->str.node_idx == tree->mchar_node_id)
    {
        if(cp_reverse) {
            mycore_string_copy(string2, string1);
        }
        else {
            mycore_string_copy(string1, string2);
        }
        
        return token_from;
    }
    else {
        mycore_string_t string_base;
        mycore_string_init(tree->mchar, tree->mchar_node_id, &string_base, (string1->length + string2->length + 2));
        
        if(cp_reverse) {
            mycore_string_copy(&string_base, string2);
            mycore_string_copy(&string_base, string1);
        }
        else {
            mycore_string_copy(&string_base, string1);
            mycore_string_copy(&string_base, string2);
        }
        
        token_to->str = string_base;
    }
    
    return token_to;
}

void myhtml_token_set_replacement_character_for_null_token(myhtml_tree_t* tree, myhtml_token_node_t* node)
{
    myhtml_token_node_wait_for_done(tree->token, node);
    
    mycore_string_t new_str;
    mycore_string_init(tree->mchar, tree->mchar_node_id, &new_str, (node->str.length + 2));
    
    mycore_string_append_with_replacement_null_characters(&new_str, node->str.data, node->str.length);
    
    node->str = new_str;
}


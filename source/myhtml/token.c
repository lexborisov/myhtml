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
    
    myhtml_token_t* token = (myhtml_token_t*)mymalloc(sizeof(myhtml_token_t));
    
    token->nodes_obj = mcobject_async_create();
    token->attr_obj  = mcobject_async_create();
    
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
    
    free(token);
    
    return NULL;
}

void myhtml_token_node_clean(myhtml_token_node_t* node)
{
    node->tag_ctx_idx    = MyHTML_TAG__UNDEF;
    node->type           = MyHTML_TOKEN_TYPE_OPEN|MyHTML_TOKEN_TYPE_WHITESPACE;
    node->attr_first     = NULL;
    node->attr_last      = NULL;
    node->begin          = 0;
    node->length         = 0;
    
    myhtml_string_clean_all(&node->my_str_tm);
}

void myhtml_token_attr_clean(myhtml_token_attr_t* attr)
{
    attr->next         = 0;
    attr->prev         = 0;
    attr->name_begin   = 0;
    attr->name_length  = 0;
    attr->value_begin  = 0;
    attr->value_length = 0;
    attr->my_namespace = MyHTML_NAMESPACE_UNDEF;
    
    myhtml_string_clean_all(&attr->entry);
}

void myhtml_token_node_wait_for_done(myhtml_token_node_t* node)
{
#ifndef MyHTML_BUILD_WITHOUT_THREADS
    
    const struct timespec timeout = {0, 10000};
    while((node->type & MyHTML_TOKEN_TYPE_DONE) == 0) {myhtml_thread_nanosleep(&timeout);}
    
#endif
}

myhtml_token_node_t * myhtml_token_node_clone(myhtml_token_t* token, myhtml_token_node_t* node, size_t token_thread_idx, size_t attr_thread_idx)
{
    if(node == NULL)
        return NULL;
    
    myhtml_tree_t* tree = token->tree;
    
    myhtml_token_node_t* new_node = mcobject_async_malloc(token->nodes_obj, token_thread_idx, NULL);
    
    new_node->tag_ctx_idx = node->tag_ctx_idx;
    new_node->type        = node->type;
    new_node->attr_first  = NULL;
    new_node->attr_last   = NULL;
    new_node->begin       = node->begin;
    new_node->length      = node->length;
    
    myhtml_string_init(tree->mchar, tree->mchar_node_id, &new_node->my_str_tm, node->my_str_tm.size);
    myhtml_token_node_attr_copy(token, node, new_node, attr_thread_idx);
    
    return new_node;
}

void myhtml_token_node_text_append(myhtml_token_t* token, myhtml_token_node_t* dest, const char* text, size_t text_len)
{
    myhtml_string_init(token->tree->mchar, token->tree->mchar_node_id, &dest->my_str_tm, (text_len + 32));
    
    myhtml_string_t* string = &dest->my_str_tm;
    
    dest->begin  = string->length;
    dest->length = text_len;
    
    myhtml_string_append(string, text, text_len);
}

myhtml_token_attr_t * myhtml_token_node_attr_append(myhtml_token_t* token, myhtml_token_node_t* dest,
                                   const char* key, size_t key_len,
                                   const char* value, size_t value_len, size_t thread_idx)
{
    myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx, NULL);
    new_attr->next = 0;
    
    myhtml_string_init(token->tree->mchar, token->tree->mchar_node_id, &new_attr->entry, (key_len + value_len + 16));
    
    if(key_len)
    {
        new_attr->name_begin = new_attr->entry.length;
        new_attr->name_length = key_len;
        
        myhtml_string_append_lowercase(&new_attr->entry,
                                       key, key_len);
    }
    else {
        new_attr->name_begin  = 0;
        new_attr->name_length = 0;
    }
    
    if(value_len)
    {
        new_attr->value_begin = new_attr->entry.length;
        new_attr->value_length = value_len;
        
        myhtml_string_append(&new_attr->entry,
                             value, value_len);
    }
    else {
        new_attr->value_begin  = 0;
        new_attr->value_length = 0;
    }
    
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
    
    return new_attr;
}

myhtml_token_attr_t * myhtml_token_node_attr_append_with_convert_encoding(myhtml_token_t* token, myhtml_token_node_t* dest,
                                                                          const char* key, size_t key_len,
                                                                          const char* value, size_t value_len,
                                                                          size_t thread_idx, myhtml_encoding_t encoding)
{
    myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx, NULL);
    new_attr->next = 0;
    
    myhtml_string_init(token->tree->mchar, token->tree->mchar_node_id, &new_attr->entry, (key_len + value_len + 16));
    
    if(key_len)
    {
        new_attr->name_begin = new_attr->entry.length;
        new_attr->name_length = key_len;
        
        if(encoding == MyHTML_ENCODING_UTF_8)
            myhtml_string_append_lowercase(&new_attr->entry, key, key_len);
        else
            myhtml_string_append_lowercase_ascii_with_convert_encoding(&new_attr->entry,
                                                                       key, key_len, encoding);
    }
    else {
        new_attr->name_begin  = 0;
        new_attr->name_length = 0;
    }
    
    if(value_len)
    {
        new_attr->value_begin = new_attr->entry.length;
        new_attr->value_length = value_len;
        
        if(encoding == MyHTML_ENCODING_UTF_8)
            myhtml_string_append(&new_attr->entry, value, value_len);
        else
            myhtml_string_append_with_convert_encoding(&new_attr->entry,
                                                       value, value_len, encoding);
    }
    else {
        new_attr->value_begin  = 0;
        new_attr->value_length = 0;
    }
    
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
    
    new_attr->my_namespace = MyHTML_NAMESPACE_HTML;
    
    return new_attr;
}

// TODO: use tree for this
void myhtml_token_node_attr_copy_with_check(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_node_t* dest, size_t thread_idx)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        if(attr->name_length && myhtml_token_attr_by_name(dest, &attr->entry.data[ attr->name_begin ], attr->name_length) == NULL) {
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
    
    myhtml_string_init(token->tree->mchar, token->tree->mchar_node_id, &new_attr->entry, (attr->name_length + attr->value_begin + 16));
    
    if(attr->name_length)
    {
        new_attr->name_begin  = new_attr->entry.length;
        new_attr->name_length = attr->name_length;
        
        myhtml_string_append_lowercase(&new_attr->entry,
                                       &attr->entry.data[attr->name_begin],
                                       attr->name_length);
    }
    else {
        new_attr->name_begin  = 0;
        new_attr->name_length = 0;
    }
    
    if(attr->value_length)
    {
        new_attr->value_begin  = new_attr->entry.length;
        new_attr->value_length = attr->value_length;
        
        myhtml_string_append(&new_attr->entry,
                             &attr->entry.data[attr->value_begin],
                             attr->value_length);
    }
    else {
        new_attr->value_begin  = 0;
        new_attr->value_length = 0;
    }
    
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
    
    return true;
}

myhtml_token_attr_t * myhtml_token_attr_match(myhtml_token_t* token, myhtml_token_node_t* target,
                                              const char* key, size_t key_size, const char* value, size_t value_size)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        if(attr->name_length == key_size && attr->value_length == value_size)
        {
            if((strncmp(key, &attr->entry.data[attr->name_begin], key_size) == 0)) {
               if((strncmp(value, &attr->entry.data[attr->value_begin], value_size) == 0)) {
                   return attr;
               }
               else {
                   return NULL;
               }
            }
        }
        
        attr = attr->next;
    }
    
    return NULL;
}

// TODO: copy/past but...
myhtml_token_attr_t * myhtml_token_attr_match_case(myhtml_token_t* token, myhtml_token_node_t* target,
                                              const char* key, size_t key_size, const char* value, size_t value_size)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        if(attr->name_length == key_size && attr->value_length == value_size)
        {
            if((strncmp(key, &attr->entry.data[attr->name_begin], key_size) == 0)) {
                if((myhtml_strncasecmp(value, &attr->entry.data[attr->value_begin], value_size) == 0)) {
                    return attr;
                }
                else {
                    return NULL;
                }
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
        memcpy(&attr->entry.data[attr->name_begin], "definitionURL", 13);
    }
}

// TODO: init mchar_async
void _myhtml_token_create_copy_srt(myhtml_token_t* token, const char* from, size_t from_size, char** to)
{
    *to = mchar_async_malloc(token->tree->mchar, token->tree->mchar_node_id, (from_size + 2));
    myhtml_string_raw_copy(*to, from, from_size);
}

void myhtml_token_strict_doctype_by_token(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_tree_doctype_t* return_doctype)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    const char* data;
    
    if(attr && attr->name_length) {
        data = attr->entry.data;
        
        _myhtml_token_create_copy_srt(token, &data[attr->name_begin], attr->name_length, &return_doctype->attr_name);
        
        if(strcmp("html", return_doctype->attr_name))
            return_doctype->is_html = false;
        else
            return_doctype->is_html = true;
    }
    else {
        return_doctype->is_html = false;
        
        //        if(return_doctype->attr_name)
        //            myfree(return_doctype->attr_name);
        //        return_doctype->attr_name = NULL;
        
        _myhtml_token_create_copy_srt(token, "\0", 1, &return_doctype->attr_name);
        
        if(return_doctype->attr_public)
            myfree(return_doctype->attr_public);
        return_doctype->attr_public = NULL;
        
        if(return_doctype->attr_system)
            myfree(return_doctype->attr_system);
        return_doctype->attr_system = NULL;
        
        return;
    }
    
    attr = attr->next;
    
    if(attr && attr->value_length)
    {
        data = attr->entry.data;
        
        if(myhtml_strncasecmp(&data[attr->value_begin], "PUBLIC", attr->value_length) == 0)
        {
            // try see public
            attr = attr->next;
            
            if(attr && attr->value_length) {
                data = attr->entry.data;
                
                _myhtml_token_create_copy_srt(token, &data[attr->value_begin], attr->value_length, &return_doctype->attr_public);
                
                // try see system
                attr = attr->next;
                
                if(attr && attr->value_length) {
                    data = attr->entry.data;
                    _myhtml_token_create_copy_srt(token, &data[attr->value_begin], attr->value_length, &return_doctype->attr_system);
                }
                else {
                    if(return_doctype->attr_system)
                        myfree(return_doctype->attr_system);
                    
                    _myhtml_token_create_copy_srt(token, "\0", 1, &return_doctype->attr_system);
                    //return_doctype->attr_system = NULL;
                }
            }
            else {
                if(return_doctype->attr_public)
                    myfree(return_doctype->attr_public);
                return_doctype->attr_public = NULL;
                
                if(return_doctype->attr_system)
                    myfree(return_doctype->attr_system);
                return_doctype->attr_system = NULL;
            }
        }
        else if(myhtml_strncasecmp(&data[attr->value_begin], "SYSTEM", attr->value_length) == 0)
        {
            attr = attr->next;
            
            if(attr && attr->value_length) {
                data = attr->entry.data;
                _myhtml_token_create_copy_srt(token, "\0", 1, &return_doctype->attr_public);
                _myhtml_token_create_copy_srt(token, &data[attr->value_begin], attr->value_length, &return_doctype->attr_system);
            }
            else {
                if(return_doctype->attr_public)
                    myfree(return_doctype->attr_public);
                return_doctype->attr_public = NULL;
                
                if(return_doctype->attr_system)
                    myfree(return_doctype->attr_system);
                return_doctype->attr_system = NULL;
            }
        }
        else {
            if(return_doctype->attr_public)
                myfree(return_doctype->attr_public);
            return_doctype->attr_public = NULL;
            
            if(return_doctype->attr_system)
                myfree(return_doctype->attr_system);
            return_doctype->attr_system = NULL;
        }
    }
}

bool myhtml_token_doctype_check_html_4_0(myhtml_tree_doctype_t* return_doctype)
{
    return strcmp(return_doctype->attr_public, "-//W3C//DTD HTML 4.0//EN") &&
    (return_doctype->attr_system == NULL || strcmp(return_doctype->attr_system, "http://www.w3.org/TR/REC-html40/strict.dtd"));
}

bool myhtml_token_doctype_check_html_4_01(myhtml_tree_doctype_t* return_doctype)
{
    return strcmp(return_doctype->attr_public, "-//W3C//DTD HTML 4.01//EN") &&
    (return_doctype->attr_system == NULL || strcmp(return_doctype->attr_system, "http://www.w3.org/TR/html4/strict.dtd"));
}

bool myhtml_token_doctype_check_xhtml_1_0(myhtml_tree_doctype_t* return_doctype)
{
    if(return_doctype->attr_system == NULL)
        return true;
    
    return strcmp(return_doctype->attr_public, "-//W3C//DTD XHTML 1.0 Strict//EN") &&
    strcmp(return_doctype->attr_system, "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd");
}

bool myhtml_token_doctype_check_xhtml_1_1(myhtml_tree_doctype_t* return_doctype)
{
    if(return_doctype->attr_system == NULL)
        return true;
    
    return strcmp(return_doctype->attr_public, "-//W3C//DTD XHTML 1.1//EN") &&
    strcmp(return_doctype->attr_system, "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd");
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
       (return_doctype->attr_system && strcmp(return_doctype->attr_system, "about:legacy-compat"))))
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

// oh
// TODO: see this code
void myhtml_token_adjust_svg_attributes(myhtml_token_node_t* target)
{
    size_t count = sizeof(myhtml_token_attr_svg_replacement) / sizeof(myhtml_token_replacement_entry_t);
    
    for (size_t i = 0; i < count; i++)
    {
        myhtml_token_attr_t* attr = myhtml_token_attr_by_name(target, myhtml_token_attr_svg_replacement[i].from,
                                                              myhtml_token_attr_svg_replacement[i].from_size);
        
        if(attr) {
            memcpy(&attr->entry.data[attr->name_begin],
                   myhtml_token_attr_svg_replacement[i].to,
                   myhtml_token_attr_svg_replacement[i].from_size);
            // from_size == to_size, but use from_size for copy
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
            memcpy(&attr->entry.data[attr->name_begin],
                   myhtml_token_attr_namespace_replacement[i].to,
                   myhtml_token_attr_namespace_replacement[i].to_size);
            
            attr->name_length = myhtml_token_attr_namespace_replacement[i].to_size;
            attr->entry.data[attr->name_length] = '\0';
            
            attr->my_namespace = myhtml_token_attr_namespace_replacement[i].my_namespace;
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
        if(target_attr->name_length == dest_attr->name_length &&
           target_attr->value_length == dest_attr->value_length)
        {
            if(strcmp(&target_attr->entry.data[target_attr->name_begin], &dest_attr->entry.data[dest_attr->name_begin]) != 0)
                break;
            
            if(myhtml_strcasecmp(&target_attr->entry.data[target_attr->value_begin], &dest_attr->entry.data[dest_attr->value_begin]) != 0)
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
        if(name_length == attr->name_length) {
            if(strncmp(name, &attr->entry.data[attr->name_begin], name_length) == 0)
                break;
        }
        
        attr = attr->next;
    }
    
    return attr;
}

void myhtml_token_delete(myhtml_token_t* token, myhtml_token_node_t* node)
{
    if(node->my_str_tm.data && node->my_str_tm.mchar) {
        mchar_async_free(node->my_str_tm.mchar, node->my_str_tm.node_idx, node->my_str_tm.data);
    }
    
    mcobject_async_free(token->nodes_obj, node);
}

void myhtml_token_attr_delete_all(myhtml_token_t* token, myhtml_token_node_t* node)
{
    myhtml_token_attr_t* attr = node->attr_first;
    
    while (attr)
    {
        if(attr->entry.data && attr->entry.mchar) {
            mchar_async_free(attr->entry.mchar, attr->entry.node_idx, attr->entry.data);
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

bool myhtml_token_is_whithspace(myhtml_tree_t* tree, myhtml_token_node_t* node)
{
    const char* html = node->my_str_tm.data;
    
    size_t i;
    for (i = node->begin; i < node->length; i++)
    {
        // TODO: see and use myhtml_whithspace
        // U+0009 // U+000A // U+000C // U+000D // U+0020
        if(html[i] != '\t' && html[i] != '\n' && html[i] != '\f' && html[i] != '\r' && html[i] != ' ') {
            return false;
        }
    }
    
    return true;
}

myhtml_token_node_t * myhtml_token_merged_two_token_string(myhtml_tree_t* tree, myhtml_token_node_t* token_to, myhtml_token_node_t* token_from, bool cp_reverse)
{
    myhtml_token_node_wait_for_done(token_to);
    myhtml_token_node_wait_for_done(token_from);
    
    myhtml_string_t *string1 = &token_to->my_str_tm;
    myhtml_string_t *string2 = &token_from->my_str_tm;
    
    if(token_to->my_str_tm.node_idx == tree->mchar_node_id)
    {
        token_to->length = token_to->length + token_from->length;
        
        if(cp_reverse) {
            //myhtml_string_copy(string2, &string_base);
        }
        else {
            myhtml_string_copy(string1, string2);
        }
        
        return token_to;
    }
    if(token_from->my_str_tm.node_idx == tree->mchar_node_id)
    {
        token_from->length = token_to->length + token_from->length;
        
        if(cp_reverse) {
            myhtml_string_copy(string2, string1);
        }
        else {
            myhtml_string_copy(string1, string2);
        }
        
        return token_from;
    }
    else {
        myhtml_string_t string_base;
        myhtml_string_init(tree->mchar, tree->mchar_node_id, &string_base, (token_to->length + 2));
        
        token_to->length = token_to->length + token_from->length;
        
        if(cp_reverse) {
            myhtml_string_copy(&string_base, string2);
            myhtml_string_copy(&string_base, string1);
        }
        else {
            myhtml_string_copy(&string_base, string1);
            myhtml_string_copy(&string_base, string2);
        }
        
        token_to->my_str_tm = string_base;
    }
    
    return token_to;
}

void myhtml_token_set_replacement_character_for_null_token(myhtml_tree_t* tree, myhtml_token_node_t* node)
{
    myhtml_token_node_wait_for_done(node);
    
    myhtml_string_t new_str;
    myhtml_string_init(tree->mchar, tree->mchar_node_id, &new_str, (node->my_str_tm.length + 32));
    
    myhtml_string_append_with_replacement_null_characters_only(&new_str, node->my_str_tm.data, node->my_str_tm.length);
    
    node->length = new_str.length;
    
    // TODO: send to cache prev str
    node->my_str_tm = new_str;
}

void myhtml_token_set_done(myhtml_token_node_t* node)
{
    node->type |= MyHTML_TOKEN_TYPE_DONE;
}

void myhtml_token_print_param_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_node_t* node, FILE* out)
{
    if(node->type & MyHTML_TOKEN_TYPE_CLOSE) {
        fprintf(out, "</");
    }
    else {
        fprintf(out, "<");
    }
    
    fprintf(out, "tag_ctx_idx=%zu; body_begin=%zu; body_length=%zu; attr_first=%zu; attr_last=%zu",
            node->tag_ctx_idx, node->begin, node->length,
            (size_t)node->attr_first, (size_t)node->attr_last);
    
    if(node->type & MyHTML_TOKEN_TYPE_CLOSE_SELF) {
        fprintf(out, " />\n");
    }
    else {
        fprintf(out, ">\n");
    }
}

void myhtml_token_print_by_idx(myhtml_tree_t* tree, myhtml_token_node_t* node, FILE* out)
{
    const myhtml_tag_context_t *ctx = myhtml_tag_get_by_id(tree->tags, node->tag_ctx_idx);
    
    if(node->tag_ctx_idx == MyHTML_TAG__TEXT ||
       node->tag_ctx_idx == MyHTML_TAG__COMMENT)
    {
        if(node->length) {
            fprintf(out, "%.*s: %.*s\n", (int)ctx->name_length, ctx->name,
                    (int)node->length, &node->my_str_tm.data[node->begin]);
        }
        else {
            fprintf(out, "%.*s is empty\n", (int)ctx->name_length, ctx->name);
        }
    }
    else
    {
        if(node->type & MyHTML_TOKEN_TYPE_CLOSE) {
            fprintf(out, "</");
        }
        else {
            fprintf(out, "<");
        }
        
        fprintf(out, "%.*s tagid=\"%zu\"", (int)ctx->name_length, ctx->name, node->tag_ctx_idx);
        
        myhtml_token_print_attr(tree, node, out);
        
        if(node->type & MyHTML_TOKEN_TYPE_CLOSE_SELF) {
            fprintf(out, " />\n");
        }
        else {
            fprintf(out, ">\n");
        }
    }
}

void myhtml_token_print_attr(myhtml_tree_t* tree, myhtml_token_node_t* node, FILE* out)
{
    myhtml_token_attr_t* attr = node->attr_first;
    
    while(attr)
    {
        fprintf(out, " %.*s", (int)attr->name_length, &attr->entry.data[attr->name_begin]);
        
        if(attr->my_namespace != MyHTML_NAMESPACE_HTML)
        {
            switch (attr->my_namespace) {
                case MyHTML_NAMESPACE_SVG:
                    fprintf(out, ":svg");
                    break;
                case MyHTML_NAMESPACE_MATHML:
                    fprintf(out, ":math");
                    break;
                case MyHTML_NAMESPACE_XLINK:
                    fprintf(out, ":xlink");
                    break;
                case MyHTML_NAMESPACE_XML:
                    fprintf(out, ":xml");
                    break;
                case MyHTML_NAMESPACE_XMLNS:
                    fprintf(out, ":xmlns");
                    break;
                default:
                    fprintf(out, ":UNDEF");
                    break;
            }
        }
        
        if(attr->value_begin) {
            fprintf(out, "=\"%.*s\"",(int)attr->value_length, &attr->entry.data[attr->value_begin]);
        }
        
        attr = attr->next;
    }
}



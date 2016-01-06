/*
 Copyright 2015 Alexander Borisov
 
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

#include "token.h"

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
    
    myhtml_string_clean(&node->my_str_tm);
}

void myhtml_token_attr_clean(myhtml_token_attr_t* attr)
{
    attr->next         = 0;
    attr->prev         = 0;
    attr->name_begin   = 0;
    attr->name_length  = 0;
    attr->value_begin  = 0;
    attr->value_length = 0;
    attr->namespace    = MyHTML_NAMESPACE_UNDEF;
    
    myhtml_string_clean(&attr->entry);
}

void myhtml_token_node_wait_for_done(myhtml_token_node_t* node)
{
    while((node->type & MyHTML_TOKEN_TYPE_DONE) == 0) {}
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
    
    myhtml_string_init(&new_node->my_str_tm,  tree->mchar, tree->mchar_node_id, node->my_str_tm.size);
    myhtml_token_node_attr_copy(token, node, new_node, attr_thread_idx);
    
    return new_node;
}

void myhtml_token_node_text_append(myhtml_token_t* token, myhtml_token_node_t* dest, const char* text, size_t text_len)
{
    myhtml_string_init(&dest->my_str_tm, token->tree->mchar,
                       token->tree->mchar_node_id, (text_len + 32));
    
    myhtml_string_t* string = &dest->my_str_tm;
    
    dest->begin  = string->length;
    dest->length = text_len;
    
    myhtml_string_append_with_null(string, text, text_len);
}

myhtml_token_attr_t * myhtml_token_node_attr_append(myhtml_token_t* token, myhtml_token_node_t* dest,
                                   const char* key, size_t key_len,
                                   const char* value, size_t value_len, size_t thread_idx)
{
    myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx, NULL);
    new_attr->next = 0;
    
    myhtml_string_init(&new_attr->entry, token->tree->mchar,
                       token->tree->mchar_node_id, (key_len + value_len + 16));
    
    if(key_len)
    {
        new_attr->name_begin = new_attr->entry.length;
        new_attr->name_length = key_len;
        
        myhtml_string_append_lowercase_with_null(&new_attr->entry,
                                                 key,
                                                 key_len);
    }
    else {
        new_attr->name_begin  = 0;
        new_attr->name_length = 0;
    }
    
    if(value_len)
    {
        new_attr->value_begin = new_attr->entry.length;
        new_attr->value_length = value_len;
        
        myhtml_string_append_lowercase_with_null(&new_attr->entry,
                                                 value,
                                                 value_len);
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

// TODO: use tree for this
void myhtml_token_node_attr_copy(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_node_t* dest, size_t thread_idx)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        myhtml_token_attr_copy(token, attr, dest, thread_idx);
        attr = attr->next;
    }
}

mybool_t myhtml_token_attr_copy(myhtml_token_t* token, myhtml_token_attr_t* attr, myhtml_token_node_t* dest, size_t thread_idx)
{
    myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx, NULL);
    new_attr->next = 0;
    
    myhtml_string_init(&new_attr->entry,  token->tree->mchar, token->tree->mchar_node_id,
                       (attr->name_length + attr->value_begin + 16));
    
    if(attr->name_length)
    {
        new_attr->name_begin  = new_attr->entry.length;
        new_attr->name_length = attr->name_length;
        
        myhtml_string_append_lowercase_with_null(&new_attr->entry,
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
        
        myhtml_string_append_lowercase_with_null(&new_attr->entry,
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
    
    return mytrue;
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
                if((strncasecmp(value, &attr->entry.data[attr->value_begin], value_size) == 0)) {
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
    strcpy(*to, from);
}

void myhtml_token_strict_doctype_by_token(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_tree_doctype_t* return_doctype)
{
    myhtml_token_attr_t* attr = target->attr_first;
    
    const char* data;
    
    if(attr && attr->name_length) {
        data = attr->entry.data;
        
        _myhtml_token_create_copy_srt(token, &data[attr->name_begin], attr->name_length, &return_doctype->name);
        
        if(strcmp("html", return_doctype->name))
             return_doctype->is_html = myfalse;
        else
             return_doctype->is_html = mytrue;
    }
    else {
        return_doctype->is_html = myfalse;
        
        if(return_doctype->name)
            myfree(return_doctype->name);
        return_doctype->name = NULL;
        
        if(return_doctype->public)
            myfree(return_doctype->public);
        return_doctype->public = NULL;
        
        if(return_doctype->system)
            myfree(return_doctype->system);
        return_doctype->system = NULL;
        
        return;
    }
    
    attr = attr->next;
    
    if(attr && attr->name_length)
    {
        data = attr->entry.data;
        
        if(strcmp("PUBLIC", &data[attr->name_begin]) == 0)
        {
            // try see public
            attr = attr->next;
            
            if(attr && attr->name_length) {
                data = attr->entry.data;
                
                _myhtml_token_create_copy_srt(token, &data[attr->name_begin], attr->name_length, &return_doctype->public);
                
                // try see system
                attr = attr->next;
                
                if(attr && attr->name_length) {
                    data = attr->entry.data;
                    _myhtml_token_create_copy_srt(token, &data[attr->name_begin], attr->name_length, &return_doctype->system);
                }
                else {
                    if(return_doctype->system)
                        myfree(return_doctype->system);
                    return_doctype->system = NULL;
                }
            }
            else {
                if(return_doctype->public)
                    myfree(return_doctype->public);
                return_doctype->public = NULL;
                
                if(return_doctype->system)
                    myfree(return_doctype->system);
                return_doctype->system = NULL;
            }
        }
        else if(strcmp("SYSTEM", &data[attr->name_begin]) == 0)
        {
            attr = attr->next;
            
            if(attr && attr->name_length) {
                data = attr->entry.data;
                _myhtml_token_create_copy_srt(token, &data[attr->name_begin], attr->name_length, &return_doctype->system);
            }
            else {
                if(return_doctype->public)
                    myfree(return_doctype->public);
                return_doctype->public = NULL;
                
                if(return_doctype->system)
                    myfree(return_doctype->system);
                return_doctype->system = NULL;
            }
        }
        else {
            if(return_doctype->public)
                myfree(return_doctype->public);
            return_doctype->public = NULL;
            
            if(return_doctype->system)
                myfree(return_doctype->system);
            return_doctype->system = NULL;
        }
    }
}

mybool_t myhtml_token_doctype_check_html_4_0(myhtml_tree_doctype_t* return_doctype)
{
    return strcmp(return_doctype->public, "-//W3C//DTD HTML 4.0//EN") &&
    (return_doctype->system == NULL || strcmp(return_doctype->system, "http://www.w3.org/TR/REC-html40/strict.dtd"));
}

mybool_t myhtml_token_doctype_check_html_4_01(myhtml_tree_doctype_t* return_doctype)
{
    return strcmp(return_doctype->public, "-//W3C//DTD HTML 4.01//EN") &&
    (return_doctype->system == NULL || strcmp(return_doctype->system, "http://www.w3.org/TR/html4/strict.dtd"));
}

mybool_t myhtml_token_doctype_check_xhtml_1_0(myhtml_tree_doctype_t* return_doctype)
{
    if(return_doctype->system == NULL)
        return mytrue;
    
    return strcmp(return_doctype->public, "-//W3C//DTD XHTML 1.0 Strict//EN") &&
    strcmp(return_doctype->system, "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd");
}

mybool_t myhtml_token_doctype_check_xhtml_1_1(myhtml_tree_doctype_t* return_doctype)
{
    if(return_doctype->system == NULL)
        return mytrue;
    
    return strcmp(return_doctype->public, "-//W3C//DTD XHTML 1.1//EN") &&
    strcmp(return_doctype->system, "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd");
}

mybool_t myhtml_token_release_and_check_doctype_attributes(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_tree_doctype_t* return_doctype)
{
    if(return_doctype == NULL)
        return myfalse;
    
    myhtml_token_strict_doctype_by_token(token, target, return_doctype);
    
    if(return_doctype->name == NULL)
        return myfalse;
    
    if((return_doctype->is_html ||
       return_doctype->public ||
       (return_doctype->system && strcmp(return_doctype->system, "about:legacy-compat"))))
    {
        if(return_doctype->public == NULL)
            return myfalse;
        
        if(return_doctype->is_html &&
           myhtml_token_doctype_check_html_4_0(return_doctype) &&
           myhtml_token_doctype_check_html_4_01(return_doctype) &&
           myhtml_token_doctype_check_xhtml_1_0(return_doctype) &&
           myhtml_token_doctype_check_xhtml_1_1(return_doctype))
        {
            return myfalse;
        }
    }
    
    return mytrue;
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
            
            attr->namespace = myhtml_token_attr_namespace_replacement[i].namespace;
        }
    }
}

mybool_t myhtml_token_attr_compare(myhtml_token_node_t* target, myhtml_token_node_t* dest)
{
    if(target == NULL || dest == NULL)
        return myfalse;
    
    myhtml_token_attr_t* target_attr = target->attr_first;
    myhtml_token_attr_t* dest_attr   = dest->attr_first;
    
    while (target_attr && dest_attr)
    {
        if(target_attr->name_length == dest_attr->name_length &&
           target_attr->value_length == dest_attr->value_length)
        {
            if(strcmp(&target_attr->entry.data[target_attr->name_begin], &dest_attr->entry.data[dest_attr->name_begin]) != 0)
                break;
            
            if(strcasecmp(&target_attr->entry.data[target_attr->value_begin], &dest_attr->entry.data[dest_attr->value_begin]) != 0)
                break;
        }
        else
            break;
        
        target_attr = target_attr->next;
        dest_attr   = dest_attr->next;
    }
    
    if(target_attr == NULL && dest_attr == NULL)
        return mytrue;
    
    return myfalse;
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
    mcobject_async_free(token->nodes_obj, 0, node);
}

myhtml_token_attr_t * myhtml_token_attr_remove_by_name(myhtml_token_node_t* node, const char* name, size_t name_length)
{
    myhtml_token_attr_t* attr = myhtml_token_attr_by_name(node, name, name_length);
    
    if(attr)
    {
        if(node->attr_first == attr)
            node->attr_first = attr->next;
        
        if(node->attr_last == attr)
            node->attr_last = attr->prev;
        
        if(attr->next) {
            attr->next->prev = attr->prev;
            attr->next = NULL;
        }
        
        if(attr->prev) {
            attr->prev->next = attr->next;
            attr->prev = NULL;
        }
    }
    
    return attr;
}

mybool_t myhtml_token_is_whithspace(myhtml_tree_t* tree, myhtml_token_node_t* node)
{
    const char* html = node->my_str_tm.data;
    
    size_t i;
    for (i = node->begin; i < node->length; i++)
    {
        // TODO: see and use myhtml_whithspace
        // U+0009 // U+000A // U+000C // U+000D // U+0020
        if(html[i] != '\t' && html[i] != '\n' && html[i] != '\f' && html[i] != '\r' && html[i] != ' ') {
            return myfalse;
        }
    }
    
    return mytrue;
}

void myhtml_token_print_param_by_idx(myhtml_tree_t* myhtml_tree, myhtml_token_node_t* node, FILE* out)
{
    if(node->type & MyHTML_TOKEN_TYPE_CLOSE) {
        fprintf(out, "</");
    }
    else {
        fprintf(out, "<");
    }
    
    fprintf(out, "tag_ctx_idx=%lu; body_begin=%lu; body_length=%lu; attr_first=%lu; attr_last=%lu",
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
    myhtml_t* myhtml            = tree->myhtml;
    mctree_node_t* mctree_nodes = myhtml->tags->tree->nodes;
    
    size_t mctree_id = mh_tags_get(node->tag_ctx_idx, mctree_id);
    size_t tag_name_size = mctree_nodes[mctree_id].str_size;
    
    if(node->tag_ctx_idx == MyHTML_TAG__TEXT ||
       node->tag_ctx_idx == MyHTML_TAG__COMMENT)
    {
        if(node->length) {
            fprintf(out, "%.*s: %.*s\n", (int)tag_name_size, mctree_nodes[mctree_id].str,
                    (int)node->length, &node->my_str_tm.data[node->begin]);
        }
        else {
            fprintf(out, "%.*s is empty\n", (int)tag_name_size, mctree_nodes[mctree_id].str);
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
        
        fprintf(out, "%.*s tagid=\"%lu\"", (int)tag_name_size, mctree_nodes[mctree_id].str,
                mh_tags_get(node->tag_ctx_idx, id));
        
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
        if(attr->value_begin)
        {
            fprintf(out, " %.*s=\"%.*s\"",
                    (int)attr->name_length, &attr->entry.data[attr->name_begin],
                    (int)attr->value_length, &attr->entry.data[attr->value_begin]);
        }
        else {
            fprintf(out, " %.*s", (int)attr->name_length, &attr->entry.data[attr->name_begin]);
        }
        
        attr = attr->next;
    }
}



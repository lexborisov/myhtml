//
//  myhtml_token.c
//  myhtml
//
//  Created by Alexander Borisov on 26.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_token.h"

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


void myhtml_token_callback_for_new(mcobject_async_t* mcobj_async, size_t idx)
{
    myhtml_token_node_t* nodes = (myhtml_token_node_t*)mcobj_async->mem;
    
    nodes[idx].entry.data   = NULL;
    nodes[idx].entry.length = 0;
    nodes[idx].entry.size   = 0;
}

myhtml_token_t * myhtml_token_create(size_t size)
{
    myhtml_token_t* token = (myhtml_token_t*)mymalloc(sizeof(myhtml_token_t));
    
    token->nodes_obj = mcobject_async_create(256, size, sizeof(myhtml_token_node_t));
    token->attr_obj  = mcobject_async_create(256, size, sizeof(myhtml_token_attr_t));
    
    mcobject_async_node_add(token->nodes_obj, 4096);
    mcobject_async_node_add(token->attr_obj , 4096);
    
    return token;
}

void myhtml_token_clean(myhtml_token_t* token)
{
    mcobject_async_clean(token->nodes_obj);
    mcobject_async_clean(token->attr_obj);
}

myhtml_token_t * myhtml_token_destroy(myhtml_token_t* token)
{
    if(token == NULL)
        return NULL;
    
//    size_t i;
//    for(i = 0; i < token->nodes_obj->mem_length; i++) {
//        myhtml_string_destroy(&token->nodes[i].entry, myfalse);
//    }
    
    if(token->nodes_obj)
        token->nodes_obj = mcobject_async_destroy(token->nodes_obj, 1);
    
    if(token->attr_obj)
        token->attr_obj = mcobject_async_destroy(token->attr_obj, 1);
    
    return NULL;
}

void myhtml_token_node_clean(myhtml_token_node_t* node)
{
    node->tag_ctx_idx    = MyTAGS_TAG__TEXT;
    node->type           = MyHTML_TOKEN_TYPE_OPEN;
    node->attr_first     = NULL;
    node->attr_last      = NULL;
    node->begin          = 0;
    node->length         = 0;
    node->is_done        = myfalse;
    
    myhtml_string_clean(&node->entry);
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
}

void myhtml_token_node_wait_for_done(myhtml_token_node_t* node)
{
    while(node->is_done == myfalse) {}
}

myhtml_token_node_t * myhtml_token_clone(myhtml_token_t* token, myhtml_token_node_t* node, size_t thread_idx)
{
    if(node == NULL)
        return NULL;
    
    myhtml_token_node_t* new_node = mcobject_async_malloc(token->nodes_obj, thread_idx);
    
    new_node->tag_ctx_idx = node->tag_ctx_idx;
    new_node->type        = node->type;
    new_node->attr_first  = 0;
    new_node->attr_last   = 0;
    new_node->begin       = node->begin;
    new_node->length      = node->length;
    
    myhtml_string_init(&new_node->entry, node->entry.size);
    myhtml_token_node_attr_copy(token, node, new_node, thread_idx);
    
    return new_node;
}

void myhtml_token_node_text_append(myhtml_token_t* token, myhtml_token_node_t* dest, const char* text, size_t text_len)
{
    myhtml_string_init(&dest->entry, (text_len + 512));
    
    myhtml_string_t* string = &dest->entry;
    
    dest->begin  = myhtml_string_len(string);
    dest->length = text_len;
    
    myhtml_string_append_with_null(string, text, text_len);
}

myhtml_token_attr_t * myhtml_token_node_attr_append(myhtml_token_t* token, myhtml_token_node_t* dest,
                                   const char* key, size_t key_len,
                                   const char* value, size_t value_len, size_t thread_idx)
{
    myhtml_string_t* dest_string = &dest->entry;
    
    myhtml_string_init(&dest->entry, (key_len + value_len + 512));
    
    myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx);
    new_attr->next = 0;
    
    if(key_len)
    {
        new_attr->name_begin = myhtml_string_len(dest_string);
        new_attr->name_length = key_len;
        
        myhtml_string_append_lowercase_with_null(dest_string,
                                                 key,
                                                 key_len);
    }
    else {
        new_attr->name_begin  = 0;
        new_attr->name_length = 0;
    }
    
    if(value_len)
    {
        new_attr->value_begin = myhtml_string_len(dest_string);
        new_attr->value_length = value_len;
        
        myhtml_string_append_lowercase_with_null(dest_string,
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
        myhtml_token_attr_copy(token, target, attr, dest, thread_idx);
        attr = attr->next;
    }
}

mybool_t myhtml_token_attr_copy(myhtml_token_t* token, myhtml_token_node_t* target, myhtml_token_attr_t* attr,
                            myhtml_token_node_t* dest, size_t thread_idx)
{
    myhtml_string_t* targ_string = &target->entry;
    myhtml_string_t* dest_string = &dest->entry;
    
//    if(myhtml_token_attr_by_name(target, &targ_string->data[attr->name_begin], attr->name_length)) {
//        return myfalse;
//    }
    
    myhtml_token_attr_t* new_attr = mcobject_async_malloc(token->attr_obj, thread_idx);
    new_attr->next = 0;
    
    myhtml_string_init(&dest->entry, 512 + attr->name_length + attr->value_begin);
    
    if(attr->name_length)
    {
        new_attr->name_begin = myhtml_string_len(dest_string);
        new_attr->name_length = attr->name_length;
        
        myhtml_string_append_lowercase_with_null(dest_string,
                                                 &targ_string->data[attr->name_begin],
                                                 attr->name_length);
    }
    else {
        new_attr->name_begin  = 0;
        new_attr->name_length = 0;
    }
    
    if(attr->value_length)
    {
        new_attr->value_begin = myhtml_string_len(dest_string);
        new_attr->value_length = attr->value_length;
        
        myhtml_string_append_lowercase_with_null(dest_string,
                                                 &targ_string->data[attr->value_begin],
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
    myhtml_string_t* targ_string = &target->entry;
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        if(attr->name_length == key_size && attr->value_length == value_size)
        {
            if((strncmp(key, &targ_string->data[attr->name_begin], key_size) == 0)) {
               if((strncmp(value, &targ_string->data[attr->value_begin], value_size) == 0)) {
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
    myhtml_string_t* targ_string = &target->entry;
    myhtml_token_attr_t* attr = target->attr_first;
    
    while (attr)
    {
        if(attr->name_length == key_size && attr->value_length == value_size)
        {
            if((strncmp(key, &targ_string->data[attr->name_begin], key_size) == 0)) {
                if((strncasecmp(value, &targ_string->data[attr->value_begin], value_size) == 0)) {
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
        myhtml_string_t* targ_string = &target->entry;
        memcpy(&targ_string->data[attr->name_begin], "definitionURL", 13);
    }
}

void _myhtml_token_create_copy_srt(const char* from, size_t from_size, char** to)
{
    if(*to)
        myfree(*to);
    
    *to = mymalloc(from_size + 2);
    
    strcpy(*to, from);
}

void myhtml_token_strict_doctype_by_token(myhtml_token_node_t* target, myhtml_tree_doctype_t* return_doctype)
{
    myhtml_string_t* targ_string = &target->entry;
    myhtml_token_attr_t* attr = target->attr_first;
    
    const char* data = targ_string->data;
    
    if(attr && attr->name_length) {
        _myhtml_token_create_copy_srt(&data[attr->name_begin], attr->name_length, &return_doctype->name);
        
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
        if(strcmp("PUBLIC", &data[attr->name_begin]) == 0)
        {
            // try see public
            attr = attr->next;
            
            if(attr && attr->name_length) {
                _myhtml_token_create_copy_srt(&data[attr->name_begin], attr->name_length, &return_doctype->public);
                
                // try see system
                attr = attr->next;
                
                if(attr && attr->name_length) {
                    _myhtml_token_create_copy_srt(&data[attr->name_begin], attr->name_length, &return_doctype->system);
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
                _myhtml_token_create_copy_srt(&data[attr->name_begin], attr->name_length, &return_doctype->system);
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
    if(return_doctype->system == NULL)
        return mytrue;
    
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
    return strcmp(return_doctype->public, "-//W3C//DTD XHTML 1.1//EN") &&
    strcmp(return_doctype->system, "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd");
}

mybool_t myhtml_token_release_and_check_doctype_attributes(myhtml_token_node_t* target, myhtml_tree_doctype_t* return_doctype)
{
    if(return_doctype == NULL)
        return myfalse;
    
    myhtml_token_strict_doctype_by_token(target, return_doctype);
    
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
            myhtml_string_t* targ_string = &target->entry;
            memcpy(&targ_string->data[attr->name_begin],
                   myhtml_token_attr_svg_replacement[i].to,
                   myhtml_token_attr_svg_replacement[i].from_size);
            // from_size == to_size, but i use from_size for copy
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
            myhtml_string_t* targ_string = &target->entry;
            memcpy(&targ_string->data[attr->name_begin],
                   myhtml_token_attr_namespace_replacement[i].to,
                   myhtml_token_attr_namespace_replacement[i].to_size);
            
            attr->name_length = myhtml_token_attr_namespace_replacement[i].to_size;
            targ_string->data[attr->name_length] = '\0';
            
            attr->namespace = myhtml_token_attr_namespace_replacement[i].namespace;
        }
    }
}

mybool_t myhtml_token_attr_compare(myhtml_token_node_t* target, myhtml_token_node_t* dest)
{
    if(target == NULL || dest == NULL)
        return myfalse;
    
    char* target_char = target->entry.data;
    char* dest_char   = dest->entry.data;
    
    myhtml_token_attr_t* target_attr = target->attr_first;
    myhtml_token_attr_t* dest_attr   = dest->attr_first;
    
    while (target_attr && dest_attr)
    {
        if(target_attr->name_length == dest_attr->name_length &&
           target_attr->value_length == dest_attr->value_length)
        {
            if(strcmp(&target_char[target_attr->name_begin], &dest_char[dest_attr->name_begin]) != 0)
                break;
            
            if(strcasecmp(&target_char[target_attr->value_begin], &dest_char[dest_attr->value_begin]) != 0)
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
    char* buff = node->entry.data;
    
    while (attr)
    {
        if(name_length == attr->name_length) {
            if(strncmp(name, &buff[attr->name_begin], name_length) == 0)
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
    const char* html = node->entry.data;
    
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
    
    if(node->tag_ctx_idx == MyTAGS_TAG__TEXT ||
       node->tag_ctx_idx == MyTAGS_TAG__COMMENT)
    {
        if(node->length) {
            fprintf(out, "%.*s: %.*s\n", (int)tag_name_size, mctree_nodes[mctree_id].str,
                    (int)node->length, &node->entry.data[node->begin]);
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
    const char* html = node->entry.data;
    
    myhtml_token_attr_t* attr = node->attr_first;
    
    while(attr)
    {
        fprintf(out, " ");
        
        if(attr->value_begin)
        {
            fprintf(out, "%.*s=\"%.*s\"",
                    (int)attr->name_length, &html[attr->name_begin],
                    (int)attr->value_length, &html[attr->value_begin]);
        }
        else {
            fprintf(out, "%.*s", (int)attr->name_length, &html[attr->name_begin]);
        }
        
        attr = attr->next;
    }
}



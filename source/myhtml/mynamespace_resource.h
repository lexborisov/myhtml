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

#ifndef MyHTML_MYNAMESPACE_RESOURCE_H
#define MyHTML_MYNAMESPACE_RESOURCE_H
#pragma once

static const myhtml_namespace_detect_name_entry_t myhtml_namespace_detect_name_entry_static_list_index[] =
{
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {"SVG", 3, MyHTML_NAMESPACE_SVG, 0, 6},
    {"XML", 3, MyHTML_NAMESPACE_XML, 0, 7},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {"MATHML", 6, MyHTML_NAMESPACE_MATHML, 0, 10},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {"XMLNS", 5, MyHTML_NAMESPACE_XMLNS, 0, 12},
    {"HTML", 4, MyHTML_NAMESPACE_HTML, 0, 13},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF, 0, 0},
    {"XLINK", 5, MyHTML_NAMESPACE_XLINK, 0, 19},
};

static const char myhtml_namespace_resource_names_map[(MyHTML_NAMESPACE_LAST_ENTRY + 1)][8] = {
    "", "HTML", "MathML", "SVG", "XLink", "XML", "XMLNS", "*"
};

static const myhtml_namespace_detect_url_entry_t myhtml_namespace_detect_url_entry_static_list_index[(MyHTML_NAMESPACE_LAST_ENTRY + 1)] = {
    {"undef", 5, MyHTML_NAMESPACE_UNDEF},
    {"http://www.w3.org/1999/xhtml", 28, MyHTML_NAMESPACE_HTML},
    {"http://www.w3.org/1998/Math/MathML", 34, MyHTML_NAMESPACE_MATHML},
    {"http://www.w3.org/2000/svg", 26, MyHTML_NAMESPACE_SVG},
    {"http://www.w3.org/1999/xlink", 28, MyHTML_NAMESPACE_XLINK},
    {"http://www.w3.org/XML/1998/namespace", 36, MyHTML_NAMESPACE_XML},
    {"http://www.w3.org/2000/xmlns/", 29, MyHTML_NAMESPACE_XMLNS},
    {NULL, 0, MyHTML_NAMESPACE_UNDEF}
};

#endif /* MyHTML_MYNAMESPACE_RESOURCE_H */

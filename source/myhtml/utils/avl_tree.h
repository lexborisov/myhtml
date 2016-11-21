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
 Foundation, Inc., 51 Franklin avl_treet, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#ifndef MyHTML_UTILS_AVL_TREE_H
#define MyHTML_UTILS_AVL_TREE_H
#pragma once

#include "myhtml/myosi.h"
#include "myhtml/utils/mcobject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct myhtml_utils_avl_tree_node myhtml_utils_avl_tree_node_t;
typedef void (*myhtml_utils_avl_tree_node_callback_f)(myhtml_utils_avl_tree_node_t* avl_tree_node, void* ctx);

struct myhtml_utils_avl_tree_node {
    void  *value;
    size_t type;
    
    myhtml_utils_avl_tree_node_t* left;
    myhtml_utils_avl_tree_node_t* right;
    myhtml_utils_avl_tree_node_t* parent;
    
    short height;
};

struct myhtml_utils_avl_tree {
    mcobject_t* mc_nodes;
}
typedef myhtml_utils_avl_tree_t;

myhtml_utils_avl_tree_t * myhtml_utils_avl_tree_create(void);
myhtml_status_t myhtml_utils_avl_tree_init(myhtml_utils_avl_tree_t* avl_tree);
void myhtml_utils_avl_tree_clean(myhtml_utils_avl_tree_t* avl_tree);
myhtml_utils_avl_tree_t * myhtml_utils_avl_tree_destroy(myhtml_utils_avl_tree_t* avl_tree, bool self_destroy);

myhtml_utils_avl_tree_node_t * myhtml_utils_avl_tree_node_create_root(myhtml_utils_avl_tree_t* avl_tree, size_t type, void* value);

void myhtml_utils_avl_tree_add(myhtml_utils_avl_tree_t* avl_tree, myhtml_utils_avl_tree_node_t** root, size_t type, void* value);
void * myhtml_utils_avl_tree_delete(myhtml_utils_avl_tree_t *avl_tree, myhtml_utils_avl_tree_node_t** root, size_t type);
myhtml_utils_avl_tree_node_t * myhtml_utils_avl_tree_search_by_type(myhtml_utils_avl_tree_t *avl_tree, myhtml_utils_avl_tree_node_t* node, size_t type);

void myhtml_utils_avl_tree_list_all_nodes(myhtml_utils_avl_tree_t *avl_tree, myhtml_utils_avl_tree_node_t* root, myhtml_utils_avl_tree_node_callback_f callback, void* ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyHTML_UTILS_AVL_TREE_H */

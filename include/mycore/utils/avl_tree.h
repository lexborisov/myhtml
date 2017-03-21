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
 Foundation, Inc., 51 Franklin avl_treet, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#ifndef MyCORE_UTILS_AVL_TREE_H
#define MyCORE_UTILS_AVL_TREE_H
#pragma once

#include <mycore/myosi.h>
#include <mycore/utils/mcobject.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mycore_utils_avl_tree_node mycore_utils_avl_tree_node_t;
typedef void (*mycore_utils_avl_tree_node_callback_f)(mycore_utils_avl_tree_node_t* avl_tree_node, void* ctx);

struct mycore_utils_avl_tree_node {
    void  *value;
    size_t type;
    
    mycore_utils_avl_tree_node_t* left;
    mycore_utils_avl_tree_node_t* right;
    mycore_utils_avl_tree_node_t* parent;
    
    short height;
};

struct mycore_utils_avl_tree {
    mcobject_t* mc_nodes;
}
typedef mycore_utils_avl_tree_t;

mycore_utils_avl_tree_t * mycore_utils_avl_tree_create(void);
mystatus_t mycore_utils_avl_tree_init(mycore_utils_avl_tree_t* avl_tree);
void mycore_utils_avl_tree_clean(mycore_utils_avl_tree_t* avl_tree);
mycore_utils_avl_tree_t * mycore_utils_avl_tree_destroy(mycore_utils_avl_tree_t* avl_tree, bool self_destroy);

mycore_utils_avl_tree_node_t * mycore_utils_avl_tree_node_create_root(mycore_utils_avl_tree_t* avl_tree, size_t type, void* value);

void mycore_utils_avl_tree_add(mycore_utils_avl_tree_t* avl_tree, mycore_utils_avl_tree_node_t** root, size_t type, void* value);
void * mycore_utils_avl_tree_delete(mycore_utils_avl_tree_t *avl_tree, mycore_utils_avl_tree_node_t** root, size_t type);
mycore_utils_avl_tree_node_t * mycore_utils_avl_tree_search_by_type(mycore_utils_avl_tree_t *avl_tree, mycore_utils_avl_tree_node_t* node, size_t type);

void mycore_utils_avl_tree_list_all_nodes(mycore_utils_avl_tree_t *avl_tree, mycore_utils_avl_tree_node_t* root, mycore_utils_avl_tree_node_callback_f callback, void* ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MyCORE_UTILS_AVL_TREE_H */

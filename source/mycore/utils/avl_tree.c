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

#include "mycore/utils/avl_tree.h"

mycore_utils_avl_tree_t * mycore_utils_avl_tree_create(void)
{
    return (mycore_utils_avl_tree_t*)mycore_calloc(1, sizeof(mycore_utils_avl_tree_t));
}

mystatus_t mycore_utils_avl_tree_init(mycore_utils_avl_tree_t* avl_tree)
{
    /* for raw declaration style */
    avl_tree->mc_nodes = mcobject_create();
    if(avl_tree->mc_nodes == NULL)
        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    
    mystatus_t mycore_status = mcobject_init(avl_tree->mc_nodes, 256, sizeof(mycore_utils_avl_tree_node_t));
    if(mycore_status)
        return MyCORE_STATUS_ERROR;
    
    return MyCORE_STATUS_OK;
}

void mycore_utils_avl_tree_clean(mycore_utils_avl_tree_t* avl_tree)
{
    mcobject_clean(avl_tree->mc_nodes);
}

mycore_utils_avl_tree_t * mycore_utils_avl_tree_destroy(mycore_utils_avl_tree_t* avl_tree, bool self_destroy)
{
    if(avl_tree == NULL)
        return NULL;
    
    mcobject_destroy(avl_tree->mc_nodes, true);
    
    if(self_destroy) {
        mycore_free(avl_tree);
        return NULL;
    }
    
    return avl_tree;
}

mycore_utils_avl_tree_node_t * mycore_utils_avl_tree_node_create_root(mycore_utils_avl_tree_t* avl_tree, size_t type, void* value)
{
    mycore_utils_avl_tree_node_t *node = mcobject_malloc(avl_tree->mc_nodes, NULL);
    memset(node, 0, sizeof(mycore_utils_avl_tree_node_t));
    
    node->type  = type;
    node->value = value;
    
    return node;
}

void mycore_utils_avl_tree_node_clean(mycore_utils_avl_tree_node_t* node)
{
    memset(node, 0, sizeof(mycore_utils_avl_tree_node_t));
}

short mycore_utils_avl_tree_node_height(mycore_utils_avl_tree_node_t* node)
{
    return (node ? node->height : 0);
}

short mycore_utils_avl_tree_node_balance_factor(mycore_utils_avl_tree_node_t* node)
{
    return (mycore_utils_avl_tree_node_height(node->right) - mycore_utils_avl_tree_node_height(node->left));
}

void mycore_utils_avl_tree_node_set_height(mycore_utils_avl_tree_node_t* node)
{
    short left_height = mycore_utils_avl_tree_node_height(node->left);
    short right_height = mycore_utils_avl_tree_node_height(node->right);
    
    node->height = (left_height > right_height ? left_height : right_height) + 1;
}

mycore_utils_avl_tree_node_t * mycore_utils_avl_tree_node_rotate_right(mycore_utils_avl_tree_node_t* pos)
{
    mycore_utils_avl_tree_node_t* node = pos->left;
    
    node->parent = pos->parent;
    
    if(node->right)
        node->right->parent = pos;
    
    pos->left = node->right;
    pos->parent = node;
    
    node->right = pos;
    
    mycore_utils_avl_tree_node_set_height(pos);
    mycore_utils_avl_tree_node_set_height(node);
    
    return node;
}

mycore_utils_avl_tree_node_t * mycore_utils_avl_tree_node_rotate_left(mycore_utils_avl_tree_node_t* pos)
{
    mycore_utils_avl_tree_node_t* node = pos->right;
    
    node->parent = pos->parent;
    
    if(node->left)
        node->left->parent = pos;
    
    pos->right = node->left;
    pos->parent = node;
    
    node->left = pos;
    
    mycore_utils_avl_tree_node_set_height(pos);
    mycore_utils_avl_tree_node_set_height(node);
    
    return node;
}

mycore_utils_avl_tree_node_t * mycore_utils_avl_tree_node_balance(mycore_utils_avl_tree_node_t* node, mycore_utils_avl_tree_node_t** root)
{
    /* set height */
    short left_height = mycore_utils_avl_tree_node_height(node->left);
    short right_height = mycore_utils_avl_tree_node_height(node->right);
    
    node->height = (left_height > right_height ? left_height : right_height) + 1;
    
    /* check balance */
    switch ((right_height - left_height))
    {
        case 2: {
            if(mycore_utils_avl_tree_node_balance_factor(node->right) < 0)
                node->right = mycore_utils_avl_tree_node_rotate_right(node->right);
            
            mycore_utils_avl_tree_node_t* parent = node->parent;
            
            if(parent) {
                if(parent->right == node)
                    return parent->right = mycore_utils_avl_tree_node_rotate_left(node);
                else
                    return parent->left = mycore_utils_avl_tree_node_rotate_left(node);
            }
            
            return mycore_utils_avl_tree_node_rotate_left(node);
        }
        case -2: {
            if(mycore_utils_avl_tree_node_balance_factor(node->left) > 0)
                node->left = mycore_utils_avl_tree_node_rotate_left(node->left);
            
            mycore_utils_avl_tree_node_t* parent = node->parent;
            
            if(parent) {
                if(parent->right == node)
                    return parent->right = mycore_utils_avl_tree_node_rotate_right(node);
                else
                    return parent->left = mycore_utils_avl_tree_node_rotate_right(node);
            }
            
            return mycore_utils_avl_tree_node_rotate_right(node);
        }
        default:
            break;
    }
    
    if(node->parent == NULL)
        *root = node;
    
    return node->parent;
}

void mycore_utils_avl_tree_add(mycore_utils_avl_tree_t* avl_tree, mycore_utils_avl_tree_node_t** root, size_t type, void* value)
{
    if(*root == NULL) {
        *root = mycore_utils_avl_tree_node_create_root(avl_tree, type, value);
        return;
    }
    
    mycore_utils_avl_tree_node_t* node = *root;
    mycore_utils_avl_tree_node_t* new_node = mcobject_malloc(avl_tree->mc_nodes, NULL);
    mycore_utils_avl_tree_node_clean(new_node);
    
    while(1)
    {
        if(type == node->type) {
            node->value = value;
            return;
        }
        else if(type < node->type) {
            if(node->left == NULL) {
                node->left = new_node;
                
                new_node->parent = node;
                new_node->type   = type;
                new_node->value  = value;
                
                node = new_node;
                break;
            }
            
            node = node->left;
        }
        else {
            if(node->right == NULL) {
                node->right = new_node;
                
                new_node->parent = node;
                new_node->type   = type;
                new_node->value  = value;
                
                node = new_node;
                break;
            }
            
            node = node->right;
        }
    }
    
    while(node) {
        node = mycore_utils_avl_tree_node_balance(node, root);
    }
}

mycore_utils_avl_tree_node_t * mycore_utils_avl_tree_find_min(mycore_utils_avl_tree_node_t* node)
{
    if(node == NULL)
        return NULL;
    
    while(node->right) {
        node = node->right;
    }
    
    return node;
}

void mycore_utils_avl_tree_rotate_for_delete(mycore_utils_avl_tree_node_t* delete_node, mycore_utils_avl_tree_node_t* node, mycore_utils_avl_tree_node_t** root)
{
    mycore_utils_avl_tree_node_t* balance_node;
    
    if(node) {
        if(delete_node->left == node) {
            balance_node = node->left ? node->left : node;
            
            node->parent = delete_node->parent;
            node->right = delete_node->right;
            
            if(delete_node->right)
                delete_node->right->parent = node;
        }
        else {
            balance_node = node;
            
            node->parent->right = NULL;
            node->parent = delete_node->parent;
            node->right = delete_node->right;
            node->left = delete_node->left;
            
            if(delete_node->left)
                delete_node->left->parent = node;
            if(delete_node->right)
                delete_node->right->parent = node;
        }
        
        if(delete_node->parent) {
            if(delete_node->parent->left == delete_node) { delete_node->parent->left = node; }
            else { delete_node->parent->right = node; }
        }
        else {
            *root = node;
        }
    }
    else {
        balance_node = delete_node->parent;
        
        if(delete_node->parent) {
            if(delete_node->parent->left == delete_node) { delete_node->parent->left = delete_node->right; }
            else { delete_node->parent->right = delete_node->right; }
        }
        else {
            *root = delete_node->right;
        }
    }
    
    while(balance_node) {
        balance_node = mycore_utils_avl_tree_node_balance(balance_node, root);
    }
}

void * mycore_utils_avl_tree_delete(mycore_utils_avl_tree_t *avl_tree, mycore_utils_avl_tree_node_t** root, size_t type)
{
    mycore_utils_avl_tree_node_t* node = *root;
    
    while(node)
    {
        if(type == node->type) {
            mycore_utils_avl_tree_rotate_for_delete(node, mycore_utils_avl_tree_find_min(node->left), root);
            
            void *value = node->value;
            mcobject_free(avl_tree->mc_nodes, node);
            
            return value;
        }
        else if(type < node->type)
            node = node->left;
        else
            node = node->right;
    }
    
    return NULL;
}

mycore_utils_avl_tree_node_t * mycore_utils_avl_tree_search_by_type(mycore_utils_avl_tree_t *avl_tree, mycore_utils_avl_tree_node_t* node, size_t type)
{
    while(node)
    {
        if(type == node->type)
            return node;
        else if(type < node->type)
            node = node->left;
        else
            node = node->right;
    }
    
    return NULL;
}

void mycore_utils_avl_tree_list_all_nodes(mycore_utils_avl_tree_t *avl_tree, mycore_utils_avl_tree_node_t* root, mycore_utils_avl_tree_node_callback_f callback, void* ctx)
{
    if(root == NULL)
        return;
    
    callback(root, ctx);
    
    mycore_utils_avl_tree_list_all_nodes(avl_tree, root->left, callback, ctx);
    mycore_utils_avl_tree_list_all_nodes(avl_tree, root->right, callback, ctx);
}



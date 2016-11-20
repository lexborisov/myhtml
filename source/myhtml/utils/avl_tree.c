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

#include "myhtml/utils/avl_tree.h"

myhtml_utils_avl_tree_t * myhtml_utils_avl_tree_create(void)
{
    return (myhtml_utils_avl_tree_t*)myhtml_calloc(1, sizeof(myhtml_utils_avl_tree_t));
}

myhtml_status_t myhtml_utils_avl_tree_init(myhtml_utils_avl_tree_t* avl_tree)
{
    /* for raw declaration style */
    avl_tree->mc_nodes = mcobject_create();
    if(avl_tree->mc_nodes == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    myhtml_status_t myhtml_status = mcobject_init(avl_tree->mc_nodes, 256, sizeof(myhtml_utils_avl_tree_node_t));
    if(myhtml_status)
        return MyHTML_STATUS_ERROR;
    
    return MyHTML_STATUS_OK;
}

void myhtml_utils_avl_tree_clean(myhtml_utils_avl_tree_t* avl_tree)
{
    mcobject_clean(avl_tree->mc_nodes);
}

myhtml_utils_avl_tree_t * myhtml_utils_avl_tree_destroy(myhtml_utils_avl_tree_t* avl_tree, bool self_destroy)
{
    if(avl_tree == NULL)
        return NULL;
    
    mcobject_destroy(avl_tree->mc_nodes, true);
    
    if(self_destroy) {
        myhtml_free(avl_tree);
        return NULL;
    }
    
    return avl_tree;
}

myhtml_utils_avl_tree_node_t * myhtml_utils_avl_tree_node_create_root(myhtml_utils_avl_tree_t* avl_tree, size_t type, void* value)
{
    myhtml_utils_avl_tree_node_t *node = mcobject_malloc(avl_tree->mc_nodes, NULL);
    memset(node, 0, sizeof(myhtml_utils_avl_tree_node_t));
    
    node->type  = type;
    node->value = value;
    
    return node;
}

void myhtml_utils_avl_tree_node_clean(myhtml_utils_avl_tree_node_t* node)
{
    memset(node, 0, sizeof(myhtml_utils_avl_tree_node_t));
}

short myhtml_utils_avl_tree_node_height(myhtml_utils_avl_tree_node_t* node)
{
    return (node ? node->height : 0);
}

short myhtml_utils_avl_tree_node_balance_factor(myhtml_utils_avl_tree_node_t* node)
{
    return (myhtml_utils_avl_tree_node_height(node->right) - myhtml_utils_avl_tree_node_height(node->left));
}

void myhtml_utils_avl_tree_node_set_height(myhtml_utils_avl_tree_node_t* node)
{
    short left_height = myhtml_utils_avl_tree_node_height(node->left);
    short right_height = myhtml_utils_avl_tree_node_height(node->right);
    
    node->height = (left_height > right_height ? left_height : right_height) + 1;
}

myhtml_utils_avl_tree_node_t * myhtml_utils_avl_tree_node_rotate_right(myhtml_utils_avl_tree_node_t* pos)
{
    myhtml_utils_avl_tree_node_t* node = pos->left;
    
    node->parent = pos->parent;
    
    if(node->right)
        node->right->parent = pos;
    
    pos->left = node->right;
    pos->parent = node;
    
    node->right = pos;
    
    myhtml_utils_avl_tree_node_set_height(pos);
    myhtml_utils_avl_tree_node_set_height(node);
    
    return node;
}

myhtml_utils_avl_tree_node_t * myhtml_utils_avl_tree_node_rotate_left(myhtml_utils_avl_tree_node_t* pos)
{
    myhtml_utils_avl_tree_node_t* node = pos->right;
    
    node->parent = pos->parent;
    
    if(node->left)
        node->left->parent = pos;
    
    pos->right = node->left;
    pos->parent = node;
    
    node->left = pos;
    
    myhtml_utils_avl_tree_node_set_height(pos);
    myhtml_utils_avl_tree_node_set_height(node);
    
    return node;
}

myhtml_utils_avl_tree_node_t * myhtml_utils_avl_tree_node_balance(myhtml_utils_avl_tree_node_t* node, myhtml_utils_avl_tree_node_t** root)
{
    /* set height */
    short left_height = myhtml_utils_avl_tree_node_height(node->left);
    short right_height = myhtml_utils_avl_tree_node_height(node->right);
    
    node->height = (left_height > right_height ? left_height : right_height) + 1;
    
    /* check balance */
    switch ((right_height - left_height))
    {
        case 2: {
            if(myhtml_utils_avl_tree_node_balance_factor(node->right) < 0)
                node->right = myhtml_utils_avl_tree_node_rotate_right(node->right);
            
            myhtml_utils_avl_tree_node_t* parent = node->parent;
            
            if(parent) {
                if(parent->right == node)
                    return parent->right = myhtml_utils_avl_tree_node_rotate_left(node);
                else
                    return parent->left = myhtml_utils_avl_tree_node_rotate_left(node);
            }
            
            return myhtml_utils_avl_tree_node_rotate_left(node);
        }
        case -2: {
            if(myhtml_utils_avl_tree_node_balance_factor(node->left) > 0)
                node->left = myhtml_utils_avl_tree_node_rotate_left(node->left);
            
            myhtml_utils_avl_tree_node_t* parent = node->parent;
            
            if(parent) {
                if(parent->right == node)
                    return parent->right = myhtml_utils_avl_tree_node_rotate_right(node);
                else
                    return parent->left = myhtml_utils_avl_tree_node_rotate_right(node);
            }
            
            return myhtml_utils_avl_tree_node_rotate_right(node);
        }
        default:
            break;
    }
    
    if(node->parent == NULL)
        *root = node;
    
    return node->parent;
}

void myhtml_utils_avl_tree_add(myhtml_utils_avl_tree_t* avl_tree, myhtml_utils_avl_tree_node_t** root, size_t type, void* value)
{
    if(*root == NULL) {
        *root = myhtml_utils_avl_tree_node_create_root(avl_tree, type, value);
        return;
    }
    
    myhtml_utils_avl_tree_node_t* node = *root;
    myhtml_utils_avl_tree_node_t* new_node = mcobject_malloc(avl_tree->mc_nodes, NULL);
    myhtml_utils_avl_tree_node_clean(new_node);
    
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
        node = myhtml_utils_avl_tree_node_balance(node, root);
    }
}

myhtml_utils_avl_tree_node_t * myhtml_utils_avl_tree_delete(myhtml_utils_avl_tree_t *avl_tree, myhtml_utils_avl_tree_node_t** root, size_t type)
{
    //myhtml_utils_avl_tree_node_t* node = *root;
    return NULL;
}

myhtml_utils_avl_tree_node_t * myhtml_utils_avl_tree_search_by_type(myhtml_utils_avl_tree_t *avl_tree, myhtml_utils_avl_tree_node_t* node, size_t type)
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

void myhtml_utils_avl_tree_list_all_nodes(myhtml_utils_avl_tree_t *avl_tree, myhtml_utils_avl_tree_node_t* root, myhtml_utils_avl_tree_node_callback_f callback, void* ctx)
{
    if(root == NULL)
        return;
    
    callback(root, ctx);
    
    myhtml_utils_avl_tree_list_all_nodes(avl_tree, root->left, callback, ctx);
    myhtml_utils_avl_tree_list_all_nodes(avl_tree, root->right, callback, ctx);
}



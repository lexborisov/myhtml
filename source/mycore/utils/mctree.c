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

#include "mycore/utils/resources.h"
#include "mycore/utils/mctree.h"

mctree_t * mctree_create(size_t start_size)
{
    mctree_t* mctree = (mctree_t*)mycore_malloc(sizeof(mctree_t));
    
    if(mctree == NULL)
        return NULL;
    
    mctree->nodes_size = start_size + 512;
    mctree->nodes_length = start_size + 1;
    mctree->nodes = (mctree_node_t*)mycore_calloc(mctree->nodes_size, sizeof(mctree_node_t));
    
    if(mctree->nodes == NULL) {
        mycore_free(mctree);
        return NULL;
    }
    
    mctree->start_size = start_size;
    
    return mctree;
}

void mctree_clean(mctree_t* mctree)
{
    mctree->nodes_length = mctree->start_size + 1;
    memset(mctree->nodes, 0, sizeof(mctree_node_t) * mctree->nodes_size);
}

mctree_t * mctree_destroy(mctree_t* mctree)
{
    if(mctree == NULL)
        return NULL;
    
    if(mctree->nodes)
        mycore_free(mctree->nodes);
    
    mycore_free(mctree);
    
    return NULL;
}

mctree_index_t __mtree_search_lowercase_to_start(mctree_t* mctree, mctree_index_t idx, const char* key, size_t key_size)
{
    mctree_node_t* nodes = mctree->nodes;
    
    while (idx)
    {
        if(key_size == nodes[idx].str_size) {
            if(mycore_strncasecmp(key, nodes[idx].str, key_size) == 0) {
                return idx;
            }
            
            idx = nodes[idx].child;
        }
        else if(key_size > nodes[idx].str_size)
        {
            if(key_size < nodes[ nodes[idx].next ].str_size) {
                return 0;
            }
            
            idx = nodes[idx].next;
        }
        else {
            if(key_size > nodes[ nodes[idx].prev ].str_size) {
                return 0;
            }
            
            idx = nodes[idx].prev;
        }
    }
    
    return 0;
}

mctree_index_t __mtree_search_to_start(mctree_t* mctree, mctree_index_t idx, const char* key, size_t key_size)
{
    mctree_node_t* nodes = mctree->nodes;
    
    while (idx)
    {
        if(key_size == nodes[idx].str_size) {
            if(memcmp((const void *)key, (const void *)(nodes[idx].str), key_size) == 0) {
                return idx;
            }
            
            idx = nodes[idx].child;
        }
        else if(key_size > nodes[idx].str_size)
        {
            if(key_size < nodes[ nodes[idx].next ].str_size) {
                return 0;
            }
            
            idx = nodes[idx].next;
        }
        else {
            if(key_size > nodes[ nodes[idx].prev ].str_size) {
                return 0;
            }
            
            idx = nodes[idx].prev;
        }
    }
    
    return 0;
}

mctree_index_t mctree_insert_child(mctree_t* mctree, mctree_index_t idx, const char* key, size_t key_size, void* value)
{
    mctree_node_t* nodes = mctree->nodes;
    mctree_index_t new_idx = mctree_node_get_free_id(mctree);
    
    nodes[idx].child = new_idx;
    
    nodes[new_idx].str = key;
    nodes[new_idx].str_size = key_size;
    nodes[new_idx].value = value;
    
    mctree_node_add(mctree);
    
    return new_idx;
}

mctree_index_t mctree_insert_after(mctree_t* mctree, mctree_index_t idx, const char* key, size_t key_size, void* value)
{
    mctree_node_t* nodes = mctree->nodes;
    mctree_index_t new_idx = mctree_node_get_free_id(mctree);
    
    if(nodes[idx].next) {
        nodes[ nodes[idx].next ].prev = new_idx;
        nodes[new_idx].next = nodes[idx].next;
    }
    
    nodes[idx].next = new_idx;
    nodes[new_idx].prev = idx;
    
    nodes[new_idx].str = key;
    nodes[new_idx].str_size = key_size;
    nodes[new_idx].value = value;
    
    mctree_node_add(mctree);
    
    return new_idx;
}

mctree_index_t mctree_insert_before(mctree_t* mctree, mctree_index_t idx, const char* key, size_t key_size, void* value)
{
    mctree_node_t* nodes = mctree->nodes;
    mctree_index_t new_idx = mctree_node_get_free_id(mctree);
    
    if(nodes[idx].prev) {
        nodes[ nodes[idx].prev ].next = new_idx;
        nodes[new_idx].prev = nodes[idx].prev;
    }
    
    nodes[idx].prev = new_idx;
    nodes[new_idx].next = idx;
    
    nodes[new_idx].str = key;
    nodes[new_idx].str_size = key_size;
    nodes[new_idx].value = value;
    
    mctree_node_add(mctree);
    
    return new_idx;
}

mctree_index_t __mtree_insert_to_start(mctree_t* mctree, mctree_index_t idx, const char* key, size_t key_size, void* value, mctree_before_insert_f b_insert)
{
    mctree_node_t* nodes = mctree->nodes;
    
    while (idx)
    {
        if(key_size == nodes[idx].str_size) {
            if(memcmp((const void *)key, (const void *)nodes[idx].str, key_size) == 0)
            {
                if(value)
                    nodes[idx].value = value;
                
                return idx;
            }
            
            if(nodes[idx].child == 0) {
                if(b_insert)
                    b_insert(key, key_size, &value);
                
                return mctree_insert_child(mctree, idx, key, key_size, value);
            }
            
            idx = nodes[idx].child;
        }
        else if(key_size > nodes[idx].str_size)
        {
            if(nodes[idx].next == 0 || key_size < nodes[ nodes[idx].next ].str_size) {
                if(b_insert)
                    b_insert(key, key_size, &value);
                
                return mctree_insert_after(mctree, idx, key, key_size, value);
            }
            
            idx = nodes[idx].next;
        }
        else {
            if(nodes[idx].prev == 0 || key_size > nodes[ nodes[idx].prev ].str_size) {
                if(b_insert)
                    b_insert(key, key_size, &value);
                
                return mctree_insert_before(mctree, idx, key, key_size, value);
            }
            
            idx = nodes[idx].prev;
        }
    }
    
    return 0;
}

mctree_index_t mctree_insert(mctree_t* mctree, const char* key, size_t key_size, void* value, mctree_before_insert_f b_insert)
{
    mctree_node_t* start = mctree->nodes;
    
    if(key_size > 0) {
        mctree_index_t idx = mctree_make_first_idx(mctree, key, key_size);
        
        if(start[idx].child) {
            return __mtree_insert_to_start(mctree, start[idx].child, key, key_size, value, b_insert);
        }
        else {
            if(b_insert)
                b_insert(key, key_size, &value);
            
            return mctree_insert_child(mctree, idx, key, key_size, value);
        }
    }
    
    return 0;
}

mctree_index_t mctree_search(mctree_t* mctree, const char* key, size_t key_size)
{
    mctree_node_t* start = mctree->nodes;
    
    if(key_size > 0) {
        mctree_index_t idx = mctree_make_first_idx(mctree, key, key_size);
        
        if(start[idx].child) {
            return __mtree_search_to_start(mctree, start[idx].child, key, key_size);
        }
    }
    
    return 0;
}

mctree_index_t mctree_search_lowercase(mctree_t* mctree, const char* key, size_t key_size)
{
    mctree_node_t* start = mctree->nodes;
    
    if(key_size > 0) {
        mctree_index_t idx = mctree_make_first_idx(mctree, key, key_size);
        
        if(start[idx].child) {
            return __mtree_search_lowercase_to_start(mctree, start[idx].child, key, key_size);
        }
    }
    
    return 0;
}




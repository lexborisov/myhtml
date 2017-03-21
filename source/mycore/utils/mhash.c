/*
 Copyright (C) 2017 Alexander Borisov
 
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

#include "mycore/utils/mhash.h"

size_t mycore_utils_mhash_hash(const char* key, size_t key_size, size_t table_size)
{
    size_t hash, i;
    
    for(hash = i = 0; i < key_size; i++)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    
    return hash % table_size;
}

mycore_utils_mhash_t * mycore_utils_mhash_create(void)
{
    return mycore_calloc(1, sizeof(mycore_utils_mhash_t));
}

mystatus_t mycore_utils_mhash_init(mycore_utils_mhash_t* mhash, size_t table_size, size_t max_depth)
{
    mystatus_t status;
    
    mhash->mchar_obj = mchar_async_create();
    if(mhash->mchar_obj == NULL)
        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    
    if((status = mchar_async_init(mhash->mchar_obj, 128, 4096)))
        return status;
    
    /* nodest data for input char* */
    mhash->mchar_node = mchar_async_node_add(mhash->mchar_obj, &status);
    if(status)
        return status;
    
    if(table_size < 128)
        table_size = 128;
    
    mhash->table = mycore_calloc(table_size, sizeof(mycore_utils_mhash_entry_t*));
    if(mhash->table == NULL)
        return MyCORE_STATUS_ERROR_MEMORY_ALLOCATION;
    
    if(max_depth < 1)
        max_depth = 1;
    
    mhash->table_max_depth = max_depth;
    mhash->table_size      = table_size;
    
    return MyCORE_STATUS_OK;
}

void mycore_utils_mhash_clean(mycore_utils_mhash_t* mhash)
{
    mchar_async_clean(mhash->mchar_obj);
    memset(mhash->table, 0, (sizeof(mycore_utils_mhash_entry_t*) * mhash->table_size));
}

mycore_utils_mhash_t * mycore_utils_mhash_destroy(mycore_utils_mhash_t* mhash, bool self_destroy)
{
    if(mhash == NULL)
        return NULL;
    
    if(mhash->table) {
        mycore_free(mhash->table);
        mhash->table = NULL;
    }
    
    if(self_destroy) {
        mycore_free(mhash->table);
        return NULL;
    }
    
    return mhash;
}

mycore_utils_mhash_entry_t * mycore_utils_mhash_create_entry(mycore_utils_mhash_t* mhash, const char* key, size_t key_size, void* value)
{
    mycore_utils_mhash_entry_t *entry = (mycore_utils_mhash_entry_t*)
    mchar_async_malloc(mhash->mchar_obj, mhash->mchar_node, sizeof(mycore_utils_mhash_entry_t));
    
    entry->key = mchar_async_malloc(mhash->mchar_obj, mhash->mchar_node, (sizeof(char) * key_size) + 1);
    
    if(entry->key == NULL) {
        mchar_async_free(mhash->mchar_obj, mhash->mchar_node, (char*)entry);
        return NULL;
    }
    
    memcpy(entry->key, key, (sizeof(char) * key_size));
    entry->key[key_size] = '\0';
    
    entry->key_length = key_size;
    entry->value      = value;
    entry->next       = NULL;
    
    return entry;
}

mycore_utils_mhash_entry_t * mycore_utils_mhash_add_with_choice(mycore_utils_mhash_t* mhash, const char* key, size_t key_size)
{
    if(key == NULL || key_size == 0)
        return NULL;
    
    size_t hash_id = mycore_utils_mhash_hash(key, key_size, mhash->table_size);
    
    
    mycore_utils_mhash_entry_t *entry;
    
    if(mhash->table[hash_id] == NULL) {
        /* rebuild table if need */
        if(mhash->table_length >= (mhash->table_size - (mhash->table_size / 4))) {
            mycore_utils_mhash_rebuld(mhash);
        }
        
        mhash->table[hash_id] = mycore_utils_mhash_create_entry(mhash, key, key_size, NULL);
        return mhash->table[hash_id];
    }
    
    size_t depth = 0;
    entry = mhash->table[hash_id];
    
    do {
        if(entry->key_length == key_size) {
            if(strncmp(entry->key, key, key_size) == 0)
                return entry;
        }
        
        if(entry->next == NULL) {
            entry->next = mycore_utils_mhash_create_entry(mhash, key, key_size, NULL);
            
            if(depth > mhash->table_max_depth) {
                mycore_utils_mhash_entry_t *entry_new = entry->next;
                mycore_utils_mhash_rebuld(mhash);
                
                return entry_new;
            }
            
            return entry->next;
        }
        
        depth++;
        entry = entry->next;
    }
    while(1);
}

mycore_utils_mhash_entry_t * mycore_utils_mhash_add(mycore_utils_mhash_t* mhash, const char* key, size_t key_size, void* value)
{
    mycore_utils_mhash_entry_t *entry = mycore_utils_mhash_add_with_choice(mhash, key, key_size);
    
    if(entry)
        entry->value = value;
    
    return entry;
}

mycore_utils_mhash_entry_t * mycore_utils_mhash_search(mycore_utils_mhash_t* mhash, const char* key, size_t key_size, void* value)
{
    if(key == NULL || key_size == 0)
        return NULL;
    
    size_t hash_id = mycore_utils_mhash_hash(key, key_size, mhash->table_size);
    
    mycore_utils_mhash_entry_t *entry = mhash->table[hash_id];
    
    while(entry) {
        if(entry->key_length == key_size) {
            if(strncmp(entry->key, key, key_size) == 0)
                return entry;
        }
        
        entry = entry->next;
    }
    
    return NULL;
}

mycore_utils_mhash_entry_t * mycore_utils_mhash_entry_by_id(mycore_utils_mhash_t* mhash, size_t id)
{
    if(mhash->table_size > id)
        return mhash->table[id];
    
    return NULL;
}

size_t mycore_utils_mhash_get_table_size(mycore_utils_mhash_t* mhash)
{
    return mhash->table_size;
}

mycore_utils_mhash_entry_t * mycore_utils_mhash_rebuild_add_entry(mycore_utils_mhash_t* mhash, const char* key, size_t key_size, mycore_utils_mhash_entry_t *ext_entry)
{
    if(key == NULL || key_size == 0)
        return NULL;
    
    ext_entry->next = NULL;
    
    size_t hash_id = mycore_utils_mhash_hash(key, key_size, mhash->table_size);
    
    if(mhash->table[hash_id] == NULL) {
        mhash->table[hash_id] = ext_entry;
        return ext_entry;
    }
    
    mycore_utils_mhash_entry_t *entry = mhash->table[hash_id];
    
    do {
        if(entry->next == NULL) {
            entry->next = ext_entry;
            break;
        }
        
        entry = entry->next;
    }
    while(1);
    
    return ext_entry;
}

mycore_utils_mhash_entry_t ** mycore_utils_mhash_rebuld(mycore_utils_mhash_t* mhash)
{
    mycore_utils_mhash_entry_t **table = mhash->table;
    size_t size = mhash->table_size;
    
    mhash->table_size = mhash->table_size << 1;
    mhash->table = mycore_calloc(mhash->table_size, sizeof(mycore_utils_mhash_entry_t*));
    
    if(mhash->table == NULL) {
        mhash->table      = table;
        mhash->table_size = size;
        
        return NULL;
    }
    
    for(size_t i = 0; i < mhash->table_size; i++) {
        mycore_utils_mhash_entry_t *entry = table[i];
        
        while(entry) {
            mycore_utils_mhash_rebuild_add_entry(mhash, entry->key, entry->key_length, entry);
            
            entry = entry->next;
        }
    }
    
    mycore_free(table);
    
    return mhash->table;
}



//
//  myhtml_tree.h
//  myhtml
//
//  Created by Alexander Borisov on 08.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_tree_h
#define myhtml_tree_h

#include "myosi.h"
#include "myhtml.h"
#include "myhtml_token.h"
#include "myhtml_string.h"
#include "mcobject.h"

#define myhtml_tree_get(__tree__, __attr__) __tree__->__attr__
#define myhtml_tree_set(__tree__, __attr__) __tree__->__attr__

#define myhtml_tree_token_current(__tree__) myhtml_tree_get(__tree__, token_current)
#define myhtml_tree_token_attr_current(__tree__) myhtml_tree_get(__tree__, attr_current)

#define myhtml_tree_node_get(__tree__, __node_id__, __attr__) __tree__->nodes[__node_id__].__attr__

enum myhtml_tree_node_type {
    MyHTML_TYPE_NONE    = 0,
    MyHTML_TYPE_BLOCK   = 1,
    MyHTML_TYPE_INLINE  = 2,
    MyHTML_TYPE_TABLE   = 3,
    MyHTML_TYPE_META    = 4,
    MyHTML_TYPE_COMMENT = 5
};

enum myhtml_close_type {
    MyHTML_CLOSE_TYPE_NONE  = 0,
    MyHTML_CLOSE_TYPE_NOW   = 1,
    MyHTML_CLOSE_TYPE_SELF  = 2,
    MyHTML_CLOSE_TYPE_BLOCK = 3
};

struct myhtml_tree_node {
    mytags_ctx_index_t tag_idx;
    
    myhtml_tree_index_t prev;                // предыдущий ид элемента этого же уровня
    myhtml_tree_index_t next;                // следеющий ид эелента этого же уровня
    myhtml_tree_index_t child;               // ид чилда
    myhtml_tree_index_t parent;              // ид родителя
    
    myhtml_tree_index_t last_child;          // ид родителя
    
    myhtml_token_index_t token;
};

enum myhtml_tree_compat_mode {
    MyHTML_TREE_COMPAT_MODE_NO_QUIRKS       = 0x00,
    MyHTML_TREE_COMPAT_MODE_QUIRKS          = 0x01,
    MyHTML_TREE_COMPAT_MODE_LIMITED_QUIRKS  = 0x02
};

enum myhtml_tree_doctype_id {
    MyHTML_TREE_DOCTYPE_ID_NAME   = 0x00,
    MyHTML_TREE_DOCTYPE_ID_SYSTEM = 0x01,
    MyHTML_TREE_DOCTYPE_ID_PUBLIC = 0x02
};

enum myhtml_tree_flags {
    MyHTML_TREE_FLAGS_CLEAN        = 0x00,
    MyHTML_TREE_FLAGS_SCRIPT       = 0x01,
    MyHTML_TREE_FLAGS_FRAMESET_OK  = 0x02
};

struct myhtml_tree_indexes {
    mytags_index_t* tags;
};

struct myhtml_tree {
    myhtml_token_t* token;
    myhtml_token_index_t token_current;
    myhtml_token_attr_index_t attr_current;
    
    myhtml_tree_node_t* nodes;
    mcobject_t* nodes_obj; // myhtml_tree_node_t
    
    myhtml_tree_indexes_t* indexes;
    
    myhtml_tree_index_t current;
    myhtml_tree_index_t document;
    myhtml_queue_node_index_t queue;
    
    myhtml_token_attr_t* queue_attr;
    
    enum myhtml_tokenizer_state state;
    enum myhtml_insertion_mode insert_mode;
    enum myhtml_insertion_mode orig_insert_mode;
    enum myhtml_tree_compat_mode compat_mode;
    enum myhtml_tree_flags flags;
    
    myhtml_t* myhtml;
    mytags_ctx_index_t tmp_tag_id;
};


myhtml_tree_t * myhtml_tree_init(myhtml_t* myhtml);
void myhtml_tree_clean(myhtml_tree_t* tree);
myhtml_tree_t * myhtml_tree_destroy(myhtml_tree_t* tree);

void myhtml_tree_node_clean(myhtml_tree_node_t* tree_node);

myhtml_tree_indexes_t * myhtml_tree_index_create(myhtml_tree_t* tree, mytags_t* mytags);

myhtml_tree_index_t myhtml_tree_node_create(myhtml_tree_t* tree);
void myhtml_tree_node_delete(myhtml_tree_t* tree, myhtml_tree_index_t idx);

void myhtml_tree_print_by_tree_idx(myhtml_tree_t* tree, myhtml_tree_index_t idx, FILE* out, size_t inc);

void myhtml_tree_node_add_child(myhtml_tree_t* myhtml_tree, myhtml_tree_index_t root, myhtml_tree_index_t idx);
void myhtml_tree_node_insert_before(myhtml_tree_t* myhtml_tree, myhtml_tree_index_t root, myhtml_tree_index_t idx);
void myhtml_tree_node_insert_after(myhtml_tree_t* myhtml_tree, myhtml_tree_index_t root, myhtml_tree_index_t idx);


#endif /* myhtml_tree_h */



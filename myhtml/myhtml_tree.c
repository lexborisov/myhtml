//
//  myhtml_tree.c
//  myhtml
//
//  Created by Alexander Borisov on 08.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_tree.h"


myhtml_tree_t * myhtml_tree_init(myhtml_t* myhtml)
{
    myhtml_tree_t* tree = (myhtml_tree_t*)malloc(sizeof(myhtml_tree_t));
    
    tree->nodes_obj   = mcobject_create(4096, sizeof(myhtml_tree_node_t), &tree->nodes, NULL);
    tree->token       = myhtml_token_create(4096 * 2);
    tree->indexes     = myhtml_tree_index_create(tree, myhtml->tags);
    
    myhtml_tree_clean(tree);
    
    tree->myhtml = myhtml;
    
    return tree;
}

void myhtml_tree_clean(myhtml_tree_t* tree)
{
    mcobject_clean(tree->nodes_obj);
    
    tree->current  = myhtml_tree_node_create(tree);
    tree->document = tree->current;
    
    tree->state       = MyHTML_TOKENIZER_STATE_DATA;
    tree->insert_mode = MyHTML_INSERTION_MODE_INITIAL;
    tree->queue       = 0;
    tree->compat_mode = MyHTML_TREE_COMPAT_MODE_NO_QUIRKS;
    tree->tmp_tag_id  = 0;
    
    myhtml_token_clean(tree->token);
    
    myhtml_token_node_malloc(tree->token, tree->token_current);
    myhtml_token_attr_malloc(tree->token, tree->attr_current);
}

myhtml_tree_t * myhtml_tree_destroy(myhtml_tree_t* tree)
{
    if(tree == NULL)
        return NULL;
    
    if(tree->nodes)
        free(tree->nodes);
    
    tree->nodes_obj = mcobject_destroy(tree->nodes_obj);
    tree->token     = myhtml_token_destroy(tree->token);
    
    free(tree);
    
    return NULL;
}

void myhtml_tree_node_clean(myhtml_tree_node_t* tree_node)
{
    tree_node->prev          = 0;
    tree_node->next          = 0;
    tree_node->child         = 0;
    tree_node->parent        = 0;
    tree_node->last_child    = 0;
    tree_node->token         = 0;
}

myhtml_tree_indexes_t * myhtml_tree_index_create(myhtml_tree_t* tree, mytags_t* mytags)
{
    myhtml_tree_indexes_t* indexes = (myhtml_tree_indexes_t*)malloc(sizeof(myhtml_tree_indexes_t));
    
    indexes->tags = mytags_index_create(mytags);
    mytags_index_init(mytags, indexes->tags);
    
    return indexes;
}

myhtml_tree_index_t myhtml_tree_combine_with_queue(myhtml_tree_t* myhtml_tree, myhtml_tree_index_t idx)
{
    return 0;
}

myhtml_tree_index_t myhtml_tree_node_create(myhtml_tree_t* tree)
{
    size_t idx = mcobject_malloc(tree->nodes_obj);
    myhtml_tree_node_clean(&tree->nodes[idx]);
    return idx;
}

void myhtml_tree_node_add_child(myhtml_tree_t* myhtml_tree, myhtml_tree_index_t root, myhtml_tree_index_t idx)
{
    myhtml_tree_node_t* nodes = myhtml_tree->nodes;
    
    if(nodes[root].last_child) {
        nodes[ nodes[root].last_child ].next = idx;
        nodes[idx].prev = nodes[root].last_child;
    }
    else {
        nodes[root].child = idx;
    }
    
    nodes[idx].parent = root;
    nodes[root].last_child = idx;
}

void myhtml_tree_node_insert_before(myhtml_tree_t* myhtml_tree, myhtml_tree_index_t root, myhtml_tree_index_t idx)
{
    myhtml_tree_node_t* nodes = myhtml_tree->nodes;
    
    if(nodes[root].prev) {
        nodes[ nodes[root].prev ].next = idx;
        nodes[idx].prev = nodes[root].prev;
    }
    else {
        nodes[ nodes[root].parent ].child = idx;
    }
    
    nodes[idx].parent = nodes[root].parent;
    nodes[idx].next   = root;
    nodes[root].prev  = idx;
}

void myhtml_tree_node_insert_after(myhtml_tree_t* myhtml_tree, myhtml_tree_index_t root, myhtml_tree_index_t idx)
{
    myhtml_tree_node_t* nodes = myhtml_tree->nodes;
    
    if(nodes[root].next) {
        nodes[ nodes[root].next ].prev = idx;
        nodes[idx].next = nodes[root].next;
    }
    else {
        nodes[ nodes[root].parent ].last_child = idx;
    }
    
    nodes[idx].parent = nodes[root].parent;
    nodes[idx].prev   = root;
    nodes[root].next  = idx;
}

void myhtml_tree_node_delete(myhtml_tree_t* tree, myhtml_tree_index_t idx)
{
    if(idx) {
        myhtml_tree_node_t* nodes = tree->nodes;
        
        myhtml_token_delete(tree->token, nodes[idx].token);
        mcobject_free(tree->nodes_obj, idx);
    }
}



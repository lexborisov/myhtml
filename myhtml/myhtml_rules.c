//
//  myhtml_rules.c
//  myhtml
//
//  Created by Alexander Borisov on 25.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml_rules.h"


void myhtml_insertion_mode_initial(myhtml_tree_t* tree, myhtml_queue_node_index_t qnode_idx)
{
    myhtml_t* myhtml = tree->myhtml;
    
    switch (mh_queue_token_get(qnode_idx, tag_ctx_idx))
    {
        case MyTAGS_TAG__TEXT:
        {
            if(myhtml_token_is_whithspace(tree, qnode_idx)) {
                break;
            }
        }
            
        case MyTAGS_TAG__COMMENT:
        {
            //myhtml_tree_add_child(tree, tree->document, qnode_idx);
            
            break;
        }
        case MyTAGS_TAG__DOCTYPE:
            
            break;
            
        default:
            break;
    }
}

void myhtml_insertion_mode_before_html(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_before_head(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_head(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_head_noscript(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_after_head(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_body(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_text(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_table(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_table_text(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_caption(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_column_group(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_table_body(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_row(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_cell(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_select(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_select_in_table(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_template(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_after_body(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_in_frameset(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_after_frameset(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_after_after_body(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_insertion_mode_after_after_frameset(myhtml_tree_t* myhtml_tree, myhtml_queue_node_index_t qnode_idx)
{
    
}

void myhtml_rules_init(myhtml_t* myhtml)
{
    myhtml->insertion_func = (myhtml_insertion_f*)malloc(sizeof(myhtml_insertion_f) * MyHTML_INSERTION_MODE_LAST_ENTRY);
    
    myhtml->insertion_func[MyHTML_INSERTION_MODE_INITIAL] = myhtml_insertion_mode_initial;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_BEFORE_HTML] = myhtml_insertion_mode_before_html;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_BEFORE_HEAD] = myhtml_insertion_mode_before_head;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_HEAD] = myhtml_insertion_mode_in_head;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_HEAD_NOSCRIPT] = myhtml_insertion_mode_in_head_noscript;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_HEAD] = myhtml_insertion_mode_after_head;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_BODY] = myhtml_insertion_mode_in_body;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_TEXT] = myhtml_insertion_mode_text;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_TABLE] = myhtml_insertion_mode_in_table;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_TABLE_TEXT] = myhtml_insertion_mode_in_table_text;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_CAPTION] = myhtml_insertion_mode_in_caption;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_COLUMN_GROUP] = myhtml_insertion_mode_in_column_group;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_TABLE_BODY] = myhtml_insertion_mode_in_table_body;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_ROW] = myhtml_insertion_mode_in_row;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_CELL] = myhtml_insertion_mode_in_cell;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_SELECT] = myhtml_insertion_mode_in_select;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_SELECT_IN_TABLE] = myhtml_insertion_mode_in_select_in_table;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_TEMPLATE] = myhtml_insertion_mode_in_template;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_BODY] = myhtml_insertion_mode_after_body;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_IN_FRAMESET] = myhtml_insertion_mode_in_frameset;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_FRAMESET] = myhtml_insertion_mode_after_frameset;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_AFTER_BODY] = myhtml_insertion_mode_after_after_body;
    myhtml->insertion_func[MyHTML_INSERTION_MODE_AFTER_AFTER_FRAMESET] = myhtml_insertion_mode_after_after_frameset;
}



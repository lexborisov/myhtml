/*
 Copyright (C) 2016 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#include <stdio.h>
#include <stdlib.h>

#include <myhtml/api.h>

int main(int argc, const char * argv[])
{
    char html[] = "<div value='1'>one<span>Best of Fragments</span><a>click to make happy</a></div><div value='2'>two</div><div value='3'>three</div>";
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // init tree
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    // parse html
    myhtml_parse(tree, MyHTML_ENCODING_UTF_8, html, (sizeof(html) - 1));
    
    myhtml_tag_index_t *tag_index = myhtml_tree_get_tag_index(tree);
    myhtml_tag_index_node_t *index_node = myhtml_tag_index_first(tag_index, MyHTML_TAG_DIV);
    
    printf("Count of DIV nodes in index: %zu\n", myhtml_tag_index_entry_count(tag_index, MyHTML_TAG_DIV));
    printf("Get all DIV nodes from index:\n");
    
    while (index_node) {
        // print node
        myhtml_tree_print_node(tree, myhtml_tag_index_tree_node(index_node), stdout);
        
        // get next node from index
        index_node = myhtml_tag_index_next(index_node);
    }
    
    // release resources
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return 0;
}





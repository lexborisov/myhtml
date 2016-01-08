/*
 Copyright 2016 Alexander Borisov
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 
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
    myhtml_parse(tree, html, (sizeof(html) - 1));
    
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





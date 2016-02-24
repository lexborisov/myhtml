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
#include <string.h>
#include <myhtml/api.h>


int main(int argc, const char * argv[])
{
    char html[] = "<div></div>";
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // init tree
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    // parse html
    myhtml_parse_fragment(tree, MyHTML_ENCODING_UTF_8, html, strlen(html), MyHTML_TAG_DIV, MyHTML_NAMESPACE_HTML);
    
    // get first DIV from index
    myhtml_tag_index_t *tag_index = myhtml_tree_get_tag_index(tree);
    myhtml_tag_index_node_t *index_node = myhtml_tag_index_first(tag_index, MyHTML_TAG_DIV);
    
    myhtml_tree_node_t *node = myhtml_tag_index_tree_node(index_node);
    
    // print original tree
    printf("Original tree:\n");
    myhtml_tree_print_node_childs(tree, myhtml_tree_get_document(tree), stdout, 0);
    
    printf("For a test; Create and delete 100000 attrs...\n");
    for(size_t j = 0; j < 100000; j++) {
        myhtml_tree_attr_t *attr = myhtml_attribute_add(tree, node, "key", 3, "value", 5, MyHTML_ENCODING_UTF_8);
        myhtml_attribute_delete(tree, node, attr);
    }
    
    // add first attr in first div in tree
    myhtml_attribute_add(tree, node, "key", 3, "value", 5, MyHTML_ENCODING_UTF_8);
    
    printf("Modified tree:\n");
    myhtml_tree_print_node_childs(tree, myhtml_tree_get_document(tree), stdout, 0);
    
    // get attr by key name
    myhtml_tree_attr_t *gets_attr = myhtml_attribute_by_key(node, "key", 3);
    const char *attr_char = myhtml_attribute_value(gets_attr, NULL);
    
    printf("Get attr by key name \"key\": %s\n", attr_char);
    
    // release resources
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return 0;
}




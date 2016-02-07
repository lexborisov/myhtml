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
    char html[] = "<div>text for manipulate</div>";
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // init tree
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    // parse html
    myhtml_parse(tree, MyHTML_ENCODING_UTF_8, html, strlen(html));
    
    // print original tree
    printf("Original Tree:\n");
    myhtml_tree_print_node_childs(tree, myhtml_tree_get_document(tree), stdout, 0);
    
    printf("Change word: manipulate => test\n");
    
    // get text node by index
    myhtml_collection_t *collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG__TEXT, NULL);
    
    if(collection && collection->list && collection->length)
    {
        myhtml_tree_node_t *text_node = collection->list[0];
        myhtml_string_t *str = myhtml_node_string(text_node);
        
        // change data
        char *data = myhtml_string_data(str);
        
        for (size_t i = 0; i < myhtml_string_length(str); i++)
        {
            if(data[i] == 'm') {
                sprintf(&data[i], "test");
                
                // set new length
                myhtml_string_length_set(str, (i + 4));
                break;
            }
        }
    }
    
    printf("Changed Tree:\n");
    // print tree
    myhtml_tree_print_node_childs(tree, myhtml_tree_get_document(tree), stdout, 0);
    
    // release resources
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return 0;
}





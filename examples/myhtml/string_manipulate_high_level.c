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
#include <string.h>
#include <myhtml/api.h>

mystatus_t serialization_callback(const char* data, size_t len, void* ctx)
{
    printf("%.*s", (int)len, data);
    return MyCORE_STATUS_OK;
}

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
    myhtml_parse(tree, MyENCODING_UTF_8, html, strlen(html));
    
    // print original tree
    printf("Original Tree:\n");
    myhtml_serialization_tree_callback(myhtml_tree_get_node_html(tree), serialization_callback, NULL);
    
    printf("Change word: manipulate => test\n");
    
    // get text node by index
    myhtml_collection_t *collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG__TEXT, NULL);
    
    if(collection && collection->list && collection->length)
    {
        myhtml_tree_node_t *text_node = collection->list[0];
        mycore_string_t *str = myhtml_node_string(text_node);
        
        // change data
        char *data = mycore_string_data(str);
        
        for (size_t i = 0; i < mycore_string_length(str); i++)
        {
            if(data[i] == 'm') {
                sprintf(&data[i], "test");
                
                // set new length
                mycore_string_length_set(str, (i + 4));
                break;
            }
        }
    }
    
    printf("Changed Tree:\n");
    // print tree
    myhtml_serialization_tree_callback(myhtml_tree_get_node_html(tree), serialization_callback, NULL);
    
    // release resources
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return 0;
}





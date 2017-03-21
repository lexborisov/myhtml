/*
 Copyright (C) 2015-2016 Alexander Borisov
 
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
    char html[] = "<div></div>";
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // init tree
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    // parse html
    myhtml_parse_fragment(tree, MyENCODING_UTF_8, html, strlen(html), MyHTML_TAG_DIV, MyHTML_NAMESPACE_HTML);
    
    // get first DIV from index
    myhtml_collection_t *div_list = myhtml_get_nodes_by_name(tree, NULL, "div", 3, NULL);
    myhtml_tree_node_t *node = div_list->list[0];
    
    // print original tree
    printf("Original tree:\n");
    myhtml_serialization_tree_callback(myhtml_tree_get_node_html(tree), serialization_callback, NULL);
    
    printf("For a test; Create and delete 100000 attrs...\n");
    for(size_t j = 0; j < 100000; j++) {
        myhtml_tree_attr_t *attr = myhtml_attribute_add(node, "key", 3, "value", 5, MyENCODING_UTF_8);
        myhtml_attribute_delete(tree, node, attr);
    }
    
    // add first attr in first div in tree
    myhtml_attribute_add(node, "key", 3, "value", 5, MyENCODING_UTF_8);
    
    printf("Modified tree:\n");
    myhtml_serialization_tree_callback(myhtml_tree_get_node_html(tree), serialization_callback, NULL);
    
    // get attr by key name
    myhtml_tree_attr_t *gets_attr = myhtml_attribute_by_key(node, "key", 3);
    const char *attr_char = myhtml_attribute_value(gets_attr, NULL);
    
    printf("Get attr by key name \"key\": %s\n", attr_char);
    
    // release resources
    myhtml_collection_destroy(div_list);
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return 0;
}




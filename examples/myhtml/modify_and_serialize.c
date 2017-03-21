/**
 *  modify_and_serialize.c
 * 
 *  Test script that checks whether a document can be read, modified and
 *  serialized
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2016 Copernica BV
 */

/**
 *  Dependencies
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <myhtml/serialization.h>

/**
 *  Write output
 *  @param  buffer
 *  @param  size
 *  @param  ptr
 */
mystatus_t write_output(const char* data, size_t len, void* ctx)
{
    printf("%.*s", (int)len, data);
    return MyCORE_STATUS_OK;
}

/**
 *  Main procedure
 *  @return int
 */
int main()
{
    // initalize html engine
    myhtml_t *myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // init tree
    myhtml_tree_t *tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    // input string
    const char *input = "<html><head></head><body><a href=http://nu.nl></body></html>";
    
    // parse html
    myhtml_parse(tree, MyENCODING_UTF_8, input, strlen(input));

    // collection of links
    myhtml_collection_t *collection = myhtml_get_nodes_by_name(tree, NULL, "a", 1, NULL);
    
    // iterate over all nodes
    for (size_t i = 0; i < collection->length; ++i)
    {
        // add attribute
        myhtml_attribute_add(collection->list[i], "title", 5, "my value", 8, MyENCODING_UTF_8);
    }
    

    // write the document again
    myhtml_serialization_tree_callback(myhtml_tree_get_document(tree), write_output, NULL);
    
    myhtml_collection_destroy(collection);
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    // done
    return 0;
}


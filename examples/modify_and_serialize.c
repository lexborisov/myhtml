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
void write_output(const char *buffer, size_t size, void *ptr)
{
    fwrite(buffer, 1, size, stdout);
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
    myhtml_parse(tree, MyHTML_ENCODING_UTF_8, input, strlen(input));

    // collection of links
    myhtml_collection_t *collection = myhtml_get_nodes_by_name(tree, NULL, "a", 1, NULL);
    
    // iterate over all nodes
    for (size_t i = 0; i < collection->length; ++i)
    {
        // add attribute
        myhtml_attribute_add(tree, collection->list[i], "title", 5, "my value", 8, MyHTML_ENCODING_UTF_8);
    }
    

    // write the document again
    myhtml_serialize(tree, myhtml_tree_get_document(tree), MyHTML_TREE_SERIALIZE_FLAGS_FULL, write_output, NULL);

    /*
    
    // parse html
    myhtml_collection_t *collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_TITLE, NULL);
    
    if(collection && collection->list && collection->length) {
        myhtml_tree_node_t *text_node = myhtml_node_child(collection->list[0]);
        
        if(text_node) {
            const char* text = myhtml_node_text(text_node, NULL);
            
            if(text)
                printf("Title: %s\n", text);
        }
    }
    
    // release resources
    myhtml_collection_destroy(collection);
    * 
    * 
    */
    myhtml_collection_destroy(collection);
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    // done
    return 0;
}


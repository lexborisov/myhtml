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
#include <stdbool.h>
#include <memory.h>

#include <myhtml/api.h>

#include "example.h"

struct res_html {
    char  *html;
    size_t size;
};

struct res_argv {
    bool is_insensitive;
    
    const char* key;
    size_t key_length;
    
    size_t search_type;
};

struct res_html load_html_file(const char* filename)
{
    FILE *fh = fopen(filename, "rb");
    if(fh == NULL) {
        fprintf(stderr, "Can't open html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    if(fseek(fh, 0L, SEEK_END) != 0) {
        fprintf(stderr, "Can't set position (fseek) in file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    long size = ftell(fh);
    
    if(fseek(fh, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "Can't set position (fseek) in file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    if(size <= 0) {
        fprintf(stderr, "Can't get file size or file is empty: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    char *html = (char*)malloc(size + 1);
    if(html == NULL) {
        fprintf(stderr, "Can't allocate mem for html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    size_t nread = fread(html, 1, size, fh);
    if (nread != size) {
        fprintf(stderr, "could not read %ld bytes (" MyCORE_FMT_Z " bytes done)\n", size, nread);
        exit(EXIT_FAILURE);
    }
    
    fclose(fh);
    
    struct res_html res = {html, (size_t)size};
    return res;
}

void print_usage(void)
{
    printf("Bad ARGV!\nUse:\n");
    printf("\tnodes_by_attr_value_high_level <path to html file> <search value> [--type ~= --key key_name -i]\n");
    printf("\t--type: One of: =, ~=, ^=, $=, *=, |=; Default: =; Optional\n");
    printf("\t--key: search value in key; Optional\n");
    printf("\t-i: search value in case-insensitive mode; any value; Optional\n");
}

struct res_argv get_argv(int len, int argc, const char ** argv)
{
    struct res_argv rargv;
    memset(&rargv, 0, sizeof(struct res_argv));
    
    while(len < argc)
    {
        if(strcmp("-i", argv[len]) == 0) {
            rargv.is_insensitive = true;
        }
        else if(strcmp("--key", argv[len]) == 0) {
            len++;
            
            if(len >= argc) {
                print_usage();
                exit(EXIT_FAILURE);
            }
            
            rargv.key = argv[len];
            rargv.key_length = strlen(argv[len]);
        }
        else if(strcmp("--type", argv[len]) == 0) {
            len++;
            
            if(len >= argc) {
                print_usage();
                exit(EXIT_FAILURE);
            }
            
            if(strcmp("=", argv[len]) == 0) {
                rargv.search_type = 0;
            }
            else if(strcmp("~=", argv[len]) == 0) {
                rargv.search_type = 1;
            }
            else if(strcmp("^=", argv[len]) == 0) {
                rargv.search_type = 2;
            }
            else if(strcmp("$=", argv[len]) == 0) {
                rargv.search_type = 3;
            }
            else if(strcmp("*=", argv[len]) == 0) {
                rargv.search_type = 4;
            }
            else if(strcmp("|=", argv[len]) == 0) {
                rargv.search_type = 5;
            }
            else {
                print_usage();
                exit(EXIT_FAILURE);
            }
        }
        else {
            print_usage();
            exit(EXIT_FAILURE);
        }
        
        len++;
    }
    
    return rargv;
}

mystatus_t serialization_callback(const char* data, size_t len, void* ctx)
{
    printf("%.*s", (int)len, data);
    return MyCORE_STATUS_OK;
}

int main(int argc, const char * argv[])
{
    const char* path;
    const char* attr_value;
    
    struct res_argv rargv;
    
    if(argc > 2) {
        path = argv[1];
        attr_value = argv[2];
        
        rargv = get_argv(3, argc, argv);
    }
    else {
        print_usage();
        exit(EXIT_FAILURE);
    }
    
    struct res_html res = load_html_file(path);
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // init tree
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    // parse html
    myhtml_parse(tree, MyENCODING_UTF_8, res.html, res.size);
    
    // get and print
    myhtml_collection_t* collection = NULL;
    
    switch (rargv.search_type) {
        case 0:
            collection = myhtml_get_nodes_by_attribute_value(tree, NULL, NULL, rargv.is_insensitive,
                                                             rargv.key, rargv.key_length,
                                                             attr_value, strlen(attr_value), NULL);
            break;
        case 1:
            collection = myhtml_get_nodes_by_attribute_value_whitespace_separated(tree, NULL, NULL, rargv.is_insensitive,
                                                                                  rargv.key, rargv.key_length,
                                                                                  attr_value, strlen(attr_value), NULL);
            break;
        case 2:
            collection = myhtml_get_nodes_by_attribute_value_begin(tree, NULL, NULL, rargv.is_insensitive,
                                                                   rargv.key, rargv.key_length,
                                                                   attr_value, strlen(attr_value), NULL);
            break;
        case 3:
            collection = myhtml_get_nodes_by_attribute_value_end(tree, NULL, NULL, rargv.is_insensitive,
                                                                 rargv.key, rargv.key_length,
                                                                 attr_value, strlen(attr_value), NULL);
            break;
        case 4:
            collection = myhtml_get_nodes_by_attribute_value_contain(tree, NULL, NULL, rargv.is_insensitive,
                                                                     rargv.key, rargv.key_length,
                                                                     attr_value, strlen(attr_value), NULL);
            break;
        case 5:
            collection = myhtml_get_nodes_by_attribute_value_hyphen_separated(tree, NULL, NULL, rargv.is_insensitive,
                                                                              rargv.key, rargv.key_length,
                                                                              attr_value, strlen(attr_value), NULL);
            break;
            
        default:
            print_usage();
            exit(EXIT_FAILURE);
    }
    
    if(collection) {
        for(size_t i = 0; i < collection->length; i++)
            myhtml_serialization_node_callback(collection->list[i], serialization_callback, NULL);
        
        printf("Total found: " MyCORE_FMT_Z "\n", collection->length);
    }
    
    myhtml_collection_destroy(collection);
    
    // release resources
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return 0;
}




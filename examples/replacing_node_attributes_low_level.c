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
 
 This example changes value for "href" attribute in all "A" tags
 before processing node token.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <myhtml/myhtml.h>
#include <myhtml/incoming.h>
#include <myhtml/utils.h>
#include <myhtml/serialization.h>

struct res_html {
    char  *html;
    size_t size;
};

struct res_context {
    char   *key;
    size_t key_length;
    
    char   *value;
    size_t  value_length;
};

struct res_html load_html_file(const char* filename)
{
    FILE *fh = fopen(filename, "rb");
    if(fh == NULL) {
        fprintf(stderr, "Can't open html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    fseek(fh, 0L, SEEK_END);
    long size = ftell(fh);
    fseek(fh, 0L, SEEK_SET);
    
    char *html = (char*)malloc(size + 1);
    if(html == NULL) {
        fprintf(stderr, "Can't allocate mem for html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    size_t nread = fread(html, 1, size, fh);
    if(nread != size) {
        fprintf(stderr, "could not read %ld bytes (%zu bytes done)\n", size, nread);
        exit(EXIT_FAILURE);
    }
    
    fclose(fh);
    
    if(size < 0) {
        size = 0;
    }
    
    struct res_html res = {html, (size_t)size};
    return res;
}

myhtml_tree_attr_t * replacing_find_attribute_by_key(myhtml_tree_t* tree, myhtml_token_node_t* token, const char *key, size_t key_len)
{
    myhtml_tree_attr_t* attr = token->attr_first;
    
    while (attr) {
        myhtml_incoming_buffer_t *inc_buf = myhtml_incoming_buffer_find_by_position(tree->incoming_buf_first, attr->raw_key_begin);
        size_t relative_begin = myhtml_incoming_buffer_relative_begin(inc_buf, attr->raw_key_begin);
        
        if(key_len == attr->raw_key_length &&
           myhtml_strncasecmp(&inc_buf->data[relative_begin], key, key_len) == 0)
        {
            return attr;
        }
        
        attr = attr->next;
    }
    
    return NULL;
}

long replacing_change_attribute_value(myhtml_tree_t* tree, myhtml_tree_attr_t* attr, struct res_context *change_data)
{
    myhtml_incoming_buffer_t *next_inc_buf = myhtml_incoming_buffer_split(tree->incoming_buf, tree->mcobject_incoming_buf, attr->raw_value_begin);
    myhtml_incoming_buffer_t *new_inc_buf  = myhtml_incoming_buffer_add(tree->incoming_buf, tree->mcobject_incoming_buf,
                                                                        change_data->value, change_data->value_length);
    
    next_inc_buf->prev->next = new_inc_buf;
    next_inc_buf->offset = new_inc_buf->offset + new_inc_buf->size;
    
    new_inc_buf->next = next_inc_buf;
    new_inc_buf->prev = next_inc_buf->prev;
    new_inc_buf->length = new_inc_buf->size;
    
    next_inc_buf->prev = new_inc_buf;
    attr->raw_value_length = change_data->value_length;
    
    attr = attr->next;
    while (attr) {
        attr->raw_key_begin += change_data->value_length;
        attr->raw_value_begin += change_data->value_length;
        
        attr = attr->next;
    }
    
    tree->global_offset += change_data->value_length;
    tree->incoming_buf = next_inc_buf;
    
    return change_data->value_length;
}

void * replacing_callback_before_token_done(myhtml_tree_t* tree, myhtml_token_node_t* token, void* ctx)
{
    struct res_context *change_data = (struct res_context*)ctx;
    
    switch (token->tag_id) {
        case MyHTML_TAG_A: {
            myhtml_tree_attr_t *needed_attr = replacing_find_attribute_by_key(tree, token, change_data->key,
                                                                              change_data->key_length);
            
            if(needed_attr) {
                long diff = replacing_change_attribute_value(tree, needed_attr, ctx);
                token->raw_begin += diff;
            }
            
            break;
        }
            
        default:
            break;
    }
    
    return ctx;
}

int main(int argc, const char * argv[])
{
    const char* path;
    
    if (argc == 2) {
        path = argv[1];
    }
    else {
        printf("Bad ARGV!\nUse: replacing_node_attributes_low_level <path_to_html_file>\n");
        exit(EXIT_FAILURE);
    }
    
    struct res_html res = load_html_file(path);
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_PARSE_MODE_SINGLE, 1, 0);
    
    // init tree
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    // set callback and changes data
    struct res_context ctx = {"href", 4, "http://www.cpan.org/", strlen("http://www.cpan.org/")};
    myhtml_callback_before_token_done_set(tree, replacing_callback_before_token_done, &ctx);
    
    // parse html
    myhtml_parse(tree, MyHTML_ENCODING_UTF_8, res.html, res.size);
    
    // display result
    myhtml_string_raw_t str = {0};
    
    myhtml_serialization(tree, tree->node_html, &str);
    printf("%s", str.data);
    
    myhtml_string_raw_destroy(&str, false);
    
    printf("\n");
    
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    free(res.html);
    
    return 0;
}



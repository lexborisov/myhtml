/*
 Copyright (C) 2015-2016 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#include "main.h"
#include "dirent.h"
#include <time.h>
#include <sys/stat.h>

#include <stdbool.h>



#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>


static myhtml_tree_t* global_tree;

#define total_count_size 20
static size_t total_count[total_count_size];

typedef void (*process_state_f)(const char* data, size_t filename_size);
typedef void (*parser_state_f)(const char* data, size_t filename_size, size_t count);

void print_total_count(void)
{
    size_t total = 0;
    for(size_t i = 0; i < 7; i++)
        total += total_count[i];
    
    printf("Total: %zu\n" ,total);
    
    printf("\t0-100: %zu\n", total_count[0]);
    printf("\t100-1000: %zu\n", total_count[1]);
    printf("\t1000-5000: %zu\n", total_count[2]);
    printf("\t5000-10000: %zu\n", total_count[3]);
    printf("\t10000-50000: %zu\n", total_count[4]);
    printf("\t50000-100000: %zu\n", total_count[5]);
    printf("\t100000 and up: %zu\n", total_count[6]);
}

void listdir(const char *name, process_state_f callback)
{
    memset(total_count, 0, sizeof(size_t) * total_count_size);
    
    DIR *dir;
    struct dirent *entry;
    
    if(!(dir = opendir(name)))
        return;
    if(!(entry = readdir(dir)))
        return;
    
    do {
        if(entry->d_type == DT_DIR) {
            char path[2048];
            
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = '\0';
            
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            
            listdir(path, callback);
        }
        else {
            char path[2048];
            
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = '\0';
            
            if(path[ (len - 3) ] == '.' && path[ (len - 2) ] == 'g' && path[ (len - 1) ] == 'z') {
                callback(path, len);
            }
        }
    }
    while ((entry = readdir(dir)));
    
    closedir(dir);
}

void read_loaded(const char *filename, const char *db_dir, process_state_f callback)
{
    memset(total_count, 0, sizeof(size_t) * total_count_size);
    
    FILE *fh = fopen(filename, "rb");
    if(fh == NULL) {
        fprintf(stderr, "Can't open html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    fseek(fh, 0L, SEEK_END);
    long size = ftell(fh);
    fseek(fh, 0L, SEEK_SET);
    
    char *data = (char*)malloc(size + 1);
    if(data == NULL) {
        fprintf(stderr, "Can't allocate mem for html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    size_t nread = fread(data, 1, size, fh);
    if (nread != size) {
        fprintf(stderr, "could not read %ld bytes (%zu bytes done)\n", size, nread);
        exit(EXIT_FAILURE);
    }
    
    fclose(fh);
    
    if(size < 0)
        size = 0;
    
    size_t from = 0;
    char path[2048];
    
    for(size_t i = 0; i < size; i++) {
        if(data[i] == '\n') {
            int len = snprintf(path, sizeof(path)-1, "%s/%.*s", db_dir, (int)(i - from), &data[from]);
            path[len] = '\0';
            
            callback(path, len);
            
            from = i + 1;
        }
    }
    
    free(data);
}

void process(const char* filename, size_t filename_size, parser_state_f parser)
{
    FILE *fh = fopen(filename, "rb");
    if(fh == NULL) {
        fprintf(stderr, "Can't open html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    fseek(fh, 0L, SEEK_SET);
    
    const char *ct = "Content-Length:";
    size_t ct_size = strlen(ct);
    
    char * line = NULL;
    long get_size = 0;
    ssize_t read = 0;
    
    size_t count = 0, read_len = 0;
    
    while ((read = getline(&line, &read_len, fh)) != -1) {
        
        if(strncmp(ct, line, ct_size) == 0) {
            size_t i;
            
            for(i = ct_size; i < read_len; i++)
                if(line[i] != '\n' && line[i] != '\r' && line[i] != ' ')
                    break;
            
            get_size = strtol(&line[i], NULL, 0);
        }
        else if(get_size && line[0] == '\r' && line[1] == '\n') {
            long head_begin = ftell(fh) + 2;
            long end = head_begin + get_size;
            
            while ((read = getline(&line, &read_len, fh)) != -1) {
                //printf("%.*s", (int)read_len, line);
                
                if(line[0] == '\r' && line[1] == '\n')
                    break;
            }
            
            long head_end = ftell(fh);
            
            size_t html_length = (end - head_end);
            char *html = malloc(html_length + 1);
            
            size_t nread = fread(html, 1, html_length, fh);
            if (nread != html_length) {
                fprintf(stderr, "could not read %ld bytes (%zu bytes done)\n", html_length, nread);
                exit(EXIT_FAILURE);
            }
            
            count++;
            parser(html, html_length, count);
            
            get_size = 0;
            free(html);
        }
    }
    
    fclose(fh);
}

void html_parser(const char* html, size_t html_length, size_t count)
{
    if((count % 1000) == 0) {
        printf("\t%zu\n", count);
    }
    
    myhtml_encoding_t encoding = 0;
    //myhtml_encoding_detect(html, html_length, &encoding);
    
    // parse html
    myhtml_status_t status = myhtml_parse(global_tree, encoding, html, html_length);
    if(status != MyHTML_STATUS_OK) {
        fprintf(stderr, "Can't parse:\n%.*s\n", (int)html_length, html);
        exit(EXIT_FAILURE);
    }
    
    if(html_length < 100)
        total_count[0]++;
    else if(html_length >= 100 && html_length < 1000)
        total_count[1]++;
    else if(html_length >= 1000 && html_length < 5000)
        total_count[2]++;
    else if(html_length >= 5000 && html_length < 10000)
        total_count[3]++;
    else if(html_length >= 10000 && html_length < 50000)
        total_count[4]++;
    else if(html_length >= 50000 && html_length < 100000)
        total_count[5]++;
    else if(html_length >= 100000)
        total_count[6]++;
    
    //myhtml_tree_print_node_children(global_tree, global_tree->document, stdout, 0);
}

void process_unpack(const char* filename, size_t filename_size)
{
    char command[2048];
    snprintf(command, sizeof(command)-1, "gzip -k -d %s", filename);
    
    printf("Unzip %s\n", filename);
    
    system(command);
    
    char new_path[2048];
    size_t new_path_size = (filename_size - 3);
    
    snprintf(new_path, sizeof(new_path)-1, "%.*s", (int)new_path_size, filename);
    
    printf("Process %s:\n", new_path);
    process(new_path, new_path_size, html_parser);
    printf("\n");
    
    unlink(new_path);
}

//int main(int argc, const char * argv[])
//{
//    // basic init
//    myhtml_t* myhtml = myhtml_create();
//    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
//    
//    // tree init
//    global_tree = myhtml_tree_create();
//    myhtml_tree_init(global_tree, myhtml);
//    
//    listdir("/Volumes/ssd/db/crawl-data", process_unpack);
//    //read_loaded("/Volumes/ssd/db/loaded.paths", "/Volumes/ssd/db", process_unpack);
//    
//    //char *file = "/Volumes/ssd/db/crawl-data/CC-MAIN-2016-07/segments/1454701145519.33/warc/CC-MAIN-20160205193905-00064-ip-10-236-182-209.ec2.internal.warc.gz";
//    //process_unpack(file, strlen(file));
//    
//    // release resources
//    myhtml_tree_destroy(global_tree);
//    myhtml_destroy(myhtml);
//    
//    print_total_count();
//    
//    return 0;
//}












struct res_html {
    char *html;
    size_t size;
};

struct res_html_test {
    char *html;
    size_t len;
    size_t size;
};

struct chunk_res_test {
    struct res_html_test data;
    struct res_html_test result;
};

struct chunk_res_result {
    myhtml_tree_t *tree;
    size_t good;
    size_t bad;
    
    bool is_fragment;
    bool enabled_script;
    myhtml_tag_id_t tag_id;
    enum myhtml_namespace ns;
};

typedef void (*chunk_test_f)(struct chunk_res_test *res_test, struct chunk_res_result *result);

struct res_html load_html(const char* filename)
{
    FILE *f = fopen(filename, "rb");
    fseek(f, 0L, SEEK_END);
    
    long l = ftell(f);
    fseek(f, 0L, SEEK_SET);
    
    char *html = (char*)myhtml_malloc(l);
    fread(html, 1, l, f);
    
    fclose(f);
    
    struct res_html res = {html, (size_t)l};
    return res;
}

void chunk_test(void)
{
    struct res_html res = load_html("/new/C-git/myhtml/test/html/chunk.data");
    //struct res_html res = load_html("/new/C-git/myhtml/test/broken.html");
    
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    myhtml_encoding_set(tree, MyHTML_ENCODING_UTF_8);
    
    size_t begin = 0, i = 0;
    while (i < res.size)
    {
        if(res.html[i] == '\n') {
            //printf("Parse chunk: %.*s\n", (int)(i - begin), &res.html[begin]);
            myhtml_parse_chunk(tree, &res.html[begin], (i - begin));
            myhtml_tokenizer_wait(tree);
            
            begin = i + 1;
        }
        
        i++;
    }
    
    myhtml_parse_chunk(tree, &res.html[begin], (i - begin));
    myhtml_parse_chunk_end(tree);
    
    i = 0;
    while (i < res.size)
    {
        if(res.html[i] != '\n') {
            printf("%c", res.html[i]);
        }
        
        i++;
    }
    printf("\n");
    
    myhtml_tree_print_node_children(tree, tree->document, stdout, 0);
    
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    free(res.html);
}

/* For a test */
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
    if (nread != size) {
        fprintf(stderr, "could not read %ld bytes (%zu bytes done)\n", size, nread);
        exit(EXIT_FAILURE);
    }
    
    fclose(fh);
    
    if(size < 0)
        size = 0;
        
        struct res_html res = {html, (size_t)size};
        return res;
}

size_t read_line(char *html, size_t size)
{
    size_t len;
    for (len = 0; len < size; len++) {
        if(html[len] == '\n')
            break;
    }
    
    return len;
}

void init_res_test_data(struct res_html_test* data)
{
    data->len  = 0;
    data->size = 4096;
    data->html = malloc(sizeof(char) * data->size);
    
    if(data->html == NULL) {
        fprintf(stderr, "Can't init resurces for test object\n");
        exit(EXIT_FAILURE);
    }
}

void init_res_test(struct chunk_res_test* res_test)
{
    init_res_test_data(&res_test->data);
    init_res_test_data(&res_test->result);
}

void clean_res_test(struct chunk_res_test* res_test)
{
    res_test->data.len   = 0;
    res_test->result.len = 0;
}

void append_to_test(struct res_html_test* data, const char *text, size_t len, bool add_newline, bool to_lowercase)
{
    if(text == NULL)
        return;
    
    if((data->len + len + 2) >= data->size) {
        data->size = data->size + len + 2048;
        data->html = realloc(data->html, data->size);
        
        if(data->html == NULL) {
            fprintf(stderr, "Can't realloc resurces for test object\n");
            exit(EXIT_FAILURE);
        }
    }
    
    if(add_newline && data->len) {
        data->html[data->len] = '\n';
        data->len++;
    }
    
    memcpy(&data->html[data->len], text, len);
    
    if(to_lowercase) {
        for(size_t i = data->len; i < (data->len + len); i++) {
            if(data->html[i] > 0x40 && data->html[i] < 0x5b)
                data->html[i] |= 0x60;
        }
    }
    
    data->len += len;
    data->html[data->len] = '\0';
}

bool sort_text(myhtml_tree_attr_t *attr1, size_t size, myhtml_tree_attr_t **list, size_t i, size_t *len)
{
    unsigned char *fisrt = (unsigned char*)attr1->key.data;
    unsigned char *sec = (unsigned char*)list[i]->key.data;
    
    for (size_t j = 0; j < size; j++)
    {
        if(fisrt[j] > sec[j]) {
            break;
        }
        else if(fisrt[j] < sec[j]) {
            memmove(&list[(i + 1)], &list[i], sizeof(myhtml_tree_attr_t*));
            list[i] = attr1;
            (*len)++;
            return true;
        }
    }
    
    return false;
}

myhtml_tree_attr_t ** sort_attr(myhtml_tree_node_t *node)
{
    myhtml_tree_attr_t *attr = myhtml_node_attribute_first(node);
    myhtml_tree_attr_t **list = calloc(1024, sizeof(myhtml_tree_attr_t*));
    size_t len = 1;
    size_t i;
    
    list[0] = attr;
    attr = attr->next;
    
    while (attr) {
        for (i = 0; i < len; i++) {
            if(attr->key.length > list[i]->key.length)
            {
                if(sort_text(attr, list[i]->key.length, list, i, &len))
                    break;
            }
            else {
                if(sort_text(attr, attr->key.length, list, i, &len))
                    break;
            }
        }
        
        if(i == len) {
            list[i] = attr;
            len++;
        }
        
        attr = attr->next;
    }
    
    return list;
}

void print_node_attr(myhtml_tree_node_t *node, struct res_html_test* data, size_t inc)
{
    if(myhtml_node_attribute_first(node) == NULL)
        return;
    
    myhtml_tree_attr_t **list_attr = sort_attr(node);
    
    size_t len = 0;
    while (*list_attr != NULL)
    {
        myhtml_tree_attr_t *attr = *list_attr;
        
        const char *name = myhtml_attribute_key(attr, &len);
        
        if(name) {
            append_to_test(data, "\n", 1, false, false);
            
            for(size_t i = 0; i < inc; i++)
                append_to_test(data, "  ", 2, false, false);
            
            if(attr->ns == MyHTML_NAMESPACE_XML) {
                append_to_test(data, "xml ", 4, false, false);
            }
            else if(attr->ns == MyHTML_NAMESPACE_XMLNS) {
                append_to_test(data, "xmlns ", 6, false, false);
            }
            else if(attr->ns == MyHTML_NAMESPACE_XLINK) {
                append_to_test(data, "xlink ", 6, false, false);
            }
            
            append_to_test(data, name, len, false, false);
            
            const char *value = myhtml_attribute_value(attr, &len);
            
            if(value) {
                append_to_test(data, "=\"", 2, false, false);
                append_to_test(data, value, len, false, false);
                append_to_test(data, "\"", 1, false, false);
            }
            else {
                append_to_test(data, "=\"\"", 3, false, false);
            }
        }
        
        //attr = myhtml_attribute_next(attr);
        list_attr++;
    }
}

void print_tree(myhtml_tree_t* tree, myhtml_tree_node_t *node, struct res_html_test* data, size_t inc)
{
    while (node)
    {
        if(data->len)
            append_to_test(data, "\n", 1, false, false);
        
        for(size_t i = 0; i < inc; i++)
            append_to_test(data, "  ", 2, false, false);
        
        size_t len = 0;
        
        if(myhtml_node_tag_id(node) == MyHTML_TAG__TEXT)
        {
            const char *text = myhtml_node_text(node, &len);
            
            append_to_test(data, "\"", 1, false, false);
            append_to_test(data, text, len, false, false);
            append_to_test(data, "\"", 1, false, false);
        }
        else if(myhtml_node_tag_id(node) == MyHTML_TAG__COMMENT)
        {
            const char *text = myhtml_node_text(node, &len);
            
            append_to_test(data, "<!-- ", 5, false, false);
            append_to_test(data, text, len, false, false);
            append_to_test(data, " -->", 4, false, false);
        }
        else if(myhtml_node_tag_id(node) == MyHTML_TAG__DOCTYPE)
        {
            append_to_test(data, "<!DOCTYPE", strlen("<!DOCTYPE"), false, false);
            
            if(tree->doctype.attr_name) {
                append_to_test(data, " ", 1, false, false);
                append_to_test(data, tree->doctype.attr_name, strlen(tree->doctype.attr_name), false, false);
            }
            
            if(tree->doctype.attr_public) {
                append_to_test(data, " \"", 2, false, false);
                append_to_test(data, tree->doctype.attr_public, strlen(tree->doctype.attr_public), false, false);
                append_to_test(data, "\"", 1, false, false);
            }
            
            if(tree->doctype.attr_system) {
                append_to_test(data, " \"", 2, false, false);
                append_to_test(data, tree->doctype.attr_system, strlen(tree->doctype.attr_system), false, false);
                append_to_test(data, "\"", 1, false, false);
            }
            
            append_to_test(data, ">", 1, false, false);
        }
        else {
            const char *tag_name = myhtml_tag_name_by_id(tree, myhtml_node_tag_id(node), &len);
            
            if(tag_name) {
                append_to_test(data, "<", 1, false, false);
                
                switch (myhtml_node_namespace(node)) {
                    case MyHTML_NAMESPACE_SVG:
                        append_to_test(data, "svg ", 4, false, false);
                        append_to_test(data, tag_name, len, false, false);
                        break;
                        
                    case MyHTML_NAMESPACE_MATHML:
                        append_to_test(data, "math ", 5, false, false);
                        append_to_test(data, tag_name, len, false, true);
                        break;
                        
                    default:
                        append_to_test(data, tag_name, len, false, false);
                        break;
                }
            }
            else
                // it can not be
                printf("<!something is wrong!");
            
//            if(myhtml_node_is_close_self(node))
//                append_to_test(data, " /", 2, false);
            
            append_to_test(data, ">", 1, false, false);
            
            if(myhtml_node_tag_id(node) == MyHTML_TAG_TEMPLATE &&
               myhtml_node_namespace(node) == MyHTML_NAMESPACE_HTML)
            {
                append_to_test(data, "\n", 1, false, false);
                
                for(size_t i = 0; i <= inc; i++)
                    append_to_test(data, "  ", 2, false, false);
                
                append_to_test(data, "content", 7, false, false);
                
                print_tree(tree, myhtml_node_child(node), data, (inc + 2));
                node = myhtml_node_next(node);
                
                continue;
            }
            
            // print node attributes
            print_node_attr(node, data, (inc + 1));
        }
        
        // print childs
        print_tree(tree, myhtml_node_child(node), data, (inc + 1));
        node = myhtml_node_next(node);
    }
}

void chunk_process(struct chunk_res_test *res_test, struct chunk_res_result *result)
{
    struct res_html_test data;
    init_res_test_data(&data);
    
    if(result->enabled_script)
        result->tree->flags |= MyHTML_TREE_FLAGS_SCRIPT;
    
    //printf("%.*s\n", res_test->data.len, res_test->data.html);
    
    if(result->is_fragment == false) {
        myhtml_parse_single(result->tree, MyHTML_ENCODING_UTF_8, res_test->data.html, res_test->data.len);
        print_tree(result->tree, result->tree->document->child, &data, 0);
    }
    else {
        myhtml_parse_fragment_single(result->tree, MyHTML_ENCODING_UTF_8, res_test->data.html, res_test->data.len, result->tag_id, result->ns);
        print_tree(result->tree, result->tree->document->child->child, &data, 0);
    }
    
    if(res_test->result.len != data.len) {
        result->bad++;
        
        printf("Error:\n");
        printf("Original raw:\n%.*s\n", (int)res_test->data.len, res_test->data.html);
        printf("Original:\n%s\n", res_test->result.html);
        printf("Result:\n%s\n", data.html);
    }
    else if(strcmp(res_test->result.html, data.html) == 0) {
        result->good++;
        printf("Done\n");
    }
    else {
        result->bad++;
        printf("Error with len ==:\n");
        printf("Original raw:\n%.*s\n", (int)res_test->data.len, res_test->data.html);
        printf("Original:\n%s\n", res_test->result.html);
        printf("Result:\n%s\n", data.html);
    }
    
    myhtml_tree_clean(result->tree);
    mythread_clean(result->tree->myhtml->thread);
    
    free(data.html);
}

void read_chunk(struct res_html* res, chunk_test_f func, struct chunk_res_result *result)
{
    struct chunk_res_test res_test;
    init_res_test(&res_test);
    
    result->is_fragment = false;
    
    size_t i = 0; size_t state = 0;
    while (i < res->size)
    {
        size_t line_len = read_line(&res->html[i], (res->size - i));
        
        if(line_len == 0 && state != 1) {
            if((i + 1) < res->size && res->html[(i + 1)] != '#') {
                append_to_test(&res_test.result, "\n", 1, true, false);
                i++;
                continue;
            }
            
            if(res_test.data.len)
                func(&res_test, result);
            
            state = 0;
            result->is_fragment = false;
            result->enabled_script = false;
            clean_res_test(&res_test);
        }
        else if(res->html[i] == '#') {
            
            if(line_len == strlen("#script-on") && strncmp(&res->html[i], "#script-on", strlen("#script-on")) == 0) {
                result->enabled_script = true;
            }
            else if(state == 0) {
                if(strncmp(&res->html[i], "#data", line_len) == 0) {
                    state = 1;
                }
            }
            else if(state == 1) {
                if(strncmp(&res->html[i], "#errors", line_len) == 0) {
                    state = 2;
                }
            }
            else if(state == 2 && line_len > 9) {
                if(strncmp(&res->html[i], "#document-fragment", line_len) == 0) {
                    state = 3;
                    result->is_fragment = true;
                }
            }
            else if(state == 2 || state == 3) {
                if(strncmp(&res->html[i], "#document", line_len) == 0) {
                    state = 4;
                }
            }
            else {
                fprintf(stderr, "Bad position in text\n");
                exit(EXIT_FAILURE);
            }
            
            i += line_len;
        }
        else if(state == 1) {
            append_to_test(&res_test.data, &res->html[i], line_len, true, false);
            i += line_len;
        }
        else if(state == 3) {
            const char *data = &res->html[i];
            size_t j;
            
            for(j = 0; j < line_len; j++)
            {
                if(data[j] == ' ')
                {
                    while(j < line_len) {
                        if(data[j] != ' ' && data[j] != '\n' && data[j] != '\t' && data[j] != '\r')
                            break;
                        
                        j++;
                    }
                    
                    break;
                }
            }
            
            if(j == line_len) {
                result->tag_id = myhtml_tag_id_by_name(result->tree, data, line_len);
                result->ns = MyHTML_NAMESPACE_HTML;
            }
            else {
                result->tag_id = myhtml_tag_id_by_name(result->tree, &data[j], (line_len - j));
                
                if(strncasecmp("math", data, 4) == 0) {
                    result->ns = MyHTML_NAMESPACE_MATHML;
                }
                else if(strncasecmp("svg", data, 3) == 0) {
                    result->ns = MyHTML_NAMESPACE_SVG;
                }
                else {
                    result->ns = MyHTML_NAMESPACE_HTML;
                }
            }
            
            i += line_len;
        }
        else if(state == 4) {
            if(res->html[i] == '|') {
                append_to_test(&res_test.result, &res->html[(i + 2)], (line_len - 2), true, false);
            }
            else if(res->html[i] != '\n') {
                append_to_test(&res_test.result, &res->html[i], line_len, true, false);
            }
            
            i += line_len;
        }
        else {
            i += line_len;
        }
        
        i++;
    }
}

void read_dir(const char* from_dir)
{
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    DIR *dir;
    struct dirent *ent;
    struct stat path_stat;
    
    size_t count = 0;
    size_t from_dir_len = strlen(from_dir);
    
    char path[4096];
    strncpy(path, from_dir, from_dir_len);
    
    if((dir = opendir(from_dir)) == NULL) {
        fprintf(stderr, "Can't open directory: %s\n", from_dir);
    }
    
    struct chunk_res_result result = {tree, 0, 0};
    
    while((ent = readdir(dir)) != NULL)
    {
        sprintf(&path[from_dir_len], "%s", ent->d_name);
        stat(path, &path_stat);
        
        if(ent->d_name[0] != '.' && !S_ISDIR(path_stat.st_mode) &&
           ent->d_namlen > 4 &&
           strcmp(".dat", &ent->d_name[ (ent->d_namlen - 4) ]) == 0)
        {
            count++;
            
            struct res_html res = load_html_file(path);
            read_chunk(&res, chunk_process, &result);
            free(res.html);
        }
    }
    
    closedir (dir);
    
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    printf("\nGood: %zu; Bad: %zu\n\n", result.good, result.bad);
}

void test_mchar_async(void)
{
    mchar_async_t *mchar = mchar_async_create(2, 128);
    size_t node_id = mchar_async_node_add(mchar);
    
    for (size_t i = 0; i < 10000; i++) {
        char *data = mchar_async_malloc(mchar, node_id, 132);
        
        for (size_t j = 0; j < 128; j++) {
            data[j] = 'a';
        }
        
        data = mchar_async_realloc(mchar, node_id, data, 128, 130);
        
        for (size_t j = 128; j < 130; j++) {
            data[j] = 'b';
        }
    }
    
    mchar_async_destroy(mchar, 1);
}


#define DIE(msg, ...) do { fprintf(stderr, msg, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)

static bool filter_node(myhtml_tree_node_t* node)
{
    myhtml_tag_id_t tag = myhtml_node_tag_id(node);
    return (tag != MyHTML_TAG__TEXT) && (tag != MyHTML_TAG__END_OF_FILE) && (tag != MyHTML_TAG__COMMENT) && (tag != MyHTML_TAG__UNDEF);
}

/* depth-first lefthand tree walk */
static void walk_subtree(myhtml_tree_t* tree, myhtml_tree_node_t* root, int level)
{
    if (!root) {
        return;
    }
    
    /* Check if we handle this node type */
    if (!filter_node(root)) {
        return;
    }
    
    /* start sexpr */
    putchar('(');
    
    /* print this node */
    //printf("%s", myhtml_tag_name_by_id(tree, myhtml_node_tag_id(root), NULL));
    myhtml_tree_attr_t* attr = myhtml_node_attribute_first(root);
    while (attr != NULL) {
        /* attribute sexpr (name value)*/
        //printf("(%s \'%s\')", myhtml_attribute_name(attr, NULL), myhtml_attribute_value(attr, NULL));
        attr = myhtml_attribute_next(attr);
    }
    
    /* left hand depth-first recoursion */
    myhtml_tree_node_t* child = myhtml_node_child(root);
    while (child != NULL) {
        walk_subtree(tree, child, level + 1);
        child = myhtml_node_next(child);
    }
    
    /* close sexpr */
    putchar(')');
}

static void usage(void)
{
    fprintf(stderr, "html2sexpr <file>\n");
}

int maindfdf()
{
    struct res_html data = load_html_file("/new/C-git/myhtml/bin/attributes_high_level");
    myhtml_status_t res;
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    res = myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // init tree
    myhtml_tree_t* tree = myhtml_tree_create();
    res = myhtml_tree_init(tree, myhtml);
    
    // parse html
    myhtml_parse(tree, MyHTML_ENCODING_UTF_8, data.html, data.size);
    
    walk_subtree(tree, myhtml_tree_get_node_html(tree), 0);
    printf("\n");
    
    // release resources
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    free(data.html);
    
    return EXIT_SUCCESS;
}

void test_all(void)
{
    setbuf(stdout, NULL);
    
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    DIR *dir;
    struct dirent *ent;
    struct stat path_stat;
    
    const char *from_dir = "/new/Test/html_files/";
    size_t from_dir_len = strlen(from_dir);
    
    char path[4096];
    strncpy(path, from_dir, from_dir_len);
    
    size_t count = 0;
    
    if((dir = opendir(from_dir)) != NULL)
    {
        while((ent = readdir(dir)) != NULL)
        {
            sprintf(&path[from_dir_len], "%s", ent->d_name);
            
            stat(path, &path_stat);
            
            if(ent->d_name[0] != '.' && !S_ISDIR(path_stat.st_mode))
            {
                count++;
                
                printf("%zu: %s\n", count, path);
                
                struct res_html res = load_html(path);
                
                myhtml_tree_t* tree = myhtml_tree_create();
                myhtml_tree_init(tree, myhtml);
                
                myhtml_parse(tree, MyHTML_ENCODING_UTF_8, res.html, res.size);
                
                myhtml_tree_destroy(tree);
                
                
                //                myhtml_tree_node_t **node_list = myhtml_get_elements_by_tag_id(tree, MyHTML_TAG_TITLE, NULL);
                //
                //                if(node_list && node_list[0])
                //                    if(node_list[0]->token)
                //                        myhtml_tree_print_by_tree_idx(tree, node_list[0]->child, stdout, 0);
                //
                //                myhtml_destroy_node_list(node_list);
                
                free(res.html);
                
            }
        }
        
        closedir (dir);
    }
    
    
    myhtml_destroy(myhtml);
}

void callback_node_insert(myhtml_tree_t* tree, myhtml_tree_node_t* node, void* ctx)
{
    printf("Insert: ");
    myhtml_tree_print_node(tree, node, stdout);
}

void callback_node_delete(myhtml_tree_t* tree, myhtml_tree_node_t* node, void* ctx)
{
    printf("Delete: ");
    myhtml_tree_print_node(tree, node, stdout);
}

void serialization_callback(const char* data, size_t len, void* ctx)
{
    printf("%.*s", (int)len, data);
}

void myhtml_print_text_with_recursion(myhtml_tree_t* tree, myhtml_tree_node_t* scope_node)
{
    if(scope_node->tag_id == MyHTML_TAG__TEXT) {
        const char* text = myhtml_node_text(scope_node, NULL);
        printf("%s", text);
    }
    
    if(scope_node->child)
        myhtml_print_text_with_recursion(tree, scope_node->child);
    if(scope_node->next)
        myhtml_print_text_with_recursion(tree, scope_node->next);
}

int main(int argc, const char * argv[])
{
    setbuf(stdout, NULL);
    
//    const myhtml_tag_context_t *tag = myhtml_tag_static_search("div", 3);
//    
////    return 0;
//    read_dir("/new/C-git/html5lib-tests/tree-construction/");
//    return 0;
//    read_dir("/new/C-git/tree-construction/");
//    return 0;
//    read_dir("/new/C-git/html5lib-tests/custom/");
//    return 0;
//
    //myhtml_encoding_decode_single_byte(0xf4);

//    myhtml_encoding_result_t res_data = {0, 0, 0, 0, 0, 0};
//    
//    unsigned char data[20];
//    data[0] = 0xAC;
//    data[1] = 0xB1;
//    data[2] = 0x42;
//    data[3] = 0x4F;
//    data[4] = 0x51;
//    data[5] = 0x1B;
//    data[6] = 0x28;
//    data[7] = 0x42;
//    data[8] = 0;
//    
//    //myhtml_encoding_dec_to_char(54936, data);
//    
//    enum myhtml_encoding_status status = MyHTML_ENCODING_STATUS_CONTINUE;
//    
//    size_t i = 0;
//    while (status & MyHTML_ENCODING_STATUS_CONTINUE) {
//        status = myhtml_encoding_decode_euc_kr(data[i], &res_data);
//        i++;
//    }
//    
//    myhtml_string_convert_dec_to_ansi_utf8(res_data.result, (char *)data);
//    
//    return 0;
//
//    uint64_t all_start1 = myhtml_hperf_clock(NULL);
//    test_all();
//    uint64_t all_stop1 = myhtml_hperf_clock(NULL);
//
//    myhtml_hperf_print("Parse html", all_start1, all_stop1, stdout);
//    return 0;
//
    /* Default path or argument value */
//    const char* path = "/new/C-git/myhtml/bin/html2sexpr";
//    const char* path = "/new/Test/html_files/http-www.gameland.ru_magstorage_pg_075_pg_075.pdf.html";
//    const char* path = "/new/Test/html_files/http-msu.ru_projects_amv_doc_h1_1_1_2.doc.html";
//    const char* path = "/new/Test/html_files/http-5fan.ru_wievjob.php_id=16163.html";
//    const char* path = "/new/Test/html_files/http-www.unodc.org_documents_scientific_MLD-06-58676_Vol_2_ebook.pdf.html";
//    const char* path = "/new/C-git/broken.html";
    const char* path = "/new/C-git/test_full.html";
//    const char* path = "/new/C-git/scribble/temp/http___1tulatv.ru_2016_08_09_55575-pogoda-v-tule-na-10-avgusta.html.html";
//    const char* path = "/new/html_parsers/test_large_4.html";
    
    if (argc == 2) {
        path = argv[1];
    }
    
//    atomic_size_t fdf = 1;
//    while((atomic_fetch_and(&fdf, 4) & 4) == 0) {}
//    
//    printf("dfssf");
    
    setbuf(stdout, 0);
    struct res_html res  = load_html(path);
    
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
//    mythread_resume_all(myhtml->thread);
//    mythread_wait_all(myhtml->thread);
//    
//    mythread_stream_pause_all(myhtml->thread);
//    mythread_batch_pause_all(myhtml->thread);
//    
//    mythread_run_all(myhtml->thread, MyTHREAD_OPT_UNDEF);
//    mythread_stop_all(myhtml->thread);
    
    uint64_t all_start = myhtml_hperf_clock(NULL);
    uint64_t tree_init_start = myhtml_hperf_clock(NULL);
    
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
//    tree->callback_tree_node_insert = callback_node_insert;
//    tree->callback_tree_node_remove = callback_node_delete;
    
//    myhtml_tree_parse_flags_set(tree, MyHTML_TREE_PARSE_FLAGS_WITHOUT_PROCESS_TOKEN);
    
//    
//    myhtml_encoding_t encoding;
//    myhtml_encoding_detect(res.html, res.size, &encoding);
    
    uint64_t tree_init_stop = myhtml_hperf_clock(NULL);
    uint64_t parse_start = myhtml_hperf_clock(NULL);
    
    for(size_t i = 0; i < 1; i++)
    {
        //tree->flags |= MyHTML_TREE_FLAGS_SCRIPT;
        myhtml_parse(tree, MyHTML_ENCODING_UTF_8, res.html, res.size);
        
//        myhtml_parse_fragment(tree, MyHTML_ENCODING_UTF_8, res.html, res.size, MyHTML_TAG_ANNOTATION_XML, MyHTML_NAMESPACE_MATHML);
        
//        myhtml_tree_print_node(tree, tree->document->child, stdout);
        
//        myhtml_collection_t *collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_CODE, NULL);
        
//        myhtml_string_raw_t str_raw;
//        if(myhtml_serialization(tree, tree->document, &str_raw)) {
//            printf("%s", str_raw.data);
//        }
        
//        myhtml_tree_print_node_children(tree, tree->document, stdout, 0);
    }
    
//    myhtml_collection_t *coll_p = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_A, NULL);
//    for (size_t i = 0; i < coll_p->length; i++) {
//        myhtml_tree_node_t *node = coll_p->list[i];
//        //if(node)
//            printf("Text in <P> element #%zu (%zu:%zu):\n", i, node->token->element_begin, node->token->raw_begin);
//        
//        //myhtml_print_text_with_recursion(tree, coll_p->list[i]);
//    }
    
//    myhtml_collection_t *all_text_nodes = NULL;
//    
//    for (size_t i = 0; i < coll_p->length; i++) {
//        all_text_nodes = myhtml_get_nodes_by_tag_id_in_scope(tree, all_text_nodes, coll_p->list[i], MyHTML_TAG__TEXT, NULL);
//        
//        printf("Text in <P> element #%zu:\n", i);
//        for (size_t j = 0; j < all_text_nodes->length; j++) {
//            const char* text = myhtml_node_text(all_text_nodes->list[j], NULL);
//            printf("%s", text);
//        }
//        
//        myhtml_collection_clean(all_text_nodes);
//    }
    
//    myhtml_collection_destroy(coll_p);
//    myhtml_collection_destroy(all_text_nodes);
    
//    myhtml_collection_t *tables = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_TABLE, NULL);
//    myhtml_collection_t *trs = NULL, *a_list = NULL, *strong_list = NULL;
//    
//    for(size_t i = 0; i < tables->length; i++) {
//        trs = myhtml_get_nodes_by_tag_id_in_scope(tree, trs, tables->list[i], MyHTML_TAG_TR, NULL);
//        
//        for(size_t t = 0; t < trs->length; t++) {
//            a_list = myhtml_get_nodes_by_tag_id_in_scope(tree, a_list, trs->list[t], MyHTML_TAG_TH, NULL);
//            
//            for(size_t ai = 0; ai < a_list->length; ai++) {
//                strong_list = myhtml_get_nodes_by_tag_id_in_scope(tree, strong_list, a_list->list[ai], MyHTML_TAG_P, NULL);
//                
//                if(strong_list->length && myhtml_node_child(strong_list->list[0])) {
//                    const char* name = myhtml_node_text(myhtml_node_child(strong_list->list[0]), NULL);
//                    printf("%s\n", name);
//                }
//                
//                myhtml_collection_clean(strong_list);
//            }
//            
//            myhtml_collection_clean(a_list);
//        }
//        
//        printf("Row Count: %zu\n", trs->length);
//        myhtml_collection_clean(trs);
//    }
//    
//    // release resources
//    myhtml_collection_destroy(strong_list);
//    myhtml_collection_destroy(a_list);
//    myhtml_collection_destroy(trs);
//    myhtml_collection_destroy(tables);
    
    //myhtml_serialization_tree_callback(tree, tree->node_html, serialization_callback, NULL);
    
    uint64_t parse_stop = myhtml_hperf_clock(NULL);
    uint64_t all_stop = myhtml_hperf_clock(NULL);
    
//    myhtml_tree_node_t *node = myhtml_tree_get_node_body(tree);
//    
//    printf("For a test; Create and delete 100000 attrs...\n");
//    for(size_t j = 0; j < 100000; j++) {
//        myhtml_tree_attr_t *attr = myhtml_attribute_add(tree, node, "key", 3, "value", 5, MyHTML_ENCODING_UTF_8);
//        myhtml_attribute_delete(tree, node, attr);
//    }
//    
//
//    
//    // add first attr in first div in tree
//    myhtml_attribute_add(tree, node, "key", 3, "value", 5, MyHTML_ENCODING_UTF_8);
//    
//    myhtml_tree_attr_t *gets_attr = myhtml_attribute_by_key(node, "key", 3);
//    const char *attr_char = myhtml_attribute_value(gets_attr, NULL);
//    
//    printf("Get attr by key name \"key\": %s\n", attr_char);
    
    
    printf("\n\nInformation:\n");
    printf("Timer (%llu ticks/sec):\n", (unsigned long long) myhtml_hperf_res(NULL));
    myhtml_hperf_print("\tFirst Tree init", tree_init_start, tree_init_stop, stdout);
    myhtml_hperf_print("\tParse html", parse_start, parse_stop, stdout);
    myhtml_hperf_print("\tTotal", all_start, all_stop, stdout);
    printf("\n");
    
//    usleep(121212121212);
    
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    free(res.html);
    
    return 0;
}


//
///*
// Copyright (C) 2016 Alexander Borisov
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
// 
// Author: lex.borisov@gmail.com (Alexander Borisov)
// 
// This example changes value for "href" attribute in all "A" tags
// before processing node token.
// */
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#include <myhtml/myhtml.h>
//#include <myhtml/incoming.h>
//#include <myhtml/utils.h>
//#include <myhtml/serialization.h>
//
//struct res_html {
//    char  *html;
//    size_t size;
//};
//
//struct res_context {
//    char   *key;
//    size_t key_length;
//    
//    char   *value;
//    size_t  value_length;
//};
//
//struct res_html load_html_file(const char* filename)
//{
//    FILE *fh = fopen(filename, "rb");
//    if(fh == NULL) {
//        fprintf(stderr, "Can't open html file: %s\n", filename);
//        exit(EXIT_FAILURE);
//    }
//    
//    fseek(fh, 0L, SEEK_END);
//    long size = ftell(fh);
//    fseek(fh, 0L, SEEK_SET);
//    
//    char *html = (char*)malloc(size + 1);
//    if(html == NULL) {
//        fprintf(stderr, "Can't allocate mem for html file: %s\n", filename);
//        exit(EXIT_FAILURE);
//    }
//    
//    size_t nread = fread(html, 1, size, fh);
//    if(nread != size) {
//        fprintf(stderr, "could not read %ld bytes (%zu bytes done)\n", size, nread);
//        exit(EXIT_FAILURE);
//    }
//    
//    fclose(fh);
//    
//    if(size < 0) {
//        size = 0;
//    }
//    
//    struct res_html res = {html, (size_t)size};
//    return res;
//}
//
//myhtml_tree_attr_t * replacing_find_attribute_by_key(myhtml_tree_t* tree, myhtml_token_node_t* token, const char *key, size_t key_len)
//{
//    myhtml_tree_attr_t* attr = token->attr_first;
//    
//    while (attr) {
//        myhtml_incoming_buffer_t *inc_buf = myhtml_incoming_buffer_find_by_position(tree->incoming_buf_first, attr->raw_key_begin);
//        size_t relative_begin = myhtml_incoming_buffer_relative_begin(inc_buf, attr->raw_key_begin);
//        
//        if(key_len == attr->raw_key_length &&
//           myhtml_strncasecmp(&inc_buf->data[relative_begin], key, key_len) == 0)
//        {
//            return attr;
//        }
//        
//        attr = attr->next;
//    }
//    
//    return NULL;
//}
//
//long replacing_change_attribute_value(myhtml_tree_t* tree, myhtml_tree_attr_t* attr, struct res_context *change_data)
//{
//    myhtml_incoming_buffer_t *next_inc_buf = myhtml_incoming_buffer_split(tree->incoming_buf, tree->mcobject_incoming_buf, attr->raw_value_begin);
//    myhtml_incoming_buffer_t *new_inc_buf  = myhtml_incoming_buffer_add(tree->incoming_buf, tree->mcobject_incoming_buf,
//                                                                        change_data->value, change_data->value_length);
//    
//    next_inc_buf->prev->next = new_inc_buf;
//    next_inc_buf->offset = new_inc_buf->offset + new_inc_buf->size;
//    
//    new_inc_buf->next = next_inc_buf;
//    new_inc_buf->prev = next_inc_buf->prev;
//    new_inc_buf->length = new_inc_buf->size;
//    
//    next_inc_buf->prev = new_inc_buf;
//    attr->raw_value_length = change_data->value_length;
//    
//    attr = attr->next;
//    while (attr) {
//        attr->raw_key_begin += change_data->value_length;
//        attr->raw_value_begin += change_data->value_length;
//        
//        attr = attr->next;
//    }
//    
//    tree->global_offset += change_data->value_length;
//    tree->incoming_buf = next_inc_buf;
//    
//    return change_data->value_length;
//}
//
//void * replacing_callback_before_token_done(myhtml_tree_t* tree, myhtml_token_node_t* token, void* ctx)
//{
//    struct res_context *change_data = (struct res_context*)ctx;
//    
//    switch (token->tag_id) {
//        case MyHTML_TAG_A: {
//            myhtml_tree_attr_t *needed_attr = replacing_find_attribute_by_key(tree, token, change_data->key,
//                                                                              change_data->key_length);
//            
//            if(needed_attr) {
//                long diff = replacing_change_attribute_value(tree, needed_attr, ctx);
//                token->raw_begin += diff;
//            }
//            
//            break;
//        }
//            
//        default:
//            break;
//    }
//    
//    return ctx;
//}
//
//int main(int argc, const char * argv[])
//{
//    const char* path = "/new/C-git/habr/test.html";
//    
//    struct res_html res = load_html_file(path);
//    
//    // basic init
//    myhtml_t* myhtml = myhtml_create();
//    myhtml_init(myhtml, MyHTML_OPTIONS_PARSE_MODE_SINGLE, 1, 0);
//    
//    // init tree
//    myhtml_tree_t* tree = myhtml_tree_create();
//    myhtml_tree_init(tree, myhtml);
//    
//    // set callback and changes data
//    struct res_context ctx = {"href", 4, "http://www.cpan.org/", strlen("http://www.cpan.org/")};
//    myhtml_callback_before_token_done_set(tree, replacing_callback_before_token_done, &ctx);
//    
//    // parse html
//    myhtml_parse(tree, MyHTML_ENCODING_UTF_8, res.html, res.size);
//    
//    // display result
//    myhtml_string_raw_t str = {0};
//    
//    myhtml_serialization(tree, tree->node_html, &str);
//    printf("%s", str.data);
//    
//    myhtml_string_raw_destroy(&str, false);
//    
//    printf("\n");
//    
//    myhtml_tree_destroy(tree);
//    myhtml_destroy(myhtml);
//    
//    free(res.html);
//    
//    return 0;
//}



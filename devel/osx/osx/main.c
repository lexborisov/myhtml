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
                
//                printf("%zu: %s\n", count, path);
                
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

int main(int argc, const char * argv[])
{
//    const myhtml_tag_context_t *tag = myhtml_tag_static_search("div", 3);
//    
////    return 0;
//    read_dir("/new/C-git/html5lib-tests/tree-construction/");
//    return 0;
//    read_dir("/new/tree-construction/");
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
    uint64_t all_start1 = myhtml_hperf_clock(NULL);
    test_all();
    uint64_t all_stop1 = myhtml_hperf_clock(NULL);

    myhtml_hperf_print("Parse html", all_start1, all_stop1, stdout);
    return 0;
//
    /* Default path or argument value */
//    const char* path = "/new/C-git/myhtml/bin/html2sexpr";
//    const char* path = "/new/Test/html_files/http-www.gameland.ru_magstorage_pg_075_pg_075.pdf.html";
//    const char* path = "/new/Test/html_files/http-msu.ru_projects_amv_doc_h1_1_1_2.doc.html";
//    const char* path = "/new/Test/html_files/http-5fan.ru_wievjob.php_id=16163.html";
//    const char* path = "/new/Test/html_files/http-www.unodc.org_documents_scientific_MLD-06-58676_Vol_2_ebook.pdf.html";
    const char* path = "/new/C-git/broken.html";
//    const char* path = "/new/C-git/test_full.html";
//    const char* path = "/new/C-git/test_full_utf_16.html";
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
    
//    myhtml_tree_parse_flags_set(tree, MyHTML_TREE_PARSE_FLAGS_WITHOUT_DOCTYPE_IN_TREE| MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN);
    
//    
//    myhtml_encoding_t encoding;
//    myhtml_encoding_detect(res.html, res.size, &encoding);
    
    uint64_t tree_init_stop = myhtml_hperf_clock(NULL);
    uint64_t parse_start = myhtml_hperf_clock(NULL);
    
    for(size_t i = 0; i < 1; i++)
    {
        myhtml_parse(tree, MyHTML_ENCODING_UTF_8, res.html, res.size);
        
//        myhtml_parse_fragment(tree, MyHTML_ENCODING_UTF_8, res.html, res.size, MyHTML_TAG_ANNOTATION_XML, MyHTML_NAMESPACE_MATHML);
        
//        myhtml_tree_print_node(tree, tree->document->child, stdout);
        
//        myhtml_tree_print_node_children(tree, tree->document, stdout, 0);
    }
    
    uint64_t parse_stop = myhtml_hperf_clock(NULL);
    uint64_t all_stop = myhtml_hperf_clock(NULL);
    
    myhtml_collection_t* collection = myhtml_get_nodes_by_attribute_value_contain(tree, NULL, NULL, 1,
                                                                          "class", 5,
                                                                          "count", 5, NULL);
    
    for(size_t i = 0; i < collection->length; i++)
        myhtml_tree_print_node(tree, collection->list[i], stdout);
    
    printf("");
    
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



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
 For HTML Pages from: http://commoncrawl.org/
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include <myhtml/api.h>

#define DIE(msg, ...) do { fprintf(stderr, msg, ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)

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

static void usage(void)
{
    fprintf(stderr, "commoncrawl <dir with *.warc.gz>\n");
}

int main(int argc, const char * argv[])
{
    if (argc != 2) {
        usage();
        DIE("Invalid number of arguments\n");
    }
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // first tree init
    global_tree = myhtml_tree_create();
    myhtml_tree_init(global_tree, myhtml);
    
    listdir(argv[1], process_unpack);
    
    // release resources
    myhtml_tree_destroy(global_tree);
    myhtml_destroy(myhtml);
    
    print_total_count();
    
    return 0;
}



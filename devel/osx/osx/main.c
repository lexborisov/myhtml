/*
 Copyright 2015 Alexander Borisov
 
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

#include "main.h"
#include "dirent.h"

struct res_html {
    char *html;
    size_t size;
};

struct res_html load_html(const char* filename)
{
    FILE *f = fopen(filename, "rb");
    fseek(f, 0L, SEEK_END);
    
    long l = ftell(f);
    fseek(f, 0L, SEEK_SET);
    
    char *html = (char*)mymalloc(l);
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
    myhtml_parse_end(tree);
    
    i = 0;
    while (i < res.size)
    {
        if(res.html[i] != '\n') {
            printf("%c", res.html[i]);
        }
        
        i++;
    }
    printf("\n");
    
    myhtml_tree_print_node_childs(tree, tree->document, stdout, 0);
    
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    free(res.html);
}

void test_all(void)
{
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    DIR *dir;
    struct dirent *ent;
    struct stat path_stat;
    
    const char *from_dir = "/new/Test/out/";
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
                myhtml_parse(tree, res.html, res.size);
                
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
    
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
}

int main(int argc, const char * argv[])
{
    chunk_test();
//    uint64_t all_start1 = myhtml_hperf_clock(NULL);
//    test_all();
//    uint64_t all_stop1 = myhtml_hperf_clock(NULL);
//
//    myhtml_hperf_print("Parse html", all_start1, all_stop1, stdout);
    return 0;
    
    /* Default path or argument value */
    //const char* path = "/new/C-git/myhtml/test/test.html";
    //const char* path = "/new/C-git/myhtml/test/broken.html";
    const char* path = "/new/C-git/myhtml/test/test_full.html";
    
    if (argc == 2) {
        path = argv[1];
    }
    
    setbuf(stdout, 0);
    
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    struct res_html res  = load_html(path);
    
    uint64_t all_start = myhtml_hperf_clock(NULL);
    uint64_t tree_init_start = myhtml_hperf_clock(NULL);
    
    // init once for N html
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    uint64_t tree_init_stop = myhtml_hperf_clock(NULL);
    uint64_t parse_start = myhtml_hperf_clock(NULL);
    
    for(size_t i = 0; i < 1; i++)
    {
        //myhtml_parse(tree, text, strlen(text));
        myhtml_parse(tree, res.html, res.size);
        
        //myhtml_tree_print_node_childs(tree, tree->document, stdout, 0);
    }
    
    uint64_t parse_stop = myhtml_hperf_clock(NULL);
    uint64_t all_stop = myhtml_hperf_clock(NULL);
    
    printf("\n\nInformation:\n");
    printf("Timer (%llu ticks/sec):\n", (unsigned long long) myhtml_hperf_res(NULL));
    myhtml_hperf_print("\tFirst Tree init", tree_init_start, tree_init_stop, stdout);
    myhtml_hperf_print("\tParse html", parse_start, parse_stop, stdout);
    myhtml_hperf_print("\tTotal", all_start, all_stop, stdout);
    printf("\n");
    
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    free(res.html);
    
    return 0;
}





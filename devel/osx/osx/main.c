//
//  main.c
//  myhtml
//
//  Created by Alexander Borisov on 28.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

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
    
    long l = ftell(f); // именно такой размер у файла
    fseek(f, 0L, SEEK_SET);
    
    char *html = (char*)mymalloc(l);
    fread(html, 1, l, f);
    
    fclose(f);
    
    struct res_html res = {html, (size_t)l};
    return res;
}

void print_token_by_index(myhtml_tree_t* tree, mytags_ctx_index_t tag_ctx_idx)
{
    mytags_t* tags = tree->myhtml->tags;
    myhtml_tree_indexes_t* indexes = tree->indexes;
    
    mctree_node_t* mctree_nodes = tags->tree->nodes;
    size_t mctree_id = mytags_get(tags, tag_ctx_idx, mctree_id);
    size_t tag_name_size = mctree_nodes[mctree_id].str_size;
    
    printf("\nPosition and length for tags \"%.*s\":\n",
           (int)tag_name_size, mctree_nodes[mctree_id].str);
    
    mytags_index_tag_node_t *node = mytags_index_tag_get_first(indexes->tags, tag_ctx_idx);
    while (node)
    {
        if(node->node->token)
            myhtml_token_print_by_idx(tree, node->node->token, stdout);
        
        node = node->next;
    }
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
                //printf("%lu: %s\n", count, path);
                
                struct res_html res = load_html(path);
                
                myhtml_parse(tree, res.html, res.size);
                
                
                
//                myhtml_tree_node_t **node_list = myhtml_get_elements_by_tag_id(tree, MyTAGS_TAG_TITLE, NULL);
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
    /* Default path or argument value */
    //const char* path = "/new/C-git/myhtml/test/test.html";
    //const char* path = "/new/C-git/myhtml/test/broken.html";
    const char* path = "/new/C-git/myhtml/test/test_full.html";
    //const char* path = "/new/Test/2.html";

    if (argc == 2) {
        path = argv[1];
    }

    setbuf(stdout, 0);
    
    //mcobject_async_test();
    //mchar_async_test();
//    uint64_t all = myhtml_rdtsc();
//    test_all();
//    uint64_t all_s = myhtml_rdtsc();
//    
//    myhtml_rdtsc_print("Total", all, all_s);
//    
//    return 0;
    
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    //usleep(10000000);
    
    struct res_html res = load_html(path);

    uint64_t all_start = myhtml_hperf_clock(NULL);

    uint64_t tree_init_start = myhtml_hperf_clock(NULL);
    // init once for N html
    
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    //tree->is_single = mytrue;
    
    //myhtml_tree_t* tree_2 = myhtml_tree_create();
    //myhtml_tree_init(tree_2, myhtml);
    
    uint64_t tree_init_stop = myhtml_hperf_clock(NULL);
    uint64_t parse_start = myhtml_hperf_clock(NULL);
    
    for(size_t i = 0; i < 1; i++)
    {
        //myhtml_tokenizer_begin(myhtml, tree, res.html, res.size);
        //myhtml_tokenizer_end(myhtml, tree);
        
        //tree->is_single = mytrue;
        myhtml_parse(tree, res.html, res.size);
        
//        myhtml_tree_node_t **node_list = myhtml_get_elements_by_tag_id(tree, MyTAGS_TAG_TITLE, NULL);
//        
//        if(node_list && node_list[0])
//            if(node_list[0]->token)
//                myhtml_tree_print_by_tree_idx(tree, node_list[0]->child, stdout, 0);
//        
//        myhtml_destroy_node_list(node_list);
        
        //print_token_by_index(tree, MyTAGS_TAG_A);
        
        myhtml_collection_t *collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyTAGS_TAG_TITLE, NULL);
        
        if(collection && collection->list) {
            for (size_t i = 0; i < collection->length; i++)
            {
                myhtml_tree_node_t *text_node = myhtml_node_child(collection->list[i]);
                
                if(text_node) {
                    const char* text = myhtml_node_text(text_node, NULL);
                    
                    if(text)
                        printf("Title: %s\n", text);
                }
            }
        }
        //myhtml_tree_print_by_tree_idx(tree, tree->document->child, stdout, 0);
    }
    
    //myhtml_parse_fragment(tree_2, res.html, res.size);
    
    uint64_t parse_stop = myhtml_hperf_clock(NULL);
    uint64_t all_stop = myhtml_hperf_clock(NULL);
    
    //myhtml_tree_print_by_tree_idx(tree_2, tree_2->document->child, stdout, 0);
    
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





//
//  main.c
//  myhtml
//
//  Created by Alexander Borisov on 28.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#include "main.h"

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
    
    char *html = (char*)malloc(l);
    fread(html, 1, l, f);
    
    fclose(f);
    
    struct res_html res = {html, (size_t)l};
    return res;
}

int main(int argc, const char * argv[]) {
    
    myhtml_t* myhtml = myhtml_init(4);
    setbuf(stdout, 0);
    
//    size_t i;
//    
//    uint64_t test_start = rdtsc();
//    for (i = 0; i < 10000; i++) {
//        mctree_index_t idx = mctree_search(myhtml->tags->tree, "div", 3);
//    }
//    print_rdtsc("Test", test_start, rdtsc());
//    
//    test_start = rdtsc();
//    for (i = 0; i < 10000; i++) {
//        mctree_index_t idx = mctree_search_lowercase(myhtml->tags->tree, "div", 3);
//    }
//    print_rdtsc("Test lowercase", test_start, rdtsc());
//    
//    exit(0);
    
    //struct res_html res = load_html("/new/C/myhtml/test/test.html");
    //struct res_html res = load_html("/new/C/myhtml/test/broken.html");
    struct res_html res = load_html("/new/C/myhtml/test/test_full.html");
    //struct res_html res = load_html("/new/C/myhtml/test/script.html");
    
    // for a freebsd
    //struct res_html res = load_html("/home/lastmac/my/mhtml/test/test_full.html");
    
    //usleep(2000000);
    
    uint64_t all_start = myhtml_rdtsc();
    
    uint64_t parse_start = myhtml_rdtsc();
    
    myhtml_tree_t* tree = myhtml_tree_init(myhtml);
    
    for(size_t i = 1; i < 2; i++)
    {
        myhtml_parse_begin(myhtml, tree, res.html, res.size);
        myhtml_parse_end(myhtml, tree);
        
        //myhtml_tree_t* tree = myhtml_parse(myhtml, res.html, res.size);
        //myhtml_tree_destroy(tree);
        
        myhtml_clean(myhtml);
        myhtml_tree_clean(tree);
    }
    
    uint64_t parse_stop = myhtml_rdtsc();
    
    //usleep(5000000);
    
//    uint64_t parse_t_start = myhtml_rdtsc();
//    myhtml_parse(myhtml, res.html, res.size);
//    myhtml_thread_wait_all_done(myhtml);
//    uint64_t parse_t_stot = myhtml_rdtsc();
    
    uint64_t all_stop = myhtml_rdtsc();
    
    printf("\n\nInformation:\n");
    printf("\tQueue length: %lu\n", myhtml->queue->nodes_length);
    printf("Timer:\n");
    myhtml_rdtsc_print("\tParse html", parse_start, parse_stop);
    //myhtml_rdtsc_print("Parse html t", parse_t_start, parse_t_stot);
    myhtml_rdtsc_print("\tTotal", all_start, all_stop);
    printf("\n");
    
    //usleep(999126226);
    
    //usleep(5000000000);
    
    myhtml_destroy(myhtml);
    free(res.html);
    
    return 0;
}



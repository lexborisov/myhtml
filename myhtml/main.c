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
    
    size_t node_id = mytags_index_tag_get_first(indexes->tags, tag_ctx_idx);
    while (node_id)
    {
        myhtml_token_node_t* token = mytags_index_tag_node_attr(indexes->tags, node_id, token);
        
        myhtml_token_print_by_idx(tree, token, stdout);
        
        node_id = mytags_index_tag_node_attr(indexes->tags, node_id, next);
    }
}

int main(int argc, const char * argv[])
{
    setbuf(stdout, 0);
    
    myhtml_t* myhtml = myhtml_init(4);
    
    //struct res_html res = load_html("/new/C-git/myhtml/test/test.html");
    //struct res_html res = load_html("/new/C-git/myhtml/test/broken.html");
    struct res_html res = load_html("/new/C-git/myhtml/test/test_full.html");
    //struct res_html res = load_html("/new/C-git/myhtml/test/script.html");
    
    uint64_t all_start = myhtml_rdtsc();
    
    uint64_t tree_init_start = myhtml_rdtsc();
    // init once for N html
    myhtml_tree_t* tree = myhtml_tree_init(myhtml);
    uint64_t tree_init_stop = myhtml_rdtsc();
    
    uint64_t parse_start = myhtml_rdtsc();
    
    for(size_t i = 1; i < 2; i++)
    {
        myhtml_tokenizer_begin(myhtml, tree, res.html, res.size);
        myhtml_tokenizer_end(myhtml, tree);
        
        //print_token_by_index(tree, MyTAGS_TAG__DOCTYPE); -- now is not work
        //myhtml_tree_print_by_tree_idx(tree, tree->document->child, stdout, 0);
        
        myhtml_clean(myhtml);
        myhtml_tree_clean(tree);
    }
    
    uint64_t parse_stop = myhtml_rdtsc();
    uint64_t all_stop = myhtml_rdtsc();
    
    printf("\n\nInformation:\n");
    printf("Timer:\n");
    myhtml_rdtsc_print("\tFirst Tree init", tree_init_start, tree_init_stop);
    myhtml_rdtsc_print("\tParse html", parse_start, parse_stop);
    myhtml_rdtsc_print("\tTotal", all_start, all_stop);
    printf("\n");
    
    myhtml_destroy(myhtml);
    free(res.html);
    
    return 0;
}





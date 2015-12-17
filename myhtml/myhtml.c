//
//  main.c
//  myhtml
//
//  Created by Alexander Borisov on 24.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#include "myhtml.h"


myhtml_t * myhtml_init(size_t thread_count)
{
    myhtml_t* myhtml = (myhtml_t*)mymalloc(sizeof(myhtml_t));
    
    myhtml->tags     = mytags_init();
    myhtml->queue    = myhtml_queue_create(4096);
    
    myhtml_tokenizer_state_init(myhtml);
    myhtml_rules_init(myhtml);
    
    // and last, create threads
    myhtml_thread_init(myhtml, "lastmac", 4, thread_count,
                       myhtml_parser_stream,
                       myhtml_parser_worker,
                       myhtml_parser_index);
    
    myhtml_clean(myhtml);
    
    return myhtml;
}

void myhtml_clean(myhtml_t* myhtml)
{
    myhtml_queue_clean(myhtml->queue);
    myhtml_thread_clean(myhtml->thread, myhtml->queue->nodes_length);
}

myhtml_t* myhtml_destroy(myhtml_t* myhtml)
{
    if(myhtml == NULL)
        return NULL;
    
    myhtml_thread_destroy(myhtml);
    myhtml_tokenizer_state_destroy(myhtml);
    
    myhtml->tags  = mytags_destroy(myhtml->tags);
    myhtml->queue = myhtml_queue_destroy(myhtml->queue);
    
    free(myhtml);
    
    return NULL;
}

myhtml_tree_t * myhtml_parse(myhtml_t* myhtml, const char* html, size_t html_size)
{
    myhtml_tree_t* tree = myhtml_tree_init(myhtml);
    
    myhtml_tokenizer_begin(myhtml, tree, html, html_size);
    myhtml_tokenizer_end(myhtml, tree);
    
    return tree;
}

myhtml_tree_t * myhtml_parse_fragment(myhtml_t* myhtml, const char* html, size_t html_size)
{
    myhtml_tree_t* tree = myhtml_tree_init(myhtml);
    
    myhtml_tokenizer_fragment_init(tree, MyTAGS_TAG_DIV, MyHTML_NAMESPACE_HTML);
    
    myhtml_tokenizer_begin(myhtml, tree, html, html_size);
    myhtml_tokenizer_end(myhtml, tree);
    
    return tree;
}

myhtml_tree_node_t * myhtml_tokenizer_fragment_init(myhtml_tree_t* tree, mytags_ctx_index_t tag_idx, enum myhtml_namespace my_namespace)
{
    // step 3
    tree->fragment = myhtml_tree_node_create(tree);
    tree->fragment->namespace = my_namespace;
    tree->fragment->tag_idx = tag_idx;
    
    // skip step 4, is already done
    
    // step 5-7
    myhtml_tree_node_t* root = myhtml_tree_node_insert_root(tree, NULL, my_namespace);
    
    if(tag_idx == MyTAGS_TAG_TEMPLATE)
        myhtml_tree_template_insertion_append(tree, MyHTML_INSERTION_MODE_IN_TEMPLATE);
    
    myhtml_tree_reset_insertion_mode_appropriately(tree);
    
    return root;
}

void myhtml_tokenizer_begin(myhtml_t* myhtml, myhtml_tree_t* tree, const char* html, size_t html_length)
{
    myhtml_queue_node_index_t qnode_idx = myhtml_queue_node_current(myhtml->queue);
    
    mh_queue_set(qnode_idx, html)        = html;
    mh_queue_set(qnode_idx, myhtml_tree) = tree;
    
    mh_tree_set(queue) = myhtml_queue_node_current(myhtml->queue);
    myhtml_token_node_malloc(tree->token, mh_queue_get(qnode_idx, token), 0);
    
    myhtml_tokenizer_continue(myhtml, tree, html, html_length);
}

void myhtml_tokenizer_end(myhtml_t* myhtml, myhtml_tree_t* tree)
{
    myhtml_thread_wait_all_for_done(myhtml);
}

void myhtml_tokenizer_continue(myhtml_t* myhtml, myhtml_tree_t* tree, const char* html, size_t html_length)
{
    myhtml_tokenizer_state_f* state_f = myhtml->parse_state_func;
    
    mh_thread_master_done(myfalse);
    mh_thread_master_post();
    
    mh_thread_stream_done(myfalse);
    mh_thread_stream_post();
    
    mh_thread_done(MyHTML_THREAD_INDEX_ID, myfalse);
    mh_thread_post(MyHTML_THREAD_INDEX_ID);
    
    size_t offset = 0;
    
    myhtml_queue_t* queue = myhtml->queue;
    
    while (offset < html_length) {
        offset = state_f[tree->state](tree, &queue->nodes[tree->queue], html, offset, html_length);
    }
}

void myhtml_tokenizer_wait(myhtml_t* myhtml)
{
    mh_thread_master_done(mytrue);
    mh_thread_stream_done(mytrue);
    mh_thread_done(MyHTML_THREAD_INDEX_ID, mytrue);
}

void myhtml_tokenizer_post(myhtml_t* myhtml)
{
    mh_thread_master_done(myfalse);
    mh_thread_stream_done(myfalse);
    mh_thread_done(MyHTML_THREAD_INDEX_ID, myfalse);
    
    mh_thread_master_post();
    mh_thread_stream_post();
    mh_thread_post(MyHTML_THREAD_INDEX_ID);
}

mybool_t myhtml_utils_strcmp(const char* ab, const char* to_lowercase, size_t size)
{
    size_t i = 0;
    
    for(;;) {
        if(i == size)
            return mytrue;
        
        if((const unsigned char)(to_lowercase[i] > 0x40 && to_lowercase[i] < 0x5b ?
                                 (to_lowercase[i]|0x60) : to_lowercase[i]) != (const unsigned char)ab[i])
        {
            return myfalse;
        }
        
        i++;
    }
    
    return myfalse;
}

uint64_t myhtml_rdtsc(void) {
    uint64_t x;
    __asm__ volatile ("rdtsc\n\tshl $32, %%rdx\n\tor %%rdx, %%rax" : "=a" (x) : : "rdx");
    return x;
}

// 2... is fixed cpu 
void myhtml_rdtsc_print(char *name, uint64_t x, uint64_t y) {
    printf("%s: %0.5f\n", name, (float)(0.001 * ((y - x) / 2000000.0)));
}

void myhtml_rdtsc_print_by_val(char *name, uint64_t x) {
    printf("%s: %0.5f\n", name, (float)(0.001 * (x / 2000000.0)));
}


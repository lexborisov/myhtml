//
//  myhtml.h
//  myhtml
//
//  Created by Alexander Borisov on 24.09.15.
//  Copyright (c) 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_myhtml_h
#define myhtml_myhtml_h

#include "myosi.h"

#include "mctree.h"
#include "mytags.h"
#include "def.h"
#include "parser.h"
#include "tokenizer.h"
#include "thread.h"
#include "tree.h"
#include "rules.h"
#include "token.h"

#define mh_thread_get(__idx__, __attr__) myhtml_thread_get(myhtml->thread, __idx__, __attr__)
#define mh_thread_set(__idx__, __attr__) myhtml_thread_set(myhtml->thread, __idx__, __attr__)

#define mh_thread_post(__idx__) sem_post(myhtml_thread_get(myhtml->thread, __idx__, sem))
#define mh_thread_wait(__idx__) sem_wait(myhtml_thread_get(myhtml->thread, __idx__, sem))
#define mh_thread_done(__idx__, __param__) myhtml_thread_set(myhtml->thread, __idx__, opt) = __param__;

#define mh_thread_master_get(__attr__) myhtml_thread_master_get(myhtml->thread, __attr__)
#define mh_thread_master_set(__attr__) myhtml_thread_master_set(myhtml->thread, __attr__)

#define mh_thread_master_post() sem_post(myhtml_thread_master_get(myhtml->thread, sem))
#define mh_thread_master_wait() sem_wait(myhtml_thread_master_get(myhtml->thread, sem))
#define mh_thread_master_done(__param__) myhtml_thread_master_set(myhtml->thread, opt) = __param__;

#define mh_thread_stream_post() sem_post(myhtml_thread_stream_get(myhtml->thread, sem))
#define mh_thread_stream_wait() sem_wait(myhtml_thread_stream_get(myhtml->thread, sem))
#define mh_thread_stream_done(__param__) myhtml_thread_stream_set(myhtml->thread, opt) = __param__;

#define mh_tree_get(__attr__) myhtml_tree_get(tree, __attr__)
#define mh_tree_set(__attr__) myhtml_tree_set(tree, __attr__)

#define mh_queue_current() mh_tree_get(queue)

#define mh_state_get(__tree__) myhtml_tree_get(__tree__, state)
#define mh_state_set(__tree__) myhtml_tree_set(__tree__, state)

#define mh_tags_get(__idx__, __attr__) mytags_get(myhtml->tags, __idx__, __attr__)

#define mh_queue_last(__attr__) myhtml->queue->nodes[myhtml_queue_node_current(myhtml->queue)].__attr__
#define mh_queue_get(__idx__, __attr__) myhtml->queue->nodes[__idx__].__attr__
#define mh_queue_set(__idx__, __attr__) mh_queue_get(__idx__, __attr__)
#define mh_queue_current_get(__attr__) mh_queue_get(mh_queue_current(), __attr__)
#define mh_queue_current_set(__attr__) mh_queue_current_get(__attr__)

#define mh_queue_add(__tree__, __html__, __begin__)                                                           \
    if(__tree__->is_single) { \
        myhtml_parser_worker(0, __tree__->current_qnode); \
        while(myhtml_rules_tree_dispatcher(__tree__, __tree__->current_qnode->token)){}; \
    } \
    __tree__->current_qnode = mythread_queue_node_malloc(__tree__->myhtml->queue, __html__, __begin__, NULL); \
    __tree__->current_qnode->tree = __tree__; \
    myhtml_token_node_malloc(__tree__->token, __tree__->current_qnode->token, __tree__->token->mcasync_token_id)

#define mh_token_get(__idx__, __attr__) tree->token->nodes[__idx__].__attr__
#define mh_token_set(__idx__, __attr__) mh_token_get(__idx__, __attr__)

#define mh_tree_token_current(__attr__) mh_token_get(myhtml_tree_token_current(tree), __attr__)
#define mh_tree_token_attr_current(__attr__) mh_token_get(myhtml_tree_token_attr_current(tree), __attr__)

#define mh_token_attr_get(__idx__, __attr__) tree->token->attr[__idx__].__attr__
#define mh_token_attr_set(__idx__, __attr__) mh_token_attr_get(__idx__, __attr__)
#define mh_token_attr_current(__idx__) mh_token_get(__idx__, attr_last)

#define mh_queue_token_get(__idx__, __attr__) mh_token_get(mh_queue_get(__idx__, token), __attr__)
#define mh_queue_token_set(__idx__, __attr__) mh_queue_token_get(__idx__, __attr__)

// space, tab, LF, FF, CR
#define myhtml_whithspace(__char__, __action__, __logic__)    \
    __char__ __action__ ' ' __logic__                    \
    __char__ __action__ '\t' __logic__                    \
    __char__ __action__ '\n' __logic__                    \
    __char__ __action__ '\f' __logic__                    \
    __char__ __action__ '\r'

#define myhtml_ascii_char_cmp(__char__)     \
    ((__char__ >= 'a' && __char__ <= 'z') || \
    (__char__ >= 'A' && __char__ <= 'Z'))

#define myhtml_ascii_char_unless_cmp(__char__) \
    ((__char__ < 'a' || __char__ > 'z') &&      \
    (__char__ < 'A' || __char__ > 'Z'))

struct myhtml {
    mytags_t            *tags;
    mythread_queue_t    *queue;
    mythread_t          *thread;
    
    myhtml_tokenizer_state_f* parse_state_func;
    myhtml_insertion_f* insertion_func;
    
    enum myhtml_options opt;
};

myhtml_t * myhtml_create(void);
void myhtml_init(myhtml_t* myhtml, enum myhtml_options opt, size_t thread_count, size_t queue_size, size_t token_nodes_size);
void myhtml_clean(myhtml_t* myhtml);
myhtml_t* myhtml_destroy(myhtml_t* myhtml);

void myhtml_parse(myhtml_tree_t* tree, const char* html, size_t html_size);
void myhtml_parse_fragment(myhtml_tree_t* tree, const char* html, size_t html_size);

myhtml_tree_node_t ** myhtml_get_elements_by_tag_id(myhtml_tree_t* tree, mytags_ctx_index_t tag_id, size_t* return_length);
myhtml_tree_node_t ** myhtml_get_elements_by_name(myhtml_tree_t* tree, const char* html, size_t length, size_t* return_length);

void myhtml_destroy_node_list(myhtml_tree_node_t **node_list);


mybool_t myhtml_utils_strcmp(const char* ab, const char* to_lowercase, size_t size);

/** 
 * Platform-specific hdef performance clock queries.
 * Implemented in perf.c
 */ 

/** Get clock resolution */
uint64_t myhtml_hperf_res(myhtml_status_t *status);

/** Get current value in clock ticks */
uint64_t myhtml_hperf_clock(myhtml_status_t *status);

/** Print an hperf measure */
myhtml_status_t myhtml_hperf_print(const char *name, uint64_t x, uint64_t y, FILE *fh);
myhtml_status_t myhtml_hperf_print_by_val(const char *name, uint64_t x, FILE *fh);

#endif

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
#include "myhtml_def.h"
#include "myhtml_parse.h"
#include "myhtml_thread.h"
#include "myhtml_tree.h"
#include "myhtml_rules.h"
#include "myhtml_token.h"
#include "myhtml_queue.h"

#define mh_thread_get(__idx__, __attr__) myhtml_thread_get(myhtml->thread, __idx__, __attr__)
#define mh_thread_set(__idx__, __attr__) myhtml_thread_set(myhtml->thread, __idx__, __attr__)

#define mh_thread_master_get(__attr__) myhtml_thread_master_get(myhtml->thread, __attr__)
#define mh_thread_master_set(__attr__) myhtml_thread_master_set(myhtml->thread, __attr__)

#define mh_thread_master_post() sem_post(myhtml_thread_master_get(myhtml->thread, sem))
#define mh_thread_master_wait() sem_wait(myhtml_thread_master_get(myhtml->thread, sem))
#define mh_thread_master_done(__bool__) myhtml_thread_master_set(myhtml->thread, is_done) = __bool__;

#define mh_thread_stream_post() sem_post(myhtml_thread_stream_get(myhtml->thread, sem))
#define mh_thread_stream_wait() sem_wait(myhtml_thread_stream_get(myhtml->thread, sem))
#define mh_thread_stream_done(__bool__) myhtml_thread_stream_set(myhtml->thread, is_done) = __bool__;

#define mh_tree_get(__attr__) myhtml_tree_get(tree, __attr__)
#define mh_tree_set(__attr__) myhtml_tree_set(tree, __attr__)

#define mh_queue_current() mh_tree_get(queue)

#define mh_state_get(__tree__) myhtml_tree_get(__tree__, state)
#define mh_state_set(__tree__) myhtml_tree_set(__tree__, state)

#define mh_tags_get(__idx__, __attr__) mytags_get(myhtml->tags, __idx__, __attr__)

#define mh_queue_last(__attr__) myhtml->queue->nodes[myhtml_queue_node_current(myhtml->queue)].__attr__
#define mh_queue_get(__idx__, __attr__) myhtml->queue->nodes[__idx__].__attr__
#define mh_queue_set(__idx__, __attr__) mh_queue_get(__idx__, __attr__)

#define mh_queue_add(__tree__, __html__, __qnode_idx__, __tag_begin__)                                  \
    mh_queue_set(__qnode_idx__, token_idx) = mh_tree_token_current_index();                             \
    myhtml_token_node_malloc(__tree__->token, __tag_begin__, mh_tree_token_current_index());            \
    myhtml_queue_node_malloc(myhtml->queue, __html__, mh_tree_token_current_index(), myfalse, 0, tree); \
    __qnode_idx__ = myhtml_queue_node_current(myhtml->queue)

#define mh_token_get(__idx__, __attr__) tree->token->nodes[__idx__].__attr__
#define mh_token_set(__idx__, __attr__) mh_token_get(__idx__, __attr__)

#define mh_tree_token_current(__attr__) mh_token_get(myhtml_tree_token_current(tree), __attr__)
#define mh_tree_token_current_index() myhtml_tree_token_current(tree)
#define mh_tree_token_attr_current(__attr__) mh_token_get(myhtml_tree_token_attr_current(tree), __attr__)
#define mh_tree_token_attr_current_index() myhtml_tree_token_attr_current(tree)

#define mh_token_attr_get(__idx__, __attr__) tree->token->attr[__idx__].__attr__
#define mh_token_attr_set(__idx__, __attr__) mh_token_attr_get(__idx__, __attr__)
#define mh_token_attr_current(__idx__) mh_token_get(__idx__, attr_last)

#define mh_queue_token_get(__idx__, __attr__) mh_token_get(mh_queue_get(__idx__, token_idx), __attr__)
#define mh_queue_token_set(__idx__, __attr__) mh_queue_token_get(__idx__, __attr__)

// space, tab, LF, FF, CR
#define myhtml_whithspace(__char__, __action__, __logic__)    \
    __char__ __action__ ' ' __logic__                    \
    __char__ __action__ '\t' __logic__                    \
    __char__ __action__ '\n' __logic__                    \
    __char__ __action__ '\f' __logic__                    \
    __char__ __action__ '\r'

#define myhtml_ascii_char_cmp(__char__)     \
    (__char__ >= 'a' && __char__ <= 'z') || \
    (__char__ >= 'A' && __char__ <= 'Z')

#define myhtml_ascii_char_unless_cmp(__char__) \
    (__char__ < 'a' || __char__ > 'z') &&      \
    (__char__ < 'A' || __char__ > 'Z')


struct myhtml {
    mytags_t          *tags;
    myhtml_queue_t    *queue;
    myhtml_thread_t   *thread;
    
    myhtml_parse_state_f* parse_state_func;
    myhtml_insertion_f* insertion_func;
};

myhtml_t * myhtml_init(size_t thread_count);
void myhtml_clean(myhtml_t* myhtml);
myhtml_t* myhtml_destroy(myhtml_t* myhtml);

myhtml_tree_t * myhtml_parse(myhtml_t* myhtml, const char* html, size_t html_size);

void myhtml_parse_begin(myhtml_t* myhtml, myhtml_tree_t* tree, const char* html, size_t html_length);
void myhtml_parse_end(myhtml_t* myhtml, myhtml_tree_t* tree);
void myhtml_parse_continue(myhtml_t* myhtml, myhtml_tree_t* tree, const char* html, size_t html_length);
void myhtml_parse_wait(myhtml_t* myhtml);
void myhtml_parse_post(myhtml_t* myhtml);

mybool_t myhtml_utils_strcmp(const char* ab, const char* to_lowercase, size_t size);

uint64_t myhtml_rdtsc(void);
void myhtml_rdtsc_print(char *name, uint64_t x, uint64_t y);
void myhtml_rdtsc_print_by_val(char *name, uint64_t x);

#endif

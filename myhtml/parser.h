//
//  myhtml_parser.h
//  myhtml
//
//  Created by Alexander Borisov on 17.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_parser_h
#define myhtml_parser_h

#include "myosi.h"
#include "myhtml.h"
#include "tree.h"
#include "token.h"
#include "string.h"
#include "utils/mcobject.h"

void myhtml_parser_index(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_worker(mythread_id_t thread_id, mythread_queue_node_t *qnode);

void myhtml_parser_worker_index_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_worker_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_worker_index(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_stream_index(mythread_id_t thread_id, mythread_queue_node_t *qnode);

#endif /* myhtml_parser_h */

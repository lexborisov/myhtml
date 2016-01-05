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

#ifndef MyHTML_PARSER_H
#define MyHTML_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "myhtml/myosi.h"
#include "myhtml/myhtml.h"
#include "myhtml/mystring.h"
#include "myhtml/tree.h"
#include "myhtml/token.h"

void myhtml_parser_index(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_worker(mythread_id_t thread_id, mythread_queue_node_t *qnode);

void myhtml_parser_worker_index_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_worker_stream(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_worker_index(mythread_id_t thread_id, mythread_queue_node_t *qnode);
void myhtml_parser_stream_index(mythread_id_t thread_id, mythread_queue_node_t *qnode);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* myhtml_parser_h */

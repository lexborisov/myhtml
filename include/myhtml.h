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

#ifndef __MyHTML_myhtml_H__
#define __MyHTML_myhtml_H__

/**
 * @file include/myhtml.h
 *
 * Fast C/C++ HTML 5 Parser. Using threads.
 * With possibility of a Single Mode.
 * 
 * C99 and POSIX Threads! No dependencies!
 *
 * By https://html.spec.whatwg.org/ specification.
 *
 */

#define MyHTML_VERSION_MAJOR 0
#define MyHTML_VERSION_MINOR 1
#define MyHTML_VERSION_PATCH 0

#include <stddef.h>

#ifdef __cplusplus
//extern "C" {
#endif

/**
 * @struct myhtml_t MyHTML
 *
 * Basic structure. Create once for using many times.
*/
typedef struct myhtml myhtml_t;

/**
 * @struct myhtml_tree_t MyHTML_TREE
 *
 * Secondary structure. Create once for using many times.
 */
typedef struct myhtml_tree myhtml_tree_t;

/***********************************************************************************
 *
 * MyHTML
 *
 ***********************************************************************************/

/**
 * Initialization of a MyHTML structure
 *
 *
 * @param[in] thread_count
 * thread_count = MyHTML_THREAD_MIN_THREADS if == 0,
 * thread_count = MyHTML_THREAD_MAX_THREADS if > MyHTML_THREAD_MAX_THREADS
 * @return MyHTML if successful, otherwise an NULL value.
 */
myhtml_t*
myhtml_init(size_t thread_count);

/**
 * Cleanup of a MyHTML structure
 *
 * @param[in] MyHTML
 */
void
myhtml_clean(myhtml_t* myhtml);

myhtml_tree_t*
myhtml_parse(myhtml_t* myhtml, const char* html, size_t html_size);

myhtml_tree_t*
myhtml_parse_fragment(myhtml_t* myhtml, const char* html, size_t html_size);

/**
 * Destroy of a MyHTML structure
 *
 * @param[in] MyHTML
 * @return NULL if successful, otherwise an MyHTML structure.
 */
myhtml_t*
myhtml_destroy(myhtml_t* myhtml);

/***********************************************************************************
 *
 * MyHTML_TREE
 *
 ***********************************************************************************/


/***********************************************************************************
 *
 * MyHTML_TOKENIZER
 *
 ***********************************************************************************/
void
myhtml_tokenizer_begin(myhtml_t* myhtml, myhtml_tree_t* tree,
                       const char* html, size_t html_length);

void
myhtml_tokenizer_end(myhtml_t* myhtml, myhtml_tree_t* tree);

void
myhtml_tokenizer_continue(myhtml_t* myhtml, myhtml_tree_t* tree,
                          const char* html, size_t html_length);

void
myhtml_tokenizer_wait(myhtml_t* myhtml);

void
myhtml_tokenizer_post(myhtml_t* myhtml);

myhtml_tree_node_t*
myhtml_tokenizer_fragment_init(myhtml_tree_t* tree,
                               mytags_ctx_index_t tag_idx,
                               enum myhtml_namespace my_namespace);

#ifdef __cplusplus
//} /* extern "C" */
#endif

#endif /*__MyHTML_myhtml_H__*/

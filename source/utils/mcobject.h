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

#ifndef MyHTML_UTILS_MCOBJECT_H
#define MyHTML_UTILS_MCOBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "myosi.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef struct mcobject mcobject_t;
typedef void (*mcobject_callback_new_f)(mcobject_t* mcobject, size_t idx);

struct mcobject {
    void**  obj;
    
    void*   nodes;
    size_t  nodes_length;
    size_t  nodes_size;
    
    size_t* cache;
    size_t  cache_length;
    volatile size_t last_length;
    
    size_t  struct_size;
    
    mcobject_callback_new_f new_func;
};

mcobject_t * mcobject_create(size_t size, size_t struct_size, void* obj, mcobject_callback_new_f callback_new);
void mcobject_clean(mcobject_t* mcobject);
mcobject_t * mcobject_destroy(mcobject_t* mcobject);

size_t mcobject_malloc(mcobject_t* mcobject);
void mcobject_free(mcobject_t* mcobject, size_t idx);

void mcobject_malloc_segment(mcobject_t* mcobject, size_t *list, size_t count);
void mcobject_free_segment(mcobject_t* mcobject, size_t *list, size_t length);

void mcobject_call_callback_new(mcobject_t* mcobject, size_t idx);
size_t mcobject_length(mcobject_t* mcobject);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* mcobject_h */


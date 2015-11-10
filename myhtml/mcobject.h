//
//  mcobject.h
//  myhtml
//
//  Created by Alexander Borisov on 27.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef mcobject_h
#define mcobject_h

#include <stdio.h>
#include <stdlib.h>

struct mcobject {
    void**  obj;
    
    void*   nodes;
    size_t  nodes_length;
    size_t  nodes_size;
    
    size_t* cache;
    size_t  cache_length;
    
    size_t struct_size;
}
typedef mcobject_t;


mcobject_t * mcobject_create(size_t size, size_t struct_size, void* obj);
void mcobject_clean(mcobject_t* mcobject);
mcobject_t * mcobject_destroy(mcobject_t* mcobject);

size_t mcobject_malloc(mcobject_t* mcobject);
void mcobject_free(mcobject_t* mcobject, size_t idx);

#endif /* mcobject_h */


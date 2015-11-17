//
//  myosi.c
//  myhtml
//
//  Created by Alexander Borisov on 16.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myosi.h"

// see who's call realloc
void* myrealloc(void* dst, size_t size)
{
    return realloc(dst, size);
}



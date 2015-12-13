//
//  myosi.c
//  myhtml
//
//  Created by Alexander Borisov on 16.11.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#include "myosi.h"

void * mymalloc(size_t size)
{
    //printf("Call malloc\n");
    return malloc(size);
}

// see who's call realloc
void * myrealloc(void* dst, size_t size)
{
    printf("Call realloc\n");
    return realloc(dst, size);
}

void * mycalloc(size_t num, size_t size)
{
    //printf("Call calloc\n");
    return calloc(num, size);
}




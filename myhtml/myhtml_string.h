//
//  myhtml_string.h
//  myhtml
//
//  Created by Alexander Borisov on 29.10.15.
//  Copyright © 2015 Alexander Borisov. All rights reserved.
//

#ifndef myhtml_string_h
#define myhtml_string_h

#include "myosi.h"

#define myhtml_string_get(__str__, __attr__) __str__->__attr__
#define myhtml_string_set(__str__, __attr__) myhtml_string_get(__str__, __attr__)
#define myhtml_string_len(__str__) myhtml_string_get(__str__, length)

struct myhtml_string {
    char*  data;
    size_t size;
    size_t length;
}
typedef myhtml_string_t;

typedef size_t myhtml_string_index_t;

myhtml_string_t * myhtml_string_create(size_t size);
void myhtml_string_init(myhtml_string_t* str, size_t size);
void myhtml_string_clean(myhtml_string_t* str);
myhtml_string_t * myhtml_string_destroy(myhtml_string_t* str, mybool_t destroy_obj);

void myhtml_string_append(myhtml_string_t* str, const char* buff, size_t length);
void myhtml_string_append_with_null(myhtml_string_t* str, const char* buff, size_t length);
void myhtml_string_append_one(myhtml_string_t* str, const char buff);
void myhtml_string_append_one_without_check(myhtml_string_t* str, const char buff);
void myhtml_string_append_lowercase_with_null(myhtml_string_t* str, const char* buff, size_t length);

mybool_t myhtml_string_check(myhtml_string_t* str, size_t length, size_t up_to);

#endif /* myhtml_string_h */

/*
 Copyright 2015-2016 Alexander Borisov
 
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

#ifndef MyHTML_UTILS_H
#define MyHTML_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "myosi.h"

size_t myhtml_strncasecmp(const char* str1, const char* str2, size_t size);
size_t myhtml_strcasecmp(const char* str1, const char* str2);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* utils_h */

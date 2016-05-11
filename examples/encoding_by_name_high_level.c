/*
 Copyright 2016 Alexander Borisov
 
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <myhtml/api.h>

int main(int argc, const char * argv[])
{
    const char *name = "cp1251";
    myhtml_encoding_t encoding;
    
    if(myhtml_encoding_by_name(name, strlen(name), &encoding))
    {
        if(encoding == MyHTML_ENCODING_WINDOWS_1251) {
            printf("Detected Encoding: windows-1251\n");
            return 0;
        }
    }
    
    printf("Detected Encoding: something wrong\n");
    
    return 0;
}





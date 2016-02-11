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
#include <myhtml/api.h>

struct res_html {
    char  *html;
    size_t size;
};

struct res_html load_html_file(const char* filename)
{
    FILE *fh = fopen(filename, "rb");
    if(fh == NULL) {
        fprintf(stderr, "Can't open html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    fseek(fh, 0L, SEEK_END);
    long size = ftell(fh);
    fseek(fh, 0L, SEEK_SET);
    
    char *html = (char*)malloc(size + 1);
    if(html == NULL) {
        fprintf(stderr, "Can't allocate mem for html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    size_t nread = fread(html, 1, size, fh);
    if (nread != size) {
        fprintf(stderr, "could not read %ld bytes (%zu bytes done)\n", size, nread);
        exit(EXIT_FAILURE);
    }

    fclose(fh);
    
    if(size < 0)
        size = 0;
    
    struct res_html res = {html, (size_t)size};
    return res;
}

int print_encoding(myhtml_encoding_t encoding)
{
    printf("Character encoding is ");
    
    switch (encoding) {
        case MyHTML_ENCODING_UTF_8:          printf("UTF-8");          break;
        case MyHTML_ENCODING_UTF_16LE:       printf("UTF_16LE");       break;
        case MyHTML_ENCODING_UTF_16BE:       printf("UTF_16BE");       break;
        case MyHTML_ENCODING_KOI8_R:         printf("KOI8_R");         break;
        case MyHTML_ENCODING_WINDOWS_1251:   printf("WINDOWS_1251");   break;
        case MyHTML_ENCODING_X_MAC_CYRILLIC: printf("X_MAC_CYRILLIC"); break;
        case MyHTML_ENCODING_IBM866:         printf("IBM866");         break;
        case MyHTML_ENCODING_ISO_8859_5:     printf("ISO_8859_5");     break;
        default:
            printf("UNKNOWN");
            break;
    }
    
    printf("\n");
    
    return 0;
}

int main(int argc, const char * argv[])
{
    const char* path;
    
    if (argc == 2) {
        path = argv[1];
    }
    else {
        printf("Bad ARGV!\nUse: detect_encoding_high_level <path_to_html_file>\n");
        exit(EXIT_FAILURE);
    }
    
    struct res_html res = load_html_file(path);
    
    myhtml_encoding_t encoding;
    
    // try detect by BOM
    if(myhtml_encoding_detect_bom(res.html, res.size, &encoding)) {
        return print_encoding(encoding);
    }
    
    if(myhtml_encoding_detect(res.html, res.size, &encoding)) {
        return print_encoding(encoding);
    }
    
    if(encoding != MyHTML_ENCODING_DEFAULT) {
        printf("It is possible that ");
        return print_encoding(encoding);
    }
    
    printf("I could not identify character encoding\n");
    return 0;
}





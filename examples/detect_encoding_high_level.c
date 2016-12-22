/*
 Copyright (C) 2015-2016 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 
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
    
    if(fseek(fh, 0L, SEEK_END) != 0) {
        fprintf(stderr, "Can't set position (fseek) in file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    long size = ftell(fh);
    
    if(fseek(fh, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "Can't set position (fseek) in file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    if(size <= 0) {
        fprintf(stderr, "Can't get file size or file is empty: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
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
    
    struct res_html res = {html, (size_t)size};
    return res;
}

void print_encoding(myhtml_encoding_t encoding)
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
    if (myhtml_encoding_detect_bom(res.html, res.size, &encoding)) {
        print_encoding(encoding);
    } else if (myhtml_encoding_detect(res.html, res.size, &encoding)) {
        print_encoding(encoding);
    } else if (encoding != MyHTML_ENCODING_DEFAULT) {
        printf("It is possible that ");
        print_encoding(encoding);
    } else {
        printf("I could not identify character encoding\n");
    }

    free(res.html);
    return 0;
}





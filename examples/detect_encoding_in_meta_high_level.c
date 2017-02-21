/*
 Copyright (C) 2015-2017 Alexander Borisov
 
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

#include "example.h"

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
        fprintf(stderr, "could not read %ld bytes (" MyHTML_FMT_Z " bytes done)\n", size, nread);
        exit(EXIT_FAILURE);
    }

    fclose(fh);

    struct res_html res = {html, (size_t)size};
    return res;
}

int main(int argc, const char * argv[])
{
    if (argc != 2) {
        printf("Bad ARGV!\nUse: detect_encoding_in_meta_high_level <path_to_html_file>\n");
        exit(EXIT_FAILURE);
    }
    
    struct res_html res = load_html_file(argv[1]);
    
    myhtml_encoding_t encoding = myhtml_encoding_prescan_stream_to_determine_encoding(res.html, res.size);
    
    if(encoding == MyHTML_ENCODING_NOT_DETERMINED) {
        printf("Can't detect encoding\n");
    }
    else {
        const char *encoding_name = myhtml_encoding_name_by_id(encoding, NULL);
        printf("Encoding: %s\n", encoding_name);
    }
    
    free(res.html);
    
    return 0;
}





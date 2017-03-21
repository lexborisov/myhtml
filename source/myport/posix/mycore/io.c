/*
 Copyright (C) 2015-2017 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#include "mycore/myosi.h"
#include <stdarg.h>

/* FILE */
FILE * mycore_fopen(const char *filename, const char *mode)
{
    return fopen(filename, mode);
}

int mycore_fclose(FILE *stream)
{
    return fclose(stream);
}

size_t mycore_fread(void *buffer, size_t size, size_t count, FILE *stream)
{
    return fread(buffer, size, count, stream);
}

size_t mycore_fwrite(const void *buffer, size_t size, size_t count, FILE *stream)
{
    return fwrite(buffer, size, count, stream);
}

int mycore_fflush(FILE *stream)
{
    return fflush(stream);
}

int mycore_fseek(FILE* stream, long offset, int origin)
{
    return fseek(stream, offset, origin);
}

long mycore_ftell(FILE* stream)
{
    return ftell(stream);
}

int mycore_ferror(FILE *stream)
{
    return ferror(stream);
}

/* setbuf */
void mycore_setbuf(FILE *stream, char *buffer)
{
    setbuf(stream, buffer);
}

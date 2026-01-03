
#ifndef FILE_C
#define FILE_C

#include "ansi_codes.h"
#include "string.c"
#include <stdio.h>

int FILE_read_to_string(FILE *file, String *str)
{
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (str->cap < length) {
        fprintf(
            stderr,
            ANSI_TEXT_RED "File to large to read to String!" ANSI_RESET);
        fflush(stderr);
        return 0;
    }

    fread(str->items, 1, length, file);

    str->len = length;
    return length;
}

#endif /* ifndef FILE_C */

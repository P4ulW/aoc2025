#include <stdio.h>
#include <string.h>
#include <time.h>
#include "arena.c"
#include "array.c"
#include "string.c"
typedef struct timeval timeval;

Array_Prototype(StringSlice);
Array_Impl(StringSlice);

const char filename[] = "test.txt";

static void read_file_to_string(String *string, FILE *file)
{
    int c;
    for (int i = 0; i < string->cap; i++) {
        c = fgetc(file);
        if (c == EOF) {
            break;
        }

        // printf("%c", (char)c);
        String_push(string, (char)c);
    }
    return;
}

static void StringSlice_split_to_slices(
    ArrayStringSlice *slices, StringSlice to_split, const char split_char)
{
    U32 len           = 0;
    StringSlice slice = {0};
    int i             = 0;
    U32 start_new     = 1;

    for (int i = 0; i < to_split.len; i++) {
        char current = to_split.items[i];
        if (start_new) {
            slice.items = &to_split.items[i];
            len         = 0;
            slice.len   = 0;
            start_new   = 0;
        }

        if (current == split_char) {
            slice.len = len;
            ArrayStringSlice_push(slices, slice);
            start_new = 1;
        }
        len += 1;
    }

    slice.len = len;
    // ArrayStringSlice_push(slices, slice);

    return;
}

static void StringSlice_print(StringSlice str)
{
    printf("str: len %d content: ", str.len);
    for (int i = 0; i < str.len; i++) {
        char current = str.items[i];
        printf("%c", current);
    }
    printf("\n");
    return;
}

static U64 StringSlice_to_int(StringSlice str)
{
    U64 result = 0;

    for (int i = 0; i < str.len; i++) {
        char current = str.items[i];
        if ((current < '0') || (current > '9')) {
            break;
        }
        result *= 10;
        result += current - (int)'0';
    }

    return result;
}

static inline U32 U32_from_char(char value)
{
    U32 out = 0;
    if (value < '0') {
        return out;
    } else if (value > '9') {
        return out;
    }

    return value - '0';
}

U32 map_get_width(String)
{
}

typedef struct {
    U32 width;
    U32 height;
    char *items;
} Map;

int main()
{
    clock_t start, end;
    start       = clock();
    Arena arena = {0};
    arena_init(&arena, Megabytes(1));
    String input = String_with_capacity(&arena, Kilobytes(200));
    FILE *file   = fopen(filename, "r");
    read_file_to_string(&input, file);

    U32 width = map_get_width(input);
    Map map   = Map_from_string(&arena, input);

    U64 result_part_1 = 0;
    U64 result_part_2 = 0;

    printf("\x1b[7m\x1b[32mresult part 1: %lu\x1b[0m\n", result_part_1);
    printf("\x1b[7m\x1b[32mresult part 2: %lu\x1b[0m\n", result_part_2);
    end       = clock();
    double dt = end - start;
    printf("time: %f ms\n", dt / 1000.0f);
    arena_free(&arena);

    return 0;
}

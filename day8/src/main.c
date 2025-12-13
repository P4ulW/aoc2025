#include <stdio.h>
#include "cbase/src/arena.c"
#include "cbase/src/string.c"
#include "cbase/src/array.c"

#define EXAMPLE 1

#if EXAMPLE == 1
#define FILENAME "test.txt"
#define NUM_BOXES 20
#define PAIRS 10
#else
#define FILENAME "input.txt"
#define NUM_BOXES 1000
#define PAIRS 1000
#endif

U32 FILE_read_to_string(FILE *file, String *str)
{
    U32 count = 0;
    for (U32 i = 0; i < str->cap; i++) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }

        String_push(str, (char)c);
        count++;
    }

    return count;
}

typedef struct {
    I32 x;
    I32 y;
    I32 z;
} Vec3;

I32 I32_from_sslice(StringSlice slice)
{
    I32 out = 0;
    for (U32 i = 0; i < slice.len; i++) {
        char c = slice.items[i];
        if ((c >= '0') && (c <= '9')) {
            out *= 10;
            out += (I32)(c - '0');
        } else {
            break;
        }
    }
    return out;
}

Vec3 Vec3_from_sslice(StringSlice slice)
{
    Vec3 out = {0};
    out.x    = I32_from_sslice(slice);

    U32 num = 2;
    for (U32 i = 0; i < slice.len; i++) {
        char c = slice.items[i];
        if (c != ',') {
            continue;
        } else {
            StringSlice next_num_slice = {
                slice.items + i + 1, slice.len - i - 1};
            if (num == 2) {
                out.y = I32_from_sslice(next_num_slice);
                num += 1;
            } else {
                out.z = I32_from_sslice(next_num_slice);
            }
        }
    }

    return out;
}

void Vec3_print(Vec3 p)
{
    printf("Vec3: (%3d, %3d, %3d)\n", p.x, p.y, p.z);
}

typedef struct Junction Junction;
struct Junction {
    F64 distance;
    U32 index_start;
    U32 index_end;
};

Array_Prototype(Vec3);
Array_Impl(Vec3);

Array_Prototype(Junction);
Array_Impl(Junction);

int main()
{

    Arena arena = {0};
    Arena_init(&arena, Megabytes(2));
    String input = String_with_capacity(&arena, Kilobytes(100));
    FILE *file   = fopen(FILENAME, "r");
    if (!file) {
        printf("could not open file!");
        return 1;
    }
    FILE_read_to_string(file, &input);
    fclose(file);

    StringSlice input_as_slice = {input.items, input.len};
    ArrayStringSlice lines =
        ArrayStringSlice_with_capacity(&arena, NUM_BOXES + 1);
    ArrayVec3 boxes = ArrayVec3_with_capacity(&arena, lines.cap);
    StringSlice_split_to_slices(&lines, input_as_slice, '\n');
    ArrayStringSlice_pop(&lines);

    for (U32 i = 0; i < lines.len; i++) {
        StringSlice line = ArrayStringSlice_get_value(&lines, i);
        // StringSlice_print(line);
        Vec3 box = Vec3_from_sslice(line);
        Vec3_print(box);
        ArrayVec3_push(&boxes, box);
    }

    Arena_free(&arena);
    return 0;
}

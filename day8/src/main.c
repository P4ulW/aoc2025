#include <stddef.h>
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
    for (size_t i = 0; i < str->cap; i++) {
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
    for (size_t i = 0; i < slice.len; i++) {
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
    for (size_t i = 0; i < slice.len; i++) {
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

static inline U64 Vec3_square_distance(Vec3 a, Vec3 b)
{
    U64 out = ((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)) +
              ((a.z - b.z) * (a.z - b.z));
    return out;
}

typedef struct Junction Junction;
struct Junction {
    U64 distance;
    size_t index_start;
    size_t index_end;
};

void Junction_print(Junction j)
{
    printf(
        "Junction: (%lu, %lu, %lu)\n", j.distance, j.index_start, j.index_end);
}

Array_Prototype(Vec3);
Array_Impl(Vec3);

Array_Prototype(Junction);
Array_Impl(Junction);

void ArrayJunction_sort_distance(ArrayJunction *junctions)
{
    if (junctions->len < 2) {
        return;
    }

    for (size_t pass = 0; pass < junctions->len - 1; pass++) {
        for (size_t i = 0; i < junctions->len - 1 - pass; i++) {
            Junction *current = ArrayJunction_get_ref(junctions, i);
            Junction *next    = ArrayJunction_get_ref(junctions, i + 1);
            if (current->distance > next->distance) {
                Junction temp = *current;
                *current      = *next;
                *next         = temp;
            }
        }
    }

    return;
}

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
    ArrayJunction junctions =
        ArrayJunction_with_capacity(&arena, NUM_BOXES * NUM_BOXES / 2);
    StringSlice_split_to_slices(&lines, input_as_slice, '\n');
    ArrayStringSlice_pop(&lines);

    for (size_t i = 0; i < lines.len; i++) {
        StringSlice line = ArrayStringSlice_get_value(&lines, i);
        // StringSlice_print(line);
        Vec3 box = Vec3_from_sslice(line);
        Vec3_print(box);
        ArrayVec3_push(&boxes, box);
    }

    for (size_t i = 0; i < NUM_BOXES - 1; i++) {
        for (size_t j = i + 1; j < NUM_BOXES; j++) {
            Vec3 b1           = ArrayVec3_get_value(&boxes, i);
            Vec3 b2           = ArrayVec3_get_value(&boxes, j);
            Junction junction = {
                .distance    = Vec3_square_distance(b1, b2),
                .index_start = i,
                .index_end   = j};
            ArrayJunction_push(&junctions, junction);
            // Junction_print(junction);
        }
    }
    ArrayJunction_sort_distance(&junctions);
    for (size_t i = 0; i < junctions.len; i++) {
        Junction j = ArrayJunction_get_value(&junctions, i);
        Junction_print(j);
    }

    Arena_free(&arena);
    return 0;
}

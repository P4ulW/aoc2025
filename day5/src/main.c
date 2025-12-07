#include <stdio.h>

#include "cbase/src/arena.c"
#include "cbase/src/array.c"
#include "cbase/src/string.c"

#define NUM_LINES 10000

char filename[] = "input.txt";

static U64 U64_from_stringslice(const StringSlice slice)
{
    U64 out = 0;
    for (U64 index = 0; index < slice.len; index++) {
        char current = slice.items[index];
        if ((current < '0') || (current > '9')) {
            break;
        }
        out *= 10;
        out += current - '0';
    }

    return out;
}

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

typedef struct {
    U64 start;
    U64 end;
} IdRange;

Array_Prototype(IdRange);
Array_Impl(IdRange);

static B32 IdRange_includes_U64(const IdRange range, const U64 id)
{
    B32 out = 1;
    if (id < range.start) {
        out = 0;
    } else if (id > range.end) {
        out = 0;
    }

    return out;
};

int main()
{
    Arena arena = {0};
    Arena_init(&arena, Megabytes(10));
    String input = String_with_capacity(&arena, Megabytes(1));
    FILE *file   = fopen(filename, "r");
    read_file_to_string(&input, file);
    fclose(file);
    String_print(input);
    StringSlice input_as_slice = {.items = input.items, .len = input.len};

    ArrayStringSlice lines = ArrayStringSlice_with_capacity(&arena, NUM_LINES);
    StringSlice_split_to_slices(&lines, input_as_slice, '\n');

    ArrayStringSlice range_slices =
        ArrayStringSlice_with_capacity(&arena, NUM_LINES);
    ArrayStringSlice ids = ArrayStringSlice_with_capacity(&arena, NUM_LINES);
    U32 is_range         = 1;
    for (U32 i = 0; i < lines.len; i++) {
        StringSlice line = ArrayStringSlice_get_value(&lines, i);
        if (line.len == 0) {
            is_range = 0;
            continue;
        }
        if (is_range) {
            ArrayStringSlice_push(&range_slices, line);
        } else {
            ArrayStringSlice_push(&ids, line);
        }
    }

    printf("ranges:\n");
    ArrayIdRange ranges = ArrayIdRange_with_capacity(&arena, NUM_LINES);
    StringSlice mem_id_range[2];
    for (U32 i = 0; i < range_slices.len; i++) {
        StringSlice range_slice = ArrayStringSlice_get_value(&range_slices, i);
        ArrayStringSlice id_range_slices = {
            .len = 0, .cap = 2, .items = (StringSlice *)&mem_id_range};
        StringSlice_split_to_slices(&id_range_slices, range_slice, '-');
        U64 id_start  = U64_from_stringslice(id_range_slices.items[0]);
        U64 id_end    = U64_from_stringslice(id_range_slices.items[1]);
        IdRange range = {.start = id_start, .end = id_end};
        printf("range: %lu - %lu\n", range.start, range.end);
        ArrayIdRange_push(&ranges, range);
    }

    U32 result_part_1 = 0;
    printf("\nIDs:\n");
    for (U32 i = 0; i < ids.len; i++) {
        StringSlice id_slice = ArrayStringSlice_get_value(&ids, i);
        StringSlice_print(id_slice);
        U64 id = U64_from_stringslice(id_slice);

        B32 is_included = 0;
        for (U32 i = 0; i < ranges.len; i++) {
            IdRange range = ArrayIdRange_get_value(&ranges, i);
            if (IdRange_includes_U64(range, id)) {
                printf("ID %lu inside %lu - %lu\n", id, range.start, range.end);
                is_included = 1;
                break;
            }
        }
        if (is_included) {
            result_part_1 += 1;
        }
    }

    printf("result_part_1: %u\n", result_part_1);

    Arena_free(&arena);
    return 0;
}

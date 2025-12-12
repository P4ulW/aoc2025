#include <stdio.h>
#include "cbase/src/ansi_codes.h"
#include "cbase/src/arena.c"
#include "cbase/src/string.c"

Array_Prototype(U32);
Array_Impl(U32);

const char filename[] = "test.txt";

void FILE_read_to_string(FILE *file, String *str)
{
    int current = 0;
    for (U32 i = 0; i < str->cap; i++) {
        current = fgetc(file);
        if (current == EOF) {
            break;
        }

        String_push(str, (char)current);
    }
    return;
}

B32 ArrayU32_contains(ArrayU32 arr, U32 value)
{
    for (U32 i = 0; i < arr.len; i++) {
        U32 current = ArrayU32_get_value(&arr, i);
        if (current == value) {
            return 1;
        }
    }
    return 0;
}

void ArrayU32_print(ArrayU32 arr)
{
    printf("ArrayU32 <" ANSI_TEXT_YELLOW);
    for (U32 i = 0; i < arr.len; i++) {
        U32 current = ArrayU32_get_value(&arr, i);
        printf("%u, ", current);
    }
    printf(ANSI_RESET ">, len: %u, cap: %u", arr.len, arr.cap);
    printf(ANSI_RESET "\n");
}

int main()
{
    Arena arena = {0};
    Arena_init(&arena, Megabytes(20));
    String input = String_with_capacity(&arena, Kilobytes(400));
    FILE *file   = fopen(filename, "r"); // FILE_read_to_string(file, &input);
    if (!file) {
        printf(
            ANSI_TEXT_B_RED "Could not open file \'%s\'\n" ANSI_RESET,
            filename);
        return 1;
    }
    FILE_read_to_string(file, &input);
    StringSlice input_as_slice = {.items = input.items, .len = input.len};
    ArrayStringSlice lines     = ArrayStringSlice_with_capacity(&arena, 2000);
    StringSlice_split_to_slices(&lines, input_as_slice, '\n');
    ArrayStringSlice_pop(&lines);

    ArrayU32 beams_curr = ArrayU32_with_capacity(&arena, 2000);
    ArrayU32 beams_prev = ArrayU32_with_capacity(&arena, 2000);

    {
        StringSlice line_first = ArrayStringSlice_get_value(&lines, 0);
        for (U32 i = 0; i < line_first.len; i++) {
            char current = line_first.items[i];
            if (current == 'S') {
                ArrayU32_push(&beams_prev, i);
            }
        }
        // StringSlice_print(line_first);
    }

    // part 1
    U32 result_part_1 = 0;
    for (U32 line_idx = 1; line_idx < lines.len; line_idx++) {
        StringSlice line = ArrayStringSlice_get_value(&lines, line_idx);

        // StringSlice_print(line);
        for (U32 i = 1; i < line.len - 1; i++) {
            char current       = line.items[i];
            B32 has_beam_input = ArrayU32_contains(beams_prev, i);

            if (!has_beam_input) {
                continue;
            }

            // printf("i %u current %c \n", i, current);

            if (current == '^') {
                ArrayU32_push(&beams_curr, i - 1);
                ArrayU32_push(&beams_curr, i + 1);
                result_part_1 += 1;

            } else {
                ArrayU32_push(&beams_curr, i);
            }
        }

        // ArrayU32_print(beams_curr);

        ArrayU32 _beams_curr = beams_curr;
        ArrayU32 _beams_prev = beams_prev;
        beams_prev           = _beams_curr;
        beams_curr           = _beams_prev;
        beams_curr.len       = 0;
    }

    // part 2
    U32 result_part_2 = 0;

    printf(ANSI_TEXT_GREEN "part 1: %u\n" ANSI_RESET, result_part_1);
    printf(ANSI_TEXT_GREEN "part 2: %u\n" ANSI_RESET, result_part_2);

    Arena_free(&arena);
    fclose(file);
    return 0;
}

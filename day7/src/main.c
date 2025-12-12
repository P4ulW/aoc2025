#include <stdio.h>
#include "cbase/src/ansi_codes.h"
#include "cbase/src/arena.c"
#include "cbase/src/string.c"

typedef struct {
    U32 index;
    U64 degeneracy;
} Beam;

Array_Prototype(Beam);
Array_Impl(Beam);

Array_Prototype(U32);
Array_Impl(U32);

const char filename[] = "input.txt";

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

U32 ArrayBeam_contains_with_index(ArrayBeam arr, U32 index)
{
    for (U32 i = 0; i < arr.len; i++) {
        Beam current = ArrayBeam_get_value(&arr, i);
        if (current.index == index) {
            return i;
        }
    }
    return 0;
}

void ArrayBeam_print(ArrayBeam arr)
{
    printf("ArrayBeam <" ANSI_TEXT_YELLOW);
    for (U32 i = 0; i < arr.len; i++) {
        Beam current = ArrayBeam_get_value(&arr, i);
        printf("[%u, %lu], ", current.index, current.degeneracy);
    }
    printf(ANSI_RESET ">, len: %u, cap: %u", arr.len, arr.cap);
    printf(ANSI_RESET "\n");
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

    // part 1
    {
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
        printf(ANSI_TEXT_GREEN "part 1: %u\n" ANSI_RESET, result_part_1);
    }

    // part 2
    {
        U64 result_part_2 = 0;

        ArrayBeam beams_curr = ArrayBeam_with_capacity(&arena, 2000);
        ArrayBeam beams_prev = ArrayBeam_with_capacity(&arena, 2000);

        {
            StringSlice line_first = ArrayStringSlice_get_value(&lines, 0);
            for (U32 i = 0; i < line_first.len; i++) {
                char current = line_first.items[i];
                if (current == 'S') {
                    Beam initial = {i, 1};
                    ArrayBeam_push(&beams_prev, initial);
                }
            }
            StringSlice_print(line_first);
        }

        for (U32 line_idx = 1; line_idx < lines.len; line_idx++) {
            StringSlice line = ArrayStringSlice_get_value(&lines, line_idx);
            StringSlice_print(line);

            for (U32 i = 0; i < beams_prev.len; i++) {
                Beam current = ArrayBeam_get_value(&beams_prev, i);
                char symbol  = line.items[current.index];

                if (symbol == '^') {
                    U32 index_left = ArrayBeam_contains_with_index(
                        beams_curr, current.index - 1);
                    U32 index_right = ArrayBeam_contains_with_index(
                        beams_curr, current.index + 1);

                    if (index_left) {
                        Beam *next = ArrayBeam_get_ref(&beams_curr, index_left);
                        next->degeneracy += current.degeneracy;
                    } else {
                        Beam next_left = {
                            current.index - 1, current.degeneracy};
                        ArrayBeam_push(&beams_curr, next_left);
                    }

                    if (index_right) {
                        Beam *next =
                            ArrayBeam_get_ref(&beams_curr, index_right);
                        next->degeneracy += current.degeneracy;
                    } else {
                        Beam next_right = {
                            current.index + 1, current.degeneracy};
                        ArrayBeam_push(&beams_curr, next_right);
                    }
                } else {
                    ArrayBeam_push(&beams_curr, current);
                }
            }
            ArrayBeam_print(beams_curr);

            ArrayBeam _beams_curr = beams_curr;
            ArrayBeam _beams_prev = beams_prev;
            beams_prev            = _beams_curr;
            beams_curr            = _beams_prev;
            beams_curr.len        = 0;
        }

        for (U32 i = 0; i < beams_prev.len; i++) {
            Beam current = ArrayBeam_get_value(&beams_prev, i);
            result_part_2 += current.degeneracy;
        }

        printf(ANSI_TEXT_GREEN "part 2: %lu\n" ANSI_RESET, result_part_2);
    }

    Arena_free(&arena);
    fclose(file);
    return 0;
}

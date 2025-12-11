#include <stdio.h>
#include "cbase/src/arena.c"
#include "cbase/src/array.c"
#include "cbase/src/string.c"

const char filename[] = "input.txt";

Array_Prototype(U64);
Array_Impl(U64);

// Array_Prototype(ArrayStringSlice);
// Array_Impl(ArrayStringSlice);

// ----------------------------------------------------- //
U64 StringSlice_at_index_to_U64(StringSlice slice, U64 index)
{
    B32 found_num = 0;
    U64 out       = 0;
    for (U32 i = index; i < slice.len; i++) {
        char current = slice.items[i];

        if (current == ' ') {
            if (found_num) {
                break;
            }
        } else {
            found_num = 1;
            out *= 10;
            out += current - '0';
        }
    }

    return out;
}

// ----------------------------------------------------- //
static void read_file_to_string(String *string, FILE *file)
{
    int c = getc(file);
    while (c != EOF) {
        String_push(string, (char)c);
        c = getc(file);
    }
    return;
}

// ----------------------------------------------------- //
int main()
{
    Arena arena;
    Arena_init(&arena, Megabytes(10));

    String input           = String_with_capacity(&arena, Kilobytes(100));
    FILE *file             = fopen(filename, "r");
    ArrayStringSlice lines = ArrayStringSlice_with_capacity(&arena, 10);
    read_file_to_string(&input, file);
    StringSlice input_as_slice = {.items = input.items, .len = input.len};
    StringSlice_split_to_slices(&lines, input_as_slice, '\n');

    for (U32 i = 0; i < lines.len; i++) {
        StringSlice line = ArrayStringSlice_get_value(&lines, i);
        // StringSlice_print(line);
    }

    U32 start_index = 0;
    StringSlice line_operators =
        ArrayStringSlice_get_value(&lines, lines.len - 2);
    // StringSlice_print(line_operators);

    U64 res_part_1 = 0;

    ArrayU64 nums = ArrayU64_with_capacity(&arena, 5);
    while (start_index < lines.items[0].len) {

        nums.len = 0;
        for (U32 i = 0; i < lines.len - 2; i++) {
            StringSlice line = ArrayStringSlice_get_value(&lines, i);
            U64 num          = StringSlice_at_index_to_U64(line, start_index);
            // printf("%10lu\n", num);
            ArrayU64_push(&nums, num);
        }

        char operator= line_operators.items[start_index];
        // printf("%10c\n----------\n", operator);

        U64 result = (operator== '+') ? 0 : 1;
        while (nums.len) {
            U64 num = ArrayU64_pop(&nums);
            if (operator== '+') {
                result += num;
            } else {
                result *= num;
            }
        }
        // printf("= %8lu\n\n", result);
        res_part_1 += result;

        start_index += 1;
        operator= line_operators.items[start_index];
        while ((operator== ' ') && (operator!= 0)) {
            start_index += 1;
            operator= line_operators.items[start_index];
        }
    }

    printf("res_part_1: %lu\n", res_part_1);
    return 0;
}

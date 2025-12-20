#include <stddef.h>
#include <stdio.h>
#include "cbase/src/ansi_codes.h"
#include "cbase/src/arena.c"
#include "cbase/src/array.c"
#include "cbase/src/string.c"
#include "cbase/src/file.c"

#define EXAMPLE 0

#if EXAMPLE == 0
#define FILENAME "test.txt"
#else
#define FILENAME "input.txt"
#endif

typedef struct Manual Manual;
struct Manual {
    StringSlice lights;
    StringSlice joltage_req;
    ArrayStringSlice wirings;
};

Array_Prototype(Manual);
Array_Impl(Manual);

Manual Manual_from_sslice(Arena *arena, StringSlice line)
{
    ArrayStringSlice parts = ArrayStringSlice_with_capacity(arena, 20);
    StringSlice_split_to_slices(&parts, line, ' ');
    Manual out      = {0};
    out.lights      = ArrayStringSlice_remove_at(&parts, 0);
    out.joltage_req = ArrayStringSlice_pop(&parts);
    out.wirings     = parts;
    return out;
}

void Manual_print(Manual manual)
{
    printf("Manual: <\nlight:       ");
    StringSlice_print(manual.lights);
    printf("joltage_req: ");
    StringSlice_print(manual.joltage_req);
    for (size_t i = 0; i < manual.wirings.len; i++) {
        if (i == 0) {
            printf("wiring:      ");
        } else {
            printf("             ");
        }

        StringSlice line = ArrayStringSlice_get_value(&manual.wirings, i);
        StringSlice_print(line);
    }
    printf(">\n");
}

int main()
{

    Arena arena = {0};
    Arena_init(&arena, Megabytes(1));
    String input           = String_with_capacity(&arena, Kilobytes(20));
    ArrayStringSlice lines = ArrayStringSlice_with_capacity(&arena, 300);
    FILE *file             = fopen(FILENAME, "rb");
    FILE_read_to_string(file, &input);
    fclose(file);
    StringSlice input_as_slice = {input.items, input.len};
    StringSlice_split_to_slices(&lines, input_as_slice, '\n');
    ArrayStringSlice_pop(&lines);

    ArrayStringSlice input_parts = ArrayStringSlice_with_capacity(&arena, 100);
    for (size_t i = 0; i < lines.len; i++) {
        StringSlice line = ArrayStringSlice_get_value(&lines, i);
        // StringSlice_print(line);
        Manual manual = Manual_from_sslice(&arena, line);
        Manual_print(manual);
    }

    return 0;
}

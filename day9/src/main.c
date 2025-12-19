#include <stddef.h>
#include <stdio.h>
#include "cbase/src/ansi_codes.h"
#include "cbase/src/arena.c"
#include "cbase/src/array.c"
#include "cbase/src/string.c"

#define EXAMPLE 0

#if EXAMPLE == 1
#define FILENAME "test.txt"
#else
#define FILENAME "input.txt"
#endif

typedef struct Tile Tile;
struct Tile {
    U32 x;
    U32 y;
};

Array_Prototype(Tile);
Array_Impl(Tile);

U32 U32_from_sslice(StringSlice slice)
{
    U32 out = 0;
    for (size_t i = 0; i < slice.len; i++) {
        char current = slice.items[i];
        if (current < '0' || current > '9') {
            break;
        }

        out *= 10;
        out += current - '0';
    }
    return out;
}

Tile Tile_from_sslice(StringSlice slice)
{
    U32 x = 0;
    U32 y = 0;

    StringSlice items[2]  = {0};
    ArrayStringSlice nums = {0, 2, (StringSlice *)&items};
    StringSlice_split_to_slices(&nums, slice, ',');
    x = U32_from_sslice(ArrayStringSlice_get_value(&nums, 0));
    y = U32_from_sslice(ArrayStringSlice_get_value(&nums, 1));

    Tile tile = {x, y};
    return tile;
}

void Tile_print(Tile tile)
{
    printf("Tile: %u %u\n", tile.x, tile.y);
}

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

U64 get_area_between_tiles(Tile tile_from, Tile tile_to)
{
    U64 area = 0;
    U32 xmin = (tile_from.x > tile_to.x) ? tile_to.x : tile_from.x;
    U32 xmax = (tile_from.x > tile_to.x) ? tile_from.x : tile_to.x;
    U32 ymin = (tile_from.y > tile_to.y) ? tile_to.y : tile_from.y;
    U32 ymax = (tile_from.y > tile_to.y) ? tile_from.y : tile_to.y;
    area     = (U64)(xmax - xmin + 1) * (U64)(ymax - ymin + 1);

    return area;
}

int main()
{

    Arena arena;
    Arena_init(&arena, Megabytes(1));

    String input = String_with_capacity(&arena, Kilobytes(20));
    FILE *file   = fopen(FILENAME, "rb");
    FILE_read_to_string(file, &input);
    fclose(file);

    StringSlice input_as_slice = {input.items, input.len};
    ArrayStringSlice lines     = ArrayStringSlice_with_capacity(&arena, 1000);
    StringSlice_split_to_slices(&lines, input_as_slice, '\n');
    ArrayStringSlice_pop(&lines);
    ArrayTile tiles = ArrayTile_with_capacity(&arena, lines.len);

    FILE *file_handle = fopen("points.csv", "w");
    for (size_t i = 0; i < lines.len; i++) {
        StringSlice line = lines.items[i];
        // StringSlice_print(line);
        Tile tile = Tile_from_sslice(line);
        Tile_print(tile);
        ArrayTile_push(&tiles, tile);
        fprintf(file_handle, "%u;%u\n", tile.x, tile.y);
    }
    fclose(file_handle);

    U64 max_area = 0;
    for (size_t i = 0; i < tiles.len - 1; i++) {
        Tile tile_from = ArrayTile_get_value(&tiles, i);
        for (size_t j = i + i; j < tiles.len; j++) {
            Tile tile_to = ArrayTile_get_value(&tiles, j);
            U64 area     = get_area_between_tiles(tile_from, tile_to);
            if (area > max_area) {
                max_area = area;
            }
        }
    }

    printf(ANSI_TEXT_GREEN "part 1: %lu\n", max_area);

    Arena_free(&arena);
    return 0;
}

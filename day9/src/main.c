#include <stddef.h>
#include <stdio.h>
#include "bits/types/siginfo_t.h"
#include "cbase/src/ansi_codes.h"
#include "cbase/src/arena.c"
#include "cbase/src/array.c"
#include "cbase/src/string.c"

#define EXAMPLE 1

#if EXAMPLE == 0
#define FILENAME "test.txt"
#else
#define FILENAME "input.txt"
#endif

typedef struct Tile Tile;
struct Tile {
    U32 x;
    U32 y;
};

typedef struct Rectangle Rectangle;
struct Rectangle {
    Tile c1;
    Tile c2;
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

static void Rectangle_normalize(Rectangle *self)
{
    U32 x1 = (self->c1.x < self->c2.x) ? self->c1.x : self->c2.x;
    U32 x2 = (self->c1.x < self->c2.x) ? self->c2.x : self->c1.x;

    U32 y1 = (self->c1.y < self->c2.y) ? self->c1.y : self->c2.y;
    U32 y2 = (self->c1.y < self->c2.y) ? self->c2.y : self->c1.y;

    self->c1.x = x1;
    self->c1.y = y1;
    self->c2.x = x2;
    self->c2.y = y2;
}

static B32 Rectangles_intersect(Rectangle rect, Rectangle other)
{
    Rectangle_normalize(&rect);
    Rectangle_normalize(&other);

    B32 a = rect.c2.x <= other.c1.x;
    if (a) {
        return 0;
    }
    B32 b = rect.c1.x >= other.c2.x;
    if (b) {
        return 0;
    }
    B32 c = rect.c2.y <= other.c1.y;
    if (c) {
        return 0;
    }
    B32 d = rect.c1.y >= other.c2.y;
    if (d) {
        return 0;
    }

    return 1;
}

static U32 Rectangle_intersects_with_others(ArrayTile tiles, Rectangle rect)
{

    for (size_t i = 0; i < tiles.len - 2; i++) {
        if (i + 2 == 248) {
            continue;
        }

        Tile tile_from  = ArrayTile_get_value(&tiles, i);
        Tile tile_to    = ArrayTile_get_value(&tiles, i + 2);
        Rectangle other = {tile_to, tile_from};
        if (Rectangles_intersect(rect, other)) {
            return 1;
        }
    }

    return 0;
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

    // FILE *file_handle = fopen("points.csv", "w");
    for (size_t i = 0; i < lines.len; i++) {
        StringSlice line = lines.items[i];
        // StringSlice_print(line);
        Tile tile = Tile_from_sslice(line);
        // Tile_print(tile);
        ArrayTile_push(&tiles, tile);
        // fprintf(file_handle, "%u;%u\n", tile.x, tile.y);
    }
    // fclose(file_handle);

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

    printf(ANSI_TEXT_GREEN "part 1: %lu\n" ANSI_RESET, max_area);

    max_area = 0;
    {
        // upper half
        const size_t k = 248;
        Tile tile_from = ArrayTile_get_value(&tiles, k);
        size_t i       = k - 200; // somewhere above
        for (; ArrayTile_get_value(&tiles, i).x < tile_from.x; i--);
        Tile tile_to = ArrayTile_get_value(&tiles, i);
        size_t j     = k - 2; // first on the circle before k
        U32 ymax     = tile_to.y;
        U32 xmin     = tile_to.x;

        Tile to_check = ArrayTile_get_value(&tiles, j);
        while (to_check.y <= ymax) {
            if (to_check.x >= xmin) {
                U64 area = get_area_between_tiles(tile_from, to_check);
                if (area > max_area) {
                    max_area = area;
                }
            }
            to_check = ArrayTile_get_value(&tiles, --j);
            xmin     = to_check.x;
        }
    }

    {
        // lower half
        const size_t k = 249;
        Tile tile_from = ArrayTile_get_value(&tiles, k);
        size_t i       = k + 200; // somewhere above
        for (; ArrayTile_get_value(&tiles, i).x < tile_from.x; i++);
        Tile tile_to = ArrayTile_get_value(&tiles, i);
        size_t j     = k + 2; // first on the circle before k
        U32 ymax     = tile_to.y;
        U32 xmin     = tile_to.x;

        Tile to_check = ArrayTile_get_value(&tiles, j);
        while (to_check.y >= ymax) {
            if (to_check.x >= xmin) {
                U64 area = get_area_between_tiles(tile_from, to_check);
                if (area > max_area) {
                    max_area = area;
                }
            }

            to_check = ArrayTile_get_value(&tiles, ++j);
            xmin     = to_check.x;
        }
    }

    printf(ANSI_TEXT_GREEN "part 2: %lu\n" ANSI_RESET, max_area);

    Arena_free(&arena);
    return 0;
}

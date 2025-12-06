#include <stdio.h>
#include <time.h>
#include "arena.c"
#include "array.c"
#include "string.c"
#include <unistd.h>
typedef struct timeval timeval;

Array_Prototype(StringSlice);
Array_Impl(StringSlice);

const char filename[] = "input.txt";

typedef enum {
    Up,
    Down,
    Left,
    Right,
    LeftUp,
    RightUp,
    LeftDown,
    RightDown,
} Direction;

Direction dirs[] = {
    Up, Down, Left, Right, LeftUp, RightUp, LeftDown, RightDown};

// -------------------------------------------- //
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

// -------------------------------------------- //
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

// -------------------------------------------- //
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

// -------------------------------------------- //
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

// -------------------------------------------- //
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

// -------------------------------------------- //
U32 map_get_width(String input)
{
    U32 width = 0;
    for (int i = 0; i < input.len; i++) {
        char current = input.items[i];
        if (current == '\n') {
            break;
        }
        width += 1;
    }
    return width;
}

typedef struct {
    U32 width;
    U32 height;
    char *items;
} Map;

// -------------------------------------------- //
Map Map_from_string(Arena *arena, String input)
{
    Map map     = {0};
    map.width   = map_get_width(input);
    map.items   = arena_alloc(arena, input.len);
    U32 height  = 0;
    U32 map_idx = 0;
    for (int i = 0; i < input.len; i++) {
        char current = input.items[i];
        if (current == '\n') {
            height += 1;
            continue;
        }

        map.items[map_idx] = current;
        map_idx += 1;
    }

    map.height = height;

    return map;
}

// -------------------------------------------- //
void Map_print(const Map map)
{
    for (int y = 0; y < map.width; y++) {
        for (int x = 0; x < map.height; x++) {
            U32 index    = x + y * map.width;
            char current = map.items[index];
            printf("%c", current);
        }
        printf("\n");
    }
}

// -------------------------------------------- //
U32 Map_check_if_inside(Map map, I32 index)
{
    if (index < 0) {
        return 0;
    } else if (index >= map.height * map.width) {
        return 0;
    }

    return 1;
}

// -------------------------------------------- //
U32 Map_get_neighbor_count(Map map, I32 tile_index)
{
    U32 count         = 0;
    U32 is_edge_left  = (tile_index % map.width == 0) ? 1 : 0;
    U32 is_edge_right = (tile_index % map.width == map.width - 1) ? 1 : 0;
    U32 is_edge_up    = (tile_index / map.width == 0) ? 1 : 0;
    U32 is_edge_down  = (tile_index / map.width == map.height - 1) ? 1 : 0;

    // printf(
    //     "tile_index: %d , left? %u, right? %u, up? %u, down? %u\n",
    //     tile_index,
    //     is_edge_left,
    //     is_edge_right,
    //     is_edge_up,
    //     is_edge_down);

    for (int i = 0; i < 8; i++) {
        Direction dir      = dirs[i];
        I32 tile_index_new = tile_index;

        switch (dir) {
            case Left:
                if (is_edge_left) {
                    continue;
                }
                tile_index_new -= 1;
                break;
            case Right:
                if (is_edge_right) {
                    continue;
                }
                tile_index_new += 1;
                break;
            case Up:
                if (is_edge_up) {
                    continue;
                }
                tile_index_new -= map.width;
                break;
            case Down:
                if (is_edge_down) {
                    continue;
                }
                tile_index_new += map.width;
                break;
            case LeftDown:
                if (is_edge_down || is_edge_left) {
                    continue;
                }
                tile_index_new += map.width;
                tile_index_new -= 1;
                break;
            case LeftUp:
                if (is_edge_up || is_edge_left) {
                    continue;
                }
                tile_index_new -= map.width;
                tile_index_new -= 1;
                break;
            case RightDown:
                if (is_edge_down || is_edge_right) {
                    continue;
                }
                tile_index_new += map.width;
                tile_index_new += 1;
                break;
            case RightUp:
                if (is_edge_up || is_edge_right) {
                    continue;
                }
                tile_index_new -= map.width;
                tile_index_new += 1;
                break;
        }

        // printf("tile_index_new: %d\n", tile_index_new);

        U32 inside = Map_check_if_inside(map, tile_index_new);
        if (!inside) {
            continue;
        }

        char tile = map.items[tile_index_new];

        if (tile != '.') {
            count += 1;
        }
    }

    char tile = map.items[tile_index];
    if (count < 4 && tile == '@') {
        tile = 'x';
    }

    // if (is_edge_right) {
    //     printf("\x1b[31m%c", tile);
    // } else if (is_edge_left) {
    //     printf("\x1b[32m%c", tile);
    // } else if (is_edge_up) {
    //     printf("\x1b[33m%c", tile);
    // } else if (is_edge_down) {
    //     printf("\x1b[34m%c", tile);
    // } else {
    //     printf("\x1b[0m%c", tile);
    // }
    //
    // if (is_edge_right) {
    //     printf("\n");
    // }

    if (tile == '.') {
        return UINT32_MAX;
    }

    // printf("count: %u\n\n", count);

    return count;
}

// -------------------------------------------- //
void Map_remove_loose_paper(Map map)
{
    for (I32 i = 0; i < map.width * map.height; i++) {
        char tile = map.items[i];
        if (tile == 'x') {
            map.items[i] = '.';
        }
    }
}

// -------------------------------------------- //
int main()
{
    U64 result_part_1 = 0;
    U64 result_part_2 = 0;
    clock_t start, end;
    start       = clock();
    Arena arena = {0};
    arena_init(&arena, Megabytes(1));
    String input = String_with_capacity(&arena, Kilobytes(200));
    FILE *file   = fopen(filename, "r");
    read_file_to_string(&input, file);

    U32 width = map_get_width(input);
    Map map   = Map_from_string(&arena, input);
    // Map_print(map);

    for (I32 i = 0; i < map.width * map.height; i++) {
        U32 neighbors = 0;
        char tile     = map.items[i];

        neighbors = Map_get_neighbor_count(map, i);
        if (neighbors < 4) {
            map.items[i] = 'x';
            result_part_1 += 1;
        }
    }

    Map_remove_loose_paper(map);

    U64 result_prev = 0;
    result_part_2   = result_part_1;

    while (result_part_2 != result_prev) {
        result_prev = result_part_2;

        // printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
        for (I32 i = 0; i < map.width * map.height; i++) {
            U32 neighbors = 0;
            char tile     = map.items[i];

            neighbors = Map_get_neighbor_count(map, i);
            if (neighbors < 4) {
                map.items[i] = 'x';
                result_part_2 += 1;
            }
        }
        Map_remove_loose_paper(map);
    }

    printf("\x1b[7m\x1b[32mresult part 1: %lu\x1b[0m\n", result_part_1);
    printf("\x1b[7m\x1b[32mresult part 2: %lu\x1b[0m\n", result_part_2);
    end       = clock();
    double dt = end - start;
    printf("time: %f ms\n", dt / 1000.0f);
    arena_free(&arena);

    return 0;
}

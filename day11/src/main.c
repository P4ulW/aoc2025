#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "cbase/src/ansi_codes.h"
#include "cbase/src/arena.c"
#include "cbase/src/array.c"
#include "cbase/src/string.c"
#include "cbase/src/file.c"

#define EXAMPLE 0

#if EXAMPLE == 1
#define FILENAME "test.txt"
#else
#define FILENAME "input.txt"
#endif

Array_Prototype(U32);
Array_Impl(U32);

typedef struct Node Node;
struct Node {
    StringSlice label;
    ArrayU32 connections;
    U32 visist;
};
Array_Prototype(Node);
Array_Impl(Node);

// ----------------------------------------------------- //
Node Node_create(Arena *arena, StringSlice label)
{
    Node result        = {};
    result.label       = label;
    result.connections = ArrayU32_with_capacity(arena, 20);
    result.visist      = 0;
    return result;
}

// ----------------------------------------------------- //
U32 ArrayNode_get_index_of_label(const ArrayNode nodes, const StringSlice label)
{
    U32 index = UINT32_MAX;
    for (size_t i = 0; i < nodes.len; i++) {
        Node node = ArrayNode_get_value(&nodes, i);
        if (!StringSlice_equal(node.label, label)) {
            continue;
        }

        index = i;
    }
    return index;
}

// ----------------------------------------------------- //
int main()
{
    Arena arena;
    Arena_init(&arena, Megabytes(10));

    String input           = String_with_capacity(&arena, Kilobytes(512));
    ArrayStringSlice lines = ArrayStringSlice_with_capacity(&arena, 1024);
    {
        FILE *file = fopen(FILENAME, "r");
        FILE_read_to_string(file, &input);
        fclose(file);
        StringSlice input_as_slice = {input.items, input.len};
        StringSlice_split_to_slices(&lines, input_as_slice, '\n');
        ArrayStringSlice_pop(&lines);
        // for (size_t i = 0; i < lines.len; i++) {
        //     StringSlice line = ArrayStringSlice_get_value(&lines, i);
        //     StringSlice_print(line);
        // }
    }

    ArrayNode nodes = ArrayNode_with_capacity(&arena, lines.len);
    for (size_t i = 0; i < lines.len; i++) {
        ArrayStringSlice parts = ArrayStringSlice_with_capacity(&arena, 2);
        ArrayStringSlice connections =
            ArrayStringSlice_with_capacity(&arena, 24);
        StringSlice line = ArrayStringSlice_get_value(&lines, i);
        StringSlice_split_to_slices(&parts, line, ':');
        parts.items[1].items += 1; // NOTE: to skip first white space
        parts.items[1].len -= 1;   // NOTE: to fix length
        StringSlice_split_to_slices(&connections, parts.items[1], ' ');
        // for (size_t j = 0; j < connections.len; j++) {
        //     StringSlice conn = ArrayStringSlice_get_value(&connections, j);
        //     StringSlice_print(conn);
        //     printf("\n");
        // }

        StringSlice label      = parts.items[0];
        U32 node_index_current = ArrayNode_get_index_of_label(nodes, label);
        if (node_index_current == UINT32_MAX) {
            // need to create a new node
            Node node_new = Node_create(&arena, label);
            ArrayNode_push(&nodes, node_new);
            node_index_current = ArrayNode_get_index_of_label(nodes, label);
        }
        assert(node_index_current != UINT32_MAX);
        Node *node_current = ArrayNode_get_ref(&nodes, node_index_current);

        for (size_t j = 0; j < connections.len; j++) {
            StringSlice label_conn =
                ArrayStringSlice_get_value(&connections, j);
        }
    }

    Arena_free(&arena);
    return 0;
}

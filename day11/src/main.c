#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "bits/types/struct_timeval.h"
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

typedef struct {
    StringSlice *items;
    U32 head;
    U32 tail;
    U32 cap;
    U32 size;
} QueueSSlice;

typedef struct Node Node;
struct Node {
    StringSlice label;
    ArrayU32 connections;
    U32 visist;
};
Array_Prototype(Node);
Array_Impl(Node);

// ----------------------------------------------------- //
QueueSSlice QueueSSlice_new(Arena *arena, U32 capacity)
{
    QueueSSlice q = {0};
    q.items       = Arena_alloc(arena, sizeof(StringSlice) * capacity);
    q.cap         = capacity;
    q.size        = 0;
    q.tail        = 0;
    q.head        = 0;
    return q;
}

// ----------------------------------------------------- //
void QueueSSlice_push(QueueSSlice *q, StringSlice slice)
{
    if (q->size == q->cap) {
        printf("QueueSSlice is full!, cannot push!\n");
        return;
    }
    q->items[q->head] = slice;
    q->head           = (q->head + 1) % q->cap;
    q->size += 1;
    return;
}

// ----------------------------------------------------- //
StringSlice QueueSSlice_pop(QueueSSlice *q)
{
    StringSlice out = {0};
    if (q->size == 0) {
        printf("QueueSSlice is empty!, cannot pop!\n");
        return out;
    }
    out     = q->items[q->tail];
    q->tail = (q->tail + 1) % q->cap;
    q->size -= 1;
    return out;
}

// ----------------------------------------------------- //
Node Node_create(Arena *arena, StringSlice label)
{
    Node result        = {};
    result.label       = label;
    result.connections = ArrayU32_with_capacity(arena, 50);
    result.visist      = 0;
    return result;
}

// ----------------------------------------------------- //
U32 ArrayNode_get_index_of_label(const ArrayNode nodes, const StringSlice label)
{
    U32 index = UINT32_MAX;
    for (size_t i = 0; i < nodes.len; i++) {
        Node node = ArrayNode_get_value(&nodes, i);
        if (StringSlice_equal(node.label, label)) {
            index = i;
            break;
        }
    }
    return index;
}

// ----------------------------------------------------- //
void ArrayNode_print(const ArrayNode nodes)
{
    for (size_t i = 0; i < nodes.len; i++) {
        Node node         = ArrayNode_get_value(&nodes, i);
        StringSlice label = node.label;
        printf("<" ANSI_TEXT_YELLOW);
        for (size_t j = 0; j < label.len; j++) {
            char c = label.items[j];
            printf("%c", c);
        }
        printf(ANSI_RESET "> -> ");

        for (size_t index_link = 0; index_link < node.connections.len;
             index_link++) {
            U32 node_index = ArrayU32_get_value(&node.connections, index_link);
            Node node_link = ArrayNode_get_value(&nodes, node_index);

            StringSlice label = node_link.label;
            printf("<" ANSI_TEXT_YELLOW);
            for (size_t j = 0; j < label.len; j++) {
                char c = label.items[j];
                printf("%c", c);
            }
            printf(ANSI_RESET "> ");
        }

        printf("visits: %u", node.visist);

        printf("\n");
    }
}

// ----------------------------------------------------- //
void fill_nodes_from_lines(
    Arena *arena, ArrayNode *nodes, ArrayStringSlice lines)
{
    for (size_t i = 0; i < lines.len; i++) {
        ArrayStringSlice parts = ArrayStringSlice_with_capacity(arena, 2);
        ArrayStringSlice connections =
            ArrayStringSlice_with_capacity(arena, 24);
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
        U32 node_index_current = ArrayNode_get_index_of_label(*nodes, label);
        if (node_index_current == UINT32_MAX) {
            // need to create a new node
            // printf("created node ");
            // StringSlice_print(label);
            Node node_new = Node_create(arena, label);
            ArrayNode_push(nodes, node_new);
            node_index_current = ArrayNode_get_index_of_label(*nodes, label);
        }
        assert(node_index_current != UINT32_MAX);
        Node *node_current = ArrayNode_get_ref(nodes, node_index_current);

        for (size_t j = 0; j < connections.len; j++) {
            StringSlice label_conn =
                ArrayStringSlice_get_value(&connections, j);

            U32 node_index_conn =
                ArrayNode_get_index_of_label(*nodes, label_conn);
            if (node_index_conn == UINT32_MAX) {
                Node node_conn_new = Node_create(arena, label_conn);
                // printf("added node ");
                // StringSlice_print(label_conn);
                ArrayNode_push(nodes, node_conn_new);
                node_index_conn =
                    ArrayNode_get_index_of_label(*nodes, label_conn);
            }

            assert(node_index_conn != UINT32_MAX);
            ArrayU32_push(&node_current->connections, node_index_conn);
        }
    }
}

// ----------------------------------------------------- //
void bfs_traverse_nodes(Arena *arena, ArrayNode nodes, Node *root)
{
    String _label_end     = String_from_cstring("out");
    StringSlice label_end = {_label_end.items, _label_end.len};
    String _label_you     = String_from_cstring("you");
    StringSlice label_you = {_label_you.items, _label_you.len};

    Temp temp     = Temp_start(arena);
    QueueSSlice q = QueueSSlice_new(temp.arena, 1000);

    for (size_t i = 0; i < root->connections.len; i++) {
        U32 index_node_next    = ArrayU32_get_value(&root->connections, i);
        Node node_next         = ArrayNode_get_value(&nodes, index_node_next);
        StringSlice label_next = node_next.label;
        QueueSSlice_push(&q, label_next);
    }

    while (q.size) {
        StringSlice label_curr = QueueSSlice_pop(&q);
        U32 index_node_curr = ArrayNode_get_index_of_label(nodes, label_curr);
        assert(index_node_curr != UINT32_MAX);
        Node *node_curr = ArrayNode_get_ref(&nodes, index_node_curr);
        node_curr->visist += 1;

        if (StringSlice_equal(label_curr, label_end)) {
            continue;
        }

        for (size_t i = 0; i < node_curr->connections.len; i++) {
            U32 index_node_next =
                ArrayU32_get_value(&node_curr->connections, i);
            Node node_next = ArrayNode_get_value(&nodes, index_node_next);
            StringSlice label_next = node_next.label;
            QueueSSlice_push(&q, label_next);
        }
    }

    Temp_end(temp);
    return;
}

// ----------------------------------------------------- //
int main()
{
    Arena arena;
    Arena_init(&arena, Megabytes(10));

    String input           = String_with_capacity(&arena, Kilobytes(1024));
    ArrayStringSlice lines = ArrayStringSlice_with_capacity(&arena, 1024);
    {
        FILE *file = fopen(FILENAME, "r");
        FILE_read_to_string(file, &input);
        fclose(file);
        StringSlice input_as_slice = {input.items, input.len};
        StringSlice_split_to_slices(&lines, input_as_slice, '\n');
        ArrayStringSlice_pop(&lines);
    }
    printf("lines: %u\n", lines.len);

    ArrayNode nodes = ArrayNode_with_capacity(&arena, lines.len + 10);
    fill_nodes_from_lines(&arena, &nodes, lines);

    String _label_you     = String_from_cstring("you");
    StringSlice label_you = {_label_you.items, _label_you.len};
    String _label_end     = String_from_cstring("out");
    StringSlice label_end = {_label_end.items, _label_end.len};
    U32 index_node_you    = ArrayNode_get_index_of_label(nodes, label_you);
    Node *node_you        = ArrayNode_get_ref(&nodes, index_node_you);
    bfs_traverse_nodes(&arena, nodes, node_you);

    U32 index_node_end = ArrayNode_get_index_of_label(nodes, label_end);
    Node *node_end     = ArrayNode_get_ref(&nodes, index_node_end);
    printf("result part 1: %u\n", node_end->visist);

    Arena_free(&arena);
    return 0;
}

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "cbase/src/arena.c"
#include "cbase/src/string.c"
#include "cbase/src/array.c"
#include "cbase/src/ansi_codes.h"

#define EXAMPLE 0

#if EXAMPLE == 1
#define FILENAME "test.txt"
#define NUM_BOXES 20
#define PAIRS 10
#else
#define FILENAME "input.txt"
#define NUM_BOXES 1000
#define PAIRS 1000
#endif

typedef struct {
    I32 x;
    I32 y;
    I32 z;
} Vec3;

typedef size_t CircuitId;

typedef struct Junction Junction;
struct Junction {
    U64 distance;
    size_t index_start;
    size_t index_end;
};

Array_Prototype(CircuitId);
Array_Impl(CircuitId);

Array_Prototype(Vec3);
Array_Impl(Vec3);

Array_Prototype(Junction);
Array_Impl(Junction);

typedef ArrayCircuitId Circuit;

Array_Prototype(U64);
Array_Impl(U64);

Array_Prototype(Circuit);
Array_Impl(Circuit);

// ----------------------------------------------------- //
static U32 FILE_read_to_string(FILE *file, String *str)
{
    U32 count = 0;
    for (size_t i = 0; i < str->cap; i++) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }

        String_push(str, (char)c);
        count++;
    }

    return count;
}

// ----------------------------------------------------- //
static I32 I32_from_sslice(StringSlice slice)
{
    I32 out = 0;
    for (size_t i = 0; i < slice.len; i++) {
        char c = slice.items[i];
        if ((c >= '0') && (c <= '9')) {
            out *= 10;
            out += (I32)(c - '0');
        } else {
            break;
        }
    }
    return out;
}

// ----------------------------------------------------- //
static Vec3 Vec3_from_sslice(StringSlice slice)
{
    Vec3 out = {0};
    out.x    = I32_from_sslice(slice);

    U32 num = 2;
    for (size_t i = 0; i < slice.len; i++) {
        char c = slice.items[i];
        if (c != ',') {
            continue;
        } else {
            StringSlice next_num_slice = {
                slice.items + i + 1, slice.len - i - 1};
            if (num == 2) {
                out.y = I32_from_sslice(next_num_slice);
                num += 1;
            } else {
                out.z = I32_from_sslice(next_num_slice);
            }
        }
    }

    return out;
}

// ----------------------------------------------------- //
static void Vec3_print(Vec3 p)
{
    printf("Vec3: (%3d, %3d, %3d)\n", p.x, p.y, p.z);
}

// ----------------------------------------------------- //
static inline U64 Vec3_square_distance(Vec3 a, Vec3 b)
{
    U64 out = ((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)) +
              ((a.z - b.z) * (a.z - b.z));
    return out;
}

// ----------------------------------------------------- //
static void Junction_print(Junction j)
{
    printf(
        "Junction: (%lu, %lu, %lu)\n", j.distance, j.index_start, j.index_end);
}

// ----------------------------------------------------- //
static void ArrayJunction_sort_distance(ArrayJunction *junctions)
{
    if (junctions->len < 2) {
        return;
    }

    for (size_t pass = 0; pass < junctions->len - 1; pass++) {
        for (size_t i = 0; i < junctions->len - 1 - pass; i++) {
            Junction *current = ArrayJunction_get_ref(junctions, i);
            Junction *next    = ArrayJunction_get_ref(junctions, i + 1);
            if (current->distance > next->distance) {
                Junction temp = *current;
                *current      = *next;
                *next         = temp;
            }
        }
    }

    return;
}

int Junction_order_asc(const void *a, const void *b)
{
    Junction ja = *(Junction *)a;
    Junction jb = *(Junction *)b;
    return ja.distance > jb.distance ? 1 : -1;
}

// ----------------------------------------------------- //
static ArrayJunction get_sorted_junctions(Arena *arena, ArrayVec3 boxes)
{

    ArrayJunction junctions =
        ArrayJunction_with_capacity(arena, NUM_BOXES * NUM_BOXES / 2);
    for (size_t i = 0; i < NUM_BOXES - 1; i++) {
        for (size_t j = i + 1; j < NUM_BOXES; j++) {
            Vec3 b1           = ArrayVec3_get_value(&boxes, i);
            Vec3 b2           = ArrayVec3_get_value(&boxes, j);
            Junction junction = {
                .distance    = Vec3_square_distance(b1, b2),
                .index_start = i,
                .index_end   = j};
            ArrayJunction_push(&junctions, junction);
        }
    }

    printf("num junctions: %lu", junctions.len);
    printf("sorting...\n");
    qsort(junctions.items, junctions.len, sizeof(Junction), Junction_order_asc);
    // ArrayJunction_sort_distance(&junctions);
    return junctions;
}

// ----------------------------------------------------- //
void ArrayCircuit_create_circuit(
    Arena *arena,
    ArrayCircuit *circuits,
    ArrayCircuitId *ids,
    Junction junction)
{
    Circuit new = ArrayCircuitId_with_capacity(arena, PAIRS);
    ArrayCircuitId_push(&new, junction.index_start);
    ArrayCircuitId_push(&new, junction.index_end);
    ArrayCircuit_push(circuits, new);
    CircuitId *id_start = ArrayCircuitId_get_ref(ids, junction.index_start);
    CircuitId *id_end   = ArrayCircuitId_get_ref(ids, junction.index_end);
    *id_start           = circuits->len;
    *id_end             = circuits->len;
    return;
}

// ----------------------------------------------------- //
void ArrayCircuit_expand_circuit(
    ArrayCircuit *circuits,
    ArrayCircuitId *ids,
    CircuitId circuit_id,
    size_t box_id)
{
    ArrayCircuitId *circuit = ArrayCircuit_get_ref(
        circuits, circuit_id - 1); // id i is item it index i-1
    ArrayCircuitId_push(circuit, box_id);
    CircuitId *id = ArrayCircuitId_get_ref(ids, box_id);
    *id           = circuit_id;
    return;
}

// ----------------------------------------------------- //
void ArrayCircuit_merge_circuits(
    ArrayCircuit *circuits,
    ArrayCircuitId *circuit_ids,
    CircuitId c_id_start,
    CircuitId c_id_end)
{
    CircuitId dest_id = (c_id_start < c_id_end) ? c_id_start : c_id_end;
    CircuitId src_id  = (c_id_start < c_id_end) ? c_id_end : c_id_start;

    ArrayCircuitId *circuit_ids_dest =
        ArrayCircuit_get_ref(circuits, dest_id - 1);
    ArrayCircuitId *circuit_ids_src =
        ArrayCircuit_get_ref(circuits, src_id - 1);
    while (circuit_ids_src->len) {
        CircuitId to_move = ArrayCircuitId_pop(circuit_ids_src);
        ArrayCircuitId_push(circuit_ids_dest, to_move);
    }
    ArrayCircuit_remove_at(circuits, src_id - 1);

    for (size_t i = 0; i < circuit_ids->len; i++) {
        CircuitId *id = ArrayCircuitId_get_ref(circuit_ids, i);
        if (*id == src_id) {
            *id = dest_id;
        } else if (*id > src_id) {
            *id -= 1;
        }
    }
    return;
}

int U64_order_desc(const void *a, const void *b)
{
    U64 num_a = *(U64 *)a;
    U64 num_b = *(U64 *)b;
    return num_a > num_b ? -1 : 1;
}

int main()
{

    Arena arena = {0};
    Arena_init(&arena, Megabytes(20));
    String input = String_with_capacity(&arena, Kilobytes(400));
    FILE *file   = fopen(FILENAME, "r");
    if (!file) {
        printf("could not open file!");
        return 1;
    }
    FILE_read_to_string(file, &input);
    fclose(file);

    StringSlice input_as_slice = {input.items, input.len};
    ArrayStringSlice lines =
        ArrayStringSlice_with_capacity(&arena, NUM_BOXES + 1);
    ArrayVec3 boxes = ArrayVec3_with_capacity(&arena, lines.cap);
    StringSlice_split_to_slices(&lines, input_as_slice, '\n');
    ArrayStringSlice_pop(&lines);

    for (size_t i = 0; i < lines.len; i++) {
        StringSlice line = ArrayStringSlice_get_value(&lines, i);
        // StringSlice_print(line);
        Vec3 box = Vec3_from_sslice(line);
        // Vec3_print(box);
        ArrayVec3_push(&boxes, box);
    }

    ArrayJunction junctions = get_sorted_junctions(&arena, boxes);
    for (size_t i = 0; i < junctions.len; i++) {
        Junction j = ArrayJunction_get_value(&junctions, i);
        // Junction_print(j);
    }

    ArrayCircuitId circuit_ids =
        ArrayCircuitId_with_capacity(&arena, NUM_BOXES);
    for (size_t i = 0; i < circuit_ids.cap; i++) {
        ArrayCircuitId_push(&circuit_ids, 0);
    }

    printf(ANSI_BG_RED ANSI_TEXT_BLACK "starting ..." ANSI_RESET "\n");
    ArrayCircuit circuits = ArrayCircuit_with_capacity(&arena, PAIRS);
    for (size_t i = 0; i < PAIRS; i++) {
        Junction junction = ArrayJunction_get_value(&junctions, i);
        size_t c_id_start =
            ArrayCircuitId_get_value(&circuit_ids, junction.index_start);
        size_t c_id_end =
            ArrayCircuitId_get_value(&circuit_ids, junction.index_end);
        // Junction_print(junction);
        // printf("c_id_start: %lu, c_id_end %lu\n", c_id_start, c_id_end);

        if (!c_id_start && !c_id_end) {
            // printf("creating new circuit\n");
            ArrayCircuit_create_circuit(
                &arena, &circuits, &circuit_ids, junction);
        } else if (!c_id_end) {
            // printf("expanding circuit %lu\n", c_id_start);
            ArrayCircuit_expand_circuit(
                &circuits, &circuit_ids, c_id_start, junction.index_end);
        } else if (!c_id_start) {
            // printf("expanding circuit %lu\n", c_id_end);
            ArrayCircuit_expand_circuit(
                &circuits, &circuit_ids, c_id_end, junction.index_start);
        } else if (c_id_end != c_id_start) {
            // printf(
            //     ANSI_BG_RED "merging circuit %lu and %lu" ANSI_RESET "\n",
            //     c_id_start,
            //     c_id_end);
            ArrayCircuit_merge_circuits(
                &circuits, &circuit_ids, c_id_start, c_id_end);
        }

        // printf("Circuits : %lu\n", circuits.len);
        // for (size_t i = 0; i < circuits.len; i++) {
        //     Circuit current = ArrayCircuit_get_value(&circuits, i);
        //     printf("Circuit %lu: <", i + 1);
        //     for (size_t j = 0; j < current.len; j++) {
        //         CircuitId id = ArrayCircuitId_get_value(&current, j);
        //         printf("%lu, ", id);
        //     }
        //     printf("> len: %lu\n", current.len);
        // }
        // for (size_t i = 0; i < circuit_ids.len; i++) {
        //     CircuitId id = ArrayCircuitId_get_value(&circuit_ids, i);
        //     printf("<%lu, %lu>  ", i, id);
        // }
        // printf("\n-------\n\n");
    }

    ArrayU64 circuit_sizes = ArrayU64_with_capacity(&arena, circuits.len);
    for (size_t i = 0; i < circuits.len; i++) {
        Circuit current = ArrayCircuit_get_value(&circuits, i);
        ArrayU64_push(&circuit_sizes, current.len);
    }

    qsort(circuit_sizes.items, circuit_sizes.len, sizeof(U64), U64_order_desc);
    U64 result_part_1 = circuit_sizes.items[0] * circuit_sizes.items[1] *
                        circuit_sizes.items[2];
    printf(ANSI_TEXT_B_RED "result_part_1: %lu\n" ANSI_RESET, result_part_1);

    Arena_free(&arena);
    return 0;
}

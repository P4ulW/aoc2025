#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "cbase/src/arena.c"
#include "cbase/src/string.c"
#include "cbase/src/array.c"
#include "cbase/src/ansi_codes.h"
#include <time.h>

#define SECONDS_TO_MICROSECONDS 1000000.0
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
    I64 x;
    I64 y;
    I64 z;
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
static inline U64 Vec3_squared(const Vec3 a)
{
    return a.x * a.x + a.y * a.y + a.z * a.z;
}

static inline Vec3 Vec3_sub(const Vec3 a, const Vec3 b)
{
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

// ----------------------------------------------------- //
static inline U64 Vec3_square_distance(Vec3 a, Vec3 b)
{
    return Vec3_squared(Vec3_sub(a, b));
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
int ArrayCircuit_create_circuit(
    Arena *arena,
    ArrayCircuit *circuits,
    ArrayCircuitId *ids,
    Junction junction)
{
    Circuit new = ArrayCircuitId_with_capacity(arena, NUM_BOXES);
    ArrayCircuitId_push(&new, junction.index_start);
    ArrayCircuitId_push(&new, junction.index_end);
    ArrayCircuit_push(circuits, new);
    CircuitId *id_start = ArrayCircuitId_get_ref(ids, junction.index_start);
    CircuitId *id_end   = ArrayCircuitId_get_ref(ids, junction.index_end);
    *id_start           = circuits->len;
    *id_end             = circuits->len;
    return 2;
}

// ----------------------------------------------------- //
int ArrayCircuit_expand_circuit(
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
    return circuit->len;
}

// ----------------------------------------------------- //
int ArrayCircuit_merge_circuits(
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
    // ArrayCircuit_remove_at(circuits, src_id - 1);

    for (size_t i = 0; i < circuit_ids->len; i++) {
        CircuitId *id = ArrayCircuitId_get_ref(circuit_ids, i);
        if (*id == src_id) {
            *id = dest_id;
        }
        // else if (*id > src_id) {
        // *id -= 1;
        // }
    }
    return (int)circuit_ids_dest->len;
}

int U64_order_desc(const void *a, const void *b)
{
    U64 num_a = *(U64 *)a;
    U64 num_b = *(U64 *)b;
    return num_a > num_b ? -1 : 1;
}

U64 add_junctions(
    Arena *arena,
    ArrayJunction *junctions,
    ArrayCircuit *circuits,
    ArrayCircuitId *circuit_ids,
    ArrayVec3 *boxes,
    size_t start,
    size_t end)
{
    int size = 0;
    size_t i = start;
    for (; (i < end) && (size != NUM_BOXES); i++) {
        Junction junction = ArrayJunction_get_value(junctions, i);
        size_t c_id_start =
            ArrayCircuitId_get_value(circuit_ids, junction.index_start);
        size_t c_id_end =
            ArrayCircuitId_get_value(circuit_ids, junction.index_end);
        // Junction_print(junction);

        if (!c_id_start && !c_id_end) {
            // printf("creating circuit %lu\n", circuits->len + 1);
            size = ArrayCircuit_create_circuit(
                arena, circuits, circuit_ids, junction);
        } else if (!c_id_end) {
            // printf("adding %lu circuit %lu\n", junction.index_end,
            // c_id_start);
            size = ArrayCircuit_expand_circuit(
                circuits, circuit_ids, c_id_start, junction.index_end);
        } else if (!c_id_start) {
            // printf("adding %lu circuit %lu\n", junction.index_start,
            // c_id_end);
            size = ArrayCircuit_expand_circuit(
                circuits, circuit_ids, c_id_end, junction.index_start);
        } else if (c_id_end != c_id_start) {
            // printf(
            //     ANSI_BG_RED "merging circuit %lu %lu" ANSI_RESET "\n",
            //     c_id_start,
            //     c_id_end);
            size = ArrayCircuit_merge_circuits(
                circuits, circuit_ids, c_id_start, c_id_end);
        }
    }

    if (size == NUM_BOXES) {
        Junction last_added = ArrayJunction_get_value(junctions, i - 1);
        Vec3 box1 = ArrayVec3_get_value(boxes, last_added.index_start);
        Vec3 box2 = ArrayVec3_get_value(boxes, last_added.index_end);
        return box1.x * box2.x;
    } else {
        return 0;
    }
}

int main()
{
    struct timespec start, end;
    double elapsed_time_us;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        perror("clock_gettime (start)");
        return 1;
    }

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
        Vec3 box         = Vec3_from_sslice(line);
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

    ArrayCircuit circuits = ArrayCircuit_with_capacity(&arena, PAIRS);
    printf(ANSI_BG_RED ANSI_TEXT_BLACK "starting ..." ANSI_RESET "\n");
    add_junctions(
        &arena, &junctions, &circuits, &circuit_ids, &boxes, 0, PAIRS);

    ArrayU64 circuit_sizes = ArrayU64_with_capacity(&arena, circuits.len);
    for (size_t i = 0; i < circuits.len; i++) {
        Circuit current = ArrayCircuit_get_value(&circuits, i);
        ArrayU64_push(&circuit_sizes, current.len);
    }

    qsort(circuit_sizes.items, circuit_sizes.len, sizeof(U64), U64_order_desc);
    U64 result_part_1 = circuit_sizes.items[0] * circuit_sizes.items[1] *
                        circuit_sizes.items[2];
    printf(ANSI_TEXT_B_RED "result_part_1: %lu\n" ANSI_RESET, result_part_1);

    U64 result_part_2 = add_junctions(
        &arena,
        &junctions,
        &circuits,
        &circuit_ids,
        &boxes,
        PAIRS,
        NUM_BOXES * NUM_BOXES / 2);
    printf(ANSI_TEXT_B_RED "result_part_2: %lu\n" ANSI_RESET, result_part_2);

    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
        perror("clock_gettime (end)");
        return 1;
    }
    double sec_diff = (double)(end.tv_sec - start.tv_sec);
    double nsec_diff_to_sec =
        (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;
    double total_time_sec = sec_diff + nsec_diff_to_sec;
    elapsed_time_us       = total_time_sec * SECONDS_TO_MICROSECONDS;

    printf("Execution took: %.2f milliseconds (ms)\n", elapsed_time_us / 1000);

    Arena_free(&arena);
    return 0;
}

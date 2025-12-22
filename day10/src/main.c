#include <complex.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "bits/types/siginfo_t.h"
#include "cbase/src/ansi_codes.h"
#include "cbase/src/arena.c"
#include "cbase/src/array.c"
#include "cbase/src/string.c"
#include "cbase/src/file.c"

#define EXAMPLE 1

#if EXAMPLE == 1
#define FILENAME "test.txt"
#else
#define FILENAME "input.txt"
#endif

Array_Prototype(U32);
Array_Impl(U32);

typedef ArrayU32 Wiring;

Array_Prototype(Wiring);
Array_Impl(Wiring);

typedef struct Manual Manual;
struct Manual {
    StringSlice lights;
    ArrayU32 joltage_req;
    ArrayWiring wirings;
};

Array_Prototype(Manual);
Array_Impl(Manual);

typedef struct IndicatorState IndicatorState;
struct IndicatorState {
    String lights;
    U32 num_presses;
};

typedef struct JoltageState JoltageState;
struct JoltageState {
    ArrayU32 joltages;
    U32 num_presses;
};

typedef struct QueueIS QueueIS;
struct QueueIS {
    IndicatorState *items;
    size_t cap;
    size_t size;
    size_t head;
    size_t tail;
};

typedef struct QueueJS QueueJS;
struct QueueJS {
    JoltageState *items;
    size_t cap;
    size_t size;
    size_t head;
    size_t tail;
};

// ----------------------------------------------------- //
ArrayU32 ArrayU32_from_joltage_slice(Arena *arena, StringSlice slice)
{
    Temp temp         = Temp_start(arena);
    ArrayU32 joltages = {0};
    slice.items += 1;
    slice.len -= 1;

    Temp_end(temp);
    return joltages;
}

// ----------------------------------------------------- //
void QueueJS_push(QueueJS *q, JoltageState state)
{
    if (q->size == q->cap) {
        fprintf(
            stderr,
            ANSI_TEXT_RED "Error: " ANSI_RESET
                          "QueueJS is full: size: %lu, cap: %lu\n",
            q->size,
            q->cap);
        return;
    }

    // fprintf(stderr, "pushing to %lu: ", q->tail);
    // String_print(state.lights);

    q->items[q->tail] = state;
    q->tail           = (q->tail + 1) % q->cap;
    q->size += 1;
    return;
}

// ----------------------------------------------------- //
JoltageState QueueJS_pop(QueueJS *q)
{
    JoltageState out = {0};
    if (q->size == 0) {
        fprintf(
            stderr,
            ANSI_TEXT_RED "Error: " ANSI_RESET
                          "QueueJS is empty: size: %lu, cap: %lu",
            q->size,
            q->cap);
        return out;
    }

    // fprintf(stderr, "popping from %lu: ", q->head);

    out = q->items[q->head];
    // String_print(out.lights);

    q->head = (q->head + 1) % q->cap;
    q->size -= 1;
    return out;
}

// ----------------------------------------------------- //
QueueJS QueueJS_new(Arena *arena, size_t capacity)
{
    QueueJS new = {0};
    new.cap     = capacity;
    new.items   = Arena_alloc(arena, sizeof(JoltageState) * capacity);
    return new;
}

// ----------------------------------------------------- //
void QueueIS_push(QueueIS *q, IndicatorState state)
{
    if (q->size == q->cap) {
        fprintf(
            stderr,
            ANSI_TEXT_RED "Error: " ANSI_RESET
                          "QueueIS is full: size: %lu, cap: %lu\n",
            q->size,
            q->cap);
        return;
    }

    // fprintf(stderr, "pushing to %lu: ", q->tail);
    // String_print(state.lights);

    q->items[q->tail] = state;
    q->tail           = (q->tail + 1) % q->cap;
    q->size += 1;
    return;
}

// ----------------------------------------------------- //
IndicatorState QueueIS_pop(QueueIS *q)
{
    IndicatorState out = {0};
    if (q->size == 0) {
        fprintf(
            stderr,
            ANSI_TEXT_RED "Error: " ANSI_RESET
                          "QueueIS is empty: size: %lu, cap: %lu",
            q->size,
            q->cap);
        return out;
    }

    // fprintf(stderr, "popping from %lu: ", q->head);

    out = q->items[q->head];
    // String_print(out.lights);

    q->head = (q->head + 1) % q->cap;
    q->size -= 1;
    return out;
}

// ----------------------------------------------------- //
QueueIS QueueIS_new(Arena *arena, size_t capacity)
{
    QueueIS new = {0};
    new.cap     = capacity;
    new.items   = Arena_alloc(arena, sizeof(IndicatorState) * capacity);
    return new;
}

// ----------------------------------------------------- //
IndicatorState IndicatorState_copy(Arena *arena, IndicatorState state)
{
    IndicatorState new = {};
    new.num_presses    = state.num_presses;
    new.lights         = String_with_capacity(arena, state.lights.len);
    new.lights.len     = state.lights.len;
    memcpy(new.lights.items, state.lights.items, state.lights.len);
    return new;
}

// ----------------------------------------------------- //
void Wiring_print(Wiring wiring)
{
    printf("Wiring <" ANSI_TEXT_YELLOW);
    for (size_t i = 0; i < wiring.len; i++) {
        U32 num = ArrayU32_get_value(&wiring, i);
        printf("%u, ", num);
    }
    printf(ANSI_RESET ">\n");
}

// ----------------------------------------------------- //
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

    if (out > 10) {
        fprintf(
            stderr,
            ANSI_TEXT_RED "Error: " ANSI_RESET
                          "Num larger then 10, this is not handled correctly!");
    }
    return out;
}

// ----------------------------------------------------- //
Wiring Wiring_from_sslice(Arena *arena, StringSlice slice)
{
    Wiring new   = ArrayU32_with_capacity(arena, slice.len);
    size_t index = 0;
    while (index < slice.len) {
        char current = slice.items[index];
        if (current < '0' || current > '9') {
            index += 1;
        } else {
            StringSlice to_parse = {slice.items + index, slice.len - index};
            U32 num              = U32_from_sslice(to_parse);
            ArrayU32_push(&new, num);
            index += 1;
        }
    }
    return new;
}

// ----------------------------------------------------- //
Manual Manual_from_sslice(Arena *arena, StringSlice line)
{
    ArrayStringSlice parts = ArrayStringSlice_with_capacity(arena, 20);
    StringSlice_split_to_slices(&parts, line, ' ');
    Manual out = {0};
    out.lights = ArrayStringSlice_remove_at(&parts, 0);
    out.lights.items += 1;
    out.lights.len -= 2;
    out.joltage_req =
        ArrayU32_from_joltage_slice(arena, ArrayStringSlice_pop(&parts));

    ArrayWiring wirings = ArrayWiring_with_capacity(arena, parts.len);
    for (size_t i = 0; i < parts.len; i++) {
        StringSlice part = ArrayStringSlice_get_value(&parts, i);
        Wiring wiring    = Wiring_from_sslice(arena, part);
        ArrayWiring_push(&wirings, wiring);
    }
    out.wirings = wirings;
    return out;
}

// ----------------------------------------------------- //
void Manual_print(Manual manual)
{
    printf("Manual: <\nlight:       ");
    StringSlice_print(manual.lights);
    printf("joltage_req: ");
    // StringSlice_print(manual.joltage_req);
    for (size_t i = 0; i < manual.wirings.len; i++) {
        if (i == 0) {
            printf("wiring:      ");
        } else {
            printf("             ");
        }

        Wiring wiring = ArrayWiring_get_value(&manual.wirings, i);
        Wiring_print(wiring);
        // StringSlice_print(line);
    }
    printf(">\n");
}

// ----------------------------------------------------- //
void IndicatorState_press_apply(IndicatorState *state, Wiring wiring)
{
    for (size_t i = 0; i < wiring.len; i++) {
        U32 index        = ArrayU32_get_value(&wiring, i);
        char light_state = String_get(state->lights, index);
        if (light_state == '#') {
            state->lights.items[index] = '.';
        } else if (light_state == '.') {
            state->lights.items[index] = '#';
        } else {
            fprintf(
                stderr,
                ANSI_TEXT_RED "Error: " ANSI_RESET
                              "invalid button state at index %u",
                index);
        }
    }
    return;
}

// ----------------------------------------------------- //
U32 Manual_find_fewest_presses(Arena *arena, Manual manual, U32 iter_max)
{
    U32 result = 0;
    Temp temp  = Temp_start(arena);

    QueueIS queue  = QueueIS_new(temp.arena, 100000000);
    String all_off = String_with_capacity(temp.arena, manual.lights.len);
    for (size_t i = 0; i < manual.lights.len; i++) {
        String_push(&all_off, '.');
    }

    // classic bfs
    B32 sucess                   = 0;
    IndicatorState state_initial = {all_off, 0};
    QueueIS_push(&queue, state_initial);
    while (queue.size) {
        IndicatorState state_curr = QueueIS_pop(&queue);
        StringSlice lights = {state_curr.lights.items, state_curr.lights.len};
        if (StringSlice_equal(lights, manual.lights)) {
            result = state_curr.num_presses;
            sucess = 1;
            break;
        }

        if (state_curr.num_presses == iter_max) {
            continue;
        }

        for (size_t i = 0; i < manual.wirings.len; i++) {
            Wiring wiring = ArrayWiring_get_value(&manual.wirings, i);
            IndicatorState state_next =
                IndicatorState_copy(temp.arena, state_curr);
            IndicatorState_press_apply(&state_next, wiring);
            state_next.num_presses += 1;
            QueueIS_push(&queue, state_next);
        }
    }

    Temp_end(temp);
    if (sucess) {
        return result;
    } else {
        return -1;
    }
}

// ----------------------------------------------------- //
int main()
{
    Arena arena = {0};
    Arena_init(&arena, Megabytes(10000));
    String input           = String_with_capacity(&arena, Kilobytes(20));
    ArrayStringSlice lines = ArrayStringSlice_with_capacity(&arena, 300);
    FILE *file             = fopen(FILENAME, "rb");
    FILE_read_to_string(file, &input);
    fclose(file);
    StringSlice input_as_slice = {input.items, input.len};
    StringSlice_split_to_slices(&lines, input_as_slice, '\n');
    ArrayStringSlice_pop(&lines);

    ArrayManual manuals = ArrayManual_with_capacity(&arena, 300);
    for (size_t i = 0; i < lines.len; i++) {
        StringSlice line = ArrayStringSlice_get_value(&lines, i);
        Manual manual    = Manual_from_sslice(&arena, line);
        ArrayManual_push(&manuals, manual);
    }

    U32 result_part_1 = 0;

    for (size_t i = 0; i < manuals.len; i++) {
        Manual manual = ArrayManual_get_value(&manuals, i);
        // Manual_print(manual);
        U32 result = 0;
        result     = Manual_find_fewest_presses(&arena, manual, 8);
        printf("result: %u\n", result);
        result_part_1 += result;
    }

    printf("result part 1: %u\n", result_part_1);

    return 0;
}

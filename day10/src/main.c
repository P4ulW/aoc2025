#include <complex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bits/types/siginfo_t.h"
#include "cbase/src/ansi_codes.h"
#include "cbase/src/arena.c"
#include "cbase/src/array.c"
#include "cbase/src/string.c"
#include "cbase/src/file.c"
#include "combinations.c"

#define EXAMPLE 1

#if EXAMPLE == 0
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

static U64 evaluations = 0;

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
    return out;
}

// ----------------------------------------------------- //
ArrayU32 ArrayU32_from_joltage_slice(Arena *arena, StringSlice slice)
{
    ArrayU32 joltages = ArrayU32_with_capacity(arena, 20);
    Temp temp         = Temp_start(arena);
    slice.items += 1; // ignore first bracket
    slice.len -= 2;   // ignore last bracket as well
    ArrayStringSlice nums = ArrayStringSlice_with_capacity(temp.arena, 20);
    StringSlice_split_to_slices(&nums, slice, ',');
    for (size_t i = 0; i < nums.len; i++) {
        StringSlice num_as_slice = nums.items[i];
        ArrayU32_push(&joltages, U32_from_sslice(num_as_slice));
    }

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
U32 U32_single_from_sslice(StringSlice slice)
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
            U32 num              = U32_single_from_sslice(to_parse);
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
    printf("joltage_req: " ANSI_TEXT_YELLOW);
    for (size_t i = 0; i < manual.joltage_req.len; i++) {
        U32 joltage = ArrayU32_get_value(&manual.joltage_req, i);
        printf("%u ", joltage);
    }
    printf(ANSI_RESET "\n");

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
void ArrayU32_press_apply(ArrayU32 *joltages, Wiring wiring)
{
    for (size_t i = 0; i < wiring.len; i++) {
        U32 index    = ArrayU32_get_value(&wiring, i);
        U32 *joltage = ArrayU32_get_ref(joltages, index);
        *joltage += 1;
    }
    return;
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

    String all_off = String_with_capacity(temp.arena, manual.lights.len);
    for (size_t i = 0; i < manual.lights.len; i++) {
        String_push(&all_off, '.');
    }

    for (int k = 0; k <= manual.wirings.len; k++) {
        if (result) {
            break;
        }

        Combinations comb = Combinations_init(arena, manual.wirings.len, k);

        while (comb.index) {
            memset(all_off.items, '.', all_off.len);
            IndicatorState state = {.lights = all_off, 0};
            for (int i = 0; i < comb.stride; i++) {
                int index     = comb.index[i];
                Wiring wiring = ArrayWiring_get_value(&manual.wirings, index);
                IndicatorState_press_apply(&state, wiring);
            }

            StringSlice lights = {state.lights.items, state.lights.len};
            if (StringSlice_equal(lights, manual.lights)) {
                result = k;
                break;
            }
            Combinations_next(&comb);
        }
    }
    printf("needed %u\n", result);
    Temp_end(temp);
    return result;
}

ArrayU32 ArrayU32_copy(Arena *arena, ArrayU32 arr)
{
    ArrayU32 out = ArrayU32_with_capacity(arena, arr.cap);
    out.len      = arr.len;
    memcpy(out.items, arr.items, sizeof(U32) * arr.len);
    return out;
}

void ArrayU32_sub(ArrayU32 *arr1, ArrayU32 arr2)
{
    if (arr1->len != arr2.len) {
        fprintf(stderr, "ERROR: ArrayU32 size mismatch in ArrayU32_sub\n");
        return;
    }

    for (size_t i = 0; i < arr1->len; i++) {
        arr1->items[i] -= arr2.items[i];
    }
    return;
}

// ----------------------------------------------------- //
void ArrayU32_print(ArrayU32 array)
{
    printf("[");
    for (U32 i = 0; i < array.len; ++i) {
        printf("%u", array.items[i]);
        if (i < array.len - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

// ----------------------------------------------------- //
U64 get_min_presses_to_get_joltages(
    Arena *arena, const Manual manual, const ArrayU32 joltages)
{
    U64 min_presses = UINT64_MAX;

    B32 all_zero = 1;
    for (size_t i = 0; i < joltages.len; i++) {
        U32 joltage = ArrayU32_get_value(&joltages, i);
        if (joltage != 0) {
            all_zero = 0;
            break;
        }
    }
    if (all_zero) {
        return 0;
    }

    Temp temp = Temp_start(arena);

    String pattern = String_with_capacity(temp.arena, manual.joltage_req.len);
    for (size_t i = 0; i < manual.joltage_req.len; i++) {
        U32 joltage    = ArrayU32_get_value(&joltages, i);
        char indicator = (joltage % 2) ? '#' : '.';
        String_push(&pattern, indicator);
    }
    StringSlice pattern_as_slice = {pattern.items, pattern.len};

    String all_off = String_with_capacity(temp.arena, manual.lights.len);
    for (size_t i = 0; i < manual.lights.len; i++) {
        String_push(&all_off, '.');
    }

    ArrayU32 joltages_new = ArrayU32_with_capacity(temp.arena, joltages.len);
    for (size_t i = 0; i < joltages.len; i++) {
        ArrayU32_push(&joltages_new, 0);
    }

    U32 additional_presses = 0;
    for (int k = 0; k <= manual.wirings.len; k++) {
        Combinations comb = Combinations_init(arena, manual.wirings.len, k);

        while (comb.index) {
            memset(all_off.items, '.', all_off.len);
            memset(joltages_new.items, 0, joltages_new.len * sizeof(U32));

            IndicatorState state = {.lights = all_off, 0};
            for (int i = 0; i < comb.stride; i++) {
                int index     = comb.index[i];
                Wiring wiring = ArrayWiring_get_value(&manual.wirings, index);
                IndicatorState_press_apply(&state, wiring);
                ArrayU32_press_apply(&joltages_new, wiring);
            }

            StringSlice lights = {state.lights.items, state.lights.len};

            if (!StringSlice_equal(lights, pattern_as_slice)) {
                Combinations_next(&comb);
                continue;
            }

            additional_presses = k;

            B32 can_subtract = 1;
            for (size_t i = 0; i < joltages.len; i++) {
                if (joltages.items[i] < joltages_new.items[i]) {
                    can_subtract = 0;
                    break;
                }
            }
            if (!can_subtract) {
                Combinations_next(&comb);
                continue;
            }

            ArrayU32 to_test = ArrayU32_copy(temp.arena, joltages);
            ArrayU32_sub(&to_test, joltages_new);

            for (size_t i = 0; i < to_test.len; i++) {
                U32 *num = ArrayU32_get_ref(&to_test, i);
                if ((*num % 2)) {
                    printf("Error: expected joltages to be even\n");
                }

                *num /= 2;
            }

            U64 half_num_presses =
                get_min_presses_to_get_joltages(temp.arena, manual, to_test);
            evaluations += 1;

            if (half_num_presses == UINT64_MAX) {
                Combinations_next(&comb);
                continue;
            }

            U64 num_presses = additional_presses + 2 * half_num_presses;

            if (num_presses < min_presses) {
                min_presses = num_presses;
            }
            Combinations_next(&comb);
        }
    }

    Temp_end(temp);
    return min_presses;
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
        StringSlice_print(line);
        Manual manual = Manual_from_sslice(&arena, line);
        ArrayManual_push(&manuals, manual);
    }

    U32 result_part_1 = 0;

    for (size_t i = 0; i < manuals.len; i++) {
        Manual manual = ArrayManual_get_value(&manuals, i);
        U32 result    = 0;
        Manual_print(manual);
        result = Manual_find_fewest_presses(&arena, manual, 8);
        // printf("result: %u\n", result);
        result_part_1 += result;
    }

    printf(ANSI_TEXT_GREEN "result part 1: %u\n" ANSI_RESET, result_part_1);

    U64 result_part_2 = 0;
    for (size_t i = 0; i < manuals.len; i++) {
        Manual manual = ArrayManual_get_value(&manuals, i);
        U64 result =
            get_min_presses_to_get_joltages(&arena, manual, manual.joltage_req);
        result_part_2 += result;
    }

    printf(ANSI_TEXT_GREEN "result part 2: %lu\n" ANSI_RESET, result_part_2);
    printf("Total evals for part 2: %lu", evaluations);

    return 0;
}

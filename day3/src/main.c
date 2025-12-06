#include "arena.c"
#include "array.c"
#include "string.c"
#include <stdio.h>
#include <string.h>
#include <time.h>
typedef struct timeval timeval;

Array_Prototype(StringSlice);
Array_Impl(StringSlice);

const char filename[] = "input.txt";

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

U32 get_bank_joltage(StringSlice bank)
{
    char *left    = bank.items;
    char *right   = bank.items;
    char *current = bank.items;
    U32 max_left  = 0;
    U32 max_right = 0;

    for (int i = 0; i < bank.len; i++) {
        U32 current = U32_from_char(*right);
        // printf("current: %d\n", current);
        if ((current > max_left) && (i != bank.len - 1)) {
            max_left  = current;
            max_right = 0;
        } else if (current > max_right) {
            max_right = current;
        }
        right += 1;
    }

    // printf("left: %u right: %d\n\n", max_left, max_right);

    return max_left * 10 + max_right;
}

U64 get_bank_joltage_proper(StringSlice bank)
{
    U32 batteries[12] = {0};
    U64 joltage       = 0;
    char *ptr         = bank.items;

    for (int i = 0; i < bank.len; i++) {
        U32 current = U32_from_char(*ptr);

        if ((current > batteries[0]) && (i < bank.len - 11)) {
            batteries[0] = current;
            memset(&batteries[1], 0, 11);
        } else if ((current > batteries[1]) && (i < bank.len - 10)) {
            batteries[1] = current;
            memset(&batteries[2], 0, 10);
        } else if ((current > batteries[2]) && (i < bank.len - 9)) {
            batteries[2] = current;
            memset(&batteries[3], 0, 9);
        } else if ((current > batteries[3]) && (i < bank.len - 8)) {
            batteries[3] = current;
            memset(&batteries[4], 0, 8);
        } else if ((current > batteries[4]) && (i < bank.len - 7)) {
            batteries[4] = current;
            memset(&batteries[5], 0, 7);
        } else if ((current > batteries[5]) && (i < bank.len - 6)) {
            batteries[5] = current;
            memset(&batteries[6], 0, 6);
        } else if ((current > batteries[6]) && (i < bank.len - 5)) {
            batteries[6] = current;
            memset(&batteries[7], 0, 5);
        } else if ((current > batteries[7]) && (i < bank.len - 4)) {
            batteries[7] = current;
            memset(&batteries[8], 0, 4);
        } else if ((current > batteries[8]) && (i < bank.len - 3)) {
            batteries[8] = current;
            memset(&batteries[9], 0, 3);
        } else if ((current > batteries[9]) && (i < bank.len - 2)) {
            batteries[9] = current;
            memset(&batteries[10], 0, 2);
        } else if ((current > batteries[10]) && (i < bank.len - 1)) {
            batteries[10] = current;
            memset(&batteries[11], 0, 1);
        } else if ((current > batteries[11]) && (i < bank.len)) {
            batteries[11] = current;
        }

        ptr += 1;
    }

    for (int i = 0; i < 12; i++) {
        joltage = joltage * 10 + batteries[i];
    }
    return joltage;
}

int main()
{
    clock_t start, end;
    start       = clock();
    Arena arena = {0};
    arena_init(&arena, Megabytes(1));
    String input           = String_with_capacity(&arena, Kilobytes(200));
    ArrayStringSlice banks = ArrayStringSlice_with_capacity(&arena, 1000);
    FILE *file             = fopen(filename, "r");
    read_file_to_string(&input, file);
    StringSlice input_as_slice = {.items = input.items, .len = input.len};
    StringSlice_split_to_slices(&banks, input_as_slice, '\n');

    U64 result_part_1 = 0;
    U64 result_part_2 = 0;

    for (int i = 0; i < banks.len; i++) {
        StringSlice bank = ArrayStringSlice_get_value(&banks, i);
        // StringSlice_print(bank);
        U64 joltage = get_bank_joltage(bank);
        result_part_1 += joltage;

        joltage = get_bank_joltage_proper(bank);
        result_part_2 += joltage;
    }

    printf("\x1b[7m\x1b[32mresult part 1: %lu\x1b[0m\n", result_part_1);
    printf("\x1b[7m\x1b[32mresult part 2: %lu\x1b[0m\n", result_part_2);
    end       = clock();
    double dt = end - start;
    printf("time: %f ms\n", dt / 1000.0f);
    arena_free(&arena);

    return 0;
}

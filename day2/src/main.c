#include "arena.c"
#include "array.c"
#include "string.c"
#include <stdio.h>
#include <time.h>
typedef struct timeval timeval;

Array_Prototype(StringSlice);
Array_Impl(StringSlice);

const char filename[] = "input.txt";

static void read_file_to_string(String *string, FILE *file) {
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

static void StringSlice_split_to_slices(ArrayStringSlice *slices,
                                        StringSlice to_split,
                                        const char split_char) {
  U32 len = 0;
  StringSlice slice = {0};
  int i = 0;
  U32 start_new = 1;

  for (int i = 0; i < to_split.len; i++) {
    char current = to_split.items[i];
    if (start_new) {
      slice.items = &to_split.items[i];
      len = 0;
      slice.len = 0;
      start_new = 0;
    }

    if (current == split_char) {
      slice.len = len;
      ArrayStringSlice_push(slices, slice);
      start_new = 1;
    }
    len += 1;
  }

  slice.len = len;
  ArrayStringSlice_push(slices, slice);

  return;
}

static void StringSlice_print(StringSlice str) {
  printf("str: len %d content: ", str.len);
  for (int i = 0; i < str.len; i++) {
    char current = str.items[i];
    printf("%c", current);
  }
  printf("\n");
  return;
}

static U64 StringSlice_to_int(StringSlice str) {
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

static U32 get_length_of_U64(U64 num) {
  U32 len = 0;
  U64 temp = num;
  while (temp != 0) {
    len += 1;
    temp = temp / 10;
  }
  return len;
}

static U32 is_num_dup(U64 num) {
  U32 len = get_length_of_U64(num);
  if (len % 2) {
    return 0;
  }

  U64 div = 1;
  for (int i = 0; i < len / 2; i++) {
    div *= 10;
  }
  U64 lower = num % div;
  U64 upper = num / div;

  // printf("num: %lu upp: %lu lower %lu\n", num, upper, lower);

  if (lower != upper) {
    return 0;
  }

  return 1;
}

static U64 nth_power_of_10(U32 n) {
  U64 out = 1;
  for (int i = 0; i < n; i++) {
    out *= 10;
  }
  return out;
}

static U64 U64_get_n_last_digits(U64 num, U32 n) {
  U64 out = 0;
  U64 div = nth_power_of_10(n);
  out = num % div;
  return out;
}

static U32 is_made_from_repeat(U64 num) {
  // printf("\x1b[7mcurrent num %lu\x1b[0m\n", num);
  U32 len = get_length_of_U64(num);
  U32 is_odd = len % 2;

  for (int sequence_lenght = 1; sequence_lenght <= len / 2; sequence_lenght++) {
    if ((sequence_lenght != 1) && (len % sequence_lenght)) {
      continue;
    }

    U64 upper_part = num / nth_power_of_10(sequence_lenght);
    U64 ref = U64_get_n_last_digits(num, sequence_lenght);

    U32 all_equal = 1;
    while (upper_part != 0) {
      U64 to_comp = U64_get_n_last_digits(upper_part, sequence_lenght);
      upper_part = upper_part / nth_power_of_10(sequence_lenght);
      if (to_comp != ref) {
        all_equal = 0;
        break;
      }
    }

    if (all_equal) {
      return 1;
    }
  }

  return 0;
}

int main() {
  clock_t start, end;
  start = clock();
  Arena arena = {0};
  arena_init(&arena, Megabytes(1));
  String input = String_with_capacity(&arena, Kilobytes(2));
  ArrayStringSlice slices = ArrayStringSlice_with_capacity(&arena, 100);
  FILE *file = fopen(filename, "r");
  read_file_to_string(&input, file);
  StringSlice input_as_slice = {.items = input.items, .len = input.len};
  StringSlice_split_to_slices(&slices, input_as_slice, ',');

  U64 result_part_1 = 0;
  U64 result_part_2 = 0;

  // for (int i = 0; i < slices.len; i++) {
  // StringSlice current = ArrayStringSlice_get_value(&slices, i);
  // StringSlice_print(current);
  // }

  for (int i = 0; i < slices.len; i++) {
    StringSlice current = ArrayStringSlice_get_value(&slices, i);
    ArrayStringSlice range = ArrayStringSlice_with_capacity(&arena, 2);
    StringSlice_split_to_slices(&range, current, '-');
    U64 lower = StringSlice_to_int(range.items[0]);
    U64 upper = StringSlice_to_int(range.items[1]);

    // printf("from %ld to %ld\n", lower, upper);
    for (U64 num = lower; num <= upper; num++) {
      U32 is_dup = is_num_dup(num);
      U32 is_repeat = is_made_from_repeat(num);
      // printf("num: %lu\n", num);
      if (is_dup) {
        // printf("num: %lu\n", num);
        result_part_1 += num;
      }

      if (is_repeat) {
        // printf("\x1b[7m\x1b[33mnum: %lu is repeat\x1b[0m\n", num);
        result_part_2 += num;
      }
    }

    // printf("\n");
  }

  printf("\x1b[7m\x1b[32mresult part 1: %lu\x1b[0m\n", result_part_1);
  printf("\x1b[7m\x1b[32mresult part 2: %lu\x1b[0m\n", result_part_2);
  // printf("string: %s size %d", input.items, input.len);
  end = clock();
  double dt = end - start;
  printf("time: %f ms\n", dt / 1000.0f);
  arena_free(&arena);

  return 0;
}

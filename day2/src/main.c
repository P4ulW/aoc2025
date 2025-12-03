#include <stdio.h>
#include <sys/time.h>
#include "arena.c"
#include "array.c"
#include "string.c"
typedef struct timeval timeval;

Array_Prototype(StringSlice);
Array_Impl(StringSlice);

const char filename[] = "input.txt";

void read_file_to_string(String* string, FILE* file) {
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

void StringSlice_split_to_slices(ArrayStringSlice* slices,
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

void StringSlice_print(StringSlice str) {
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

U32 is_num_dup(U64 num) {
  int len = 0;
  U64 temp = num;
  while (temp != 0) {
    len += 1;
    temp = temp / 10;
  }

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

int main() {
  timeval start, end;
  gettimeofday(&start, NULL);
  Arena arena = {0};
  arena_init(&arena, Megabytes(1));
  String input = String_with_capacity(&arena, Kilobytes(2));
  ArrayStringSlice slices = ArrayStringSlice_with_capacity(&arena, 100);
  FILE* file = fopen(filename, "r");
  read_file_to_string(&input, file);
  StringSlice input_as_slice = {.items = input.items, .len = input.len};
  StringSlice_split_to_slices(&slices, input_as_slice, ',');

  U64 result = 0;

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

    // printf("form %ld to %ld\n", lower, upper);
    for (U64 num = lower; num <= upper; num++) {
      U32 is_dup = is_num_dup(num);
      if (is_dup) {
        // printf("num: %lu\n", num);
        result += num;
      }
    }

    // printf("\n");
  }

  printf("\x1b[7m\x1b[32mresult part 1: %lu\x1b[0m\n", result);
  // printf("string: %s size %d", input.items, input.len);
  gettimeofday(&end, NULL);
  double dt = end.tv_usec - start.tv_usec;
  printf("time: %f µs\n", dt);
  arena_free(&arena);

  return 0;
}

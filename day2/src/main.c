#include <stdio.h>
#include "arena.c"
#include "array.c"
#include "string.c"

Array_Prototype(StringSlice);
Array_Impl(StringSlice);

const char filename[] = "test.txt";

void read_file_to_string(String* string, FILE* file) {
  int c;
  for (int i = 0; i < string->cap; i++) {
    c = fgetc(file);
    if (c == EOF) {
      break;
    }

    printf("%c", (char)c);
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

static int StringSlice_to_int(StringSlice str) {
  int result = 0;

  for (int i = 0; i < str.len; i++) {
    char current = str.items[i];
    if ((current < '0') && (current > '9')) {
      break;
    }
    result *= 10;
    result += current - (int)'0';
  }

  return result;
}

int main() {
  Arena arena = {0};
  arena_init(&arena, Megabytes(1));
  String input = String_with_capacity(&arena, Kilobytes(2));
  ArrayStringSlice slices = ArrayStringSlice_with_capacity(&arena, 100);
  FILE* file = fopen(filename, "r");
  read_file_to_string(&input, file);
  StringSlice input_as_slice = {.items = input.items, .len = input.len};
  StringSlice_split_to_slices(&slices, input_as_slice, ',');

  U32 result = 0;

  for (int i = 0; i < slices.len; i++) {
    StringSlice current = ArrayStringSlice_get_value(&slices, i);
    StringSlice_print(current);
  }

  for (int i = 0; i < slices.len; i++) {
    StringSlice current = ArrayStringSlice_get_value(&slices, i);
    ArrayStringSlice range = ArrayStringSlice_with_capacity(&arena, 2);
    StringSlice_split_to_slices(&range, current, '-');
    int lower = StringSlice_to_int(range.items[0]);
    int upper = StringSlice_to_int(range.items[1]);

    printf("form %d to %d\n", lower, upper);

    // StringSlice_print(range.items[0]);
    // StringSlice_print(range.items[1]);
  }

  // printf("string: %s size %d", input.items, input.len);
  arena_free(&arena);

  return 0;
}

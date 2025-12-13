#include <stdio.h>
#include <stdlib.h>
#include "arena.c"
#include "array.c"
#include "types.h"

typedef struct {
    char *items;
    U32 len;
    U32 cap;
} String;

typedef struct {
    char *items;
    U32 len;
} StringSlice;

Array_Prototype(String);
Array_Prototype(StringSlice);

Array_Impl(String);
Array_Impl(StringSlice);

// ------------------------------------------------------ //
char String_get(const String self, const U32 index)
{
    if (index < 0 || index >= self.len) {
        fprintf(
            stderr,
            "Could not get char in String at index %d due to out of bounds\n",
            index);
        fflush(stderr);
        return 0;
    }

    return self.items[index];
}

// ------------------------------------------------------ //
String String_with_capacity(Arena *arena, U32 capacity)
{
    String string = {0};
    char *mem     = Arena_alloc(arena, capacity);
    string.items  = mem;
    string.cap    = capacity;
    string.len    = 0;
    return string;
}

// ------------------------------------------------------ //
void String_push(String *self, const char value)
{
    if (self->len >= self->cap) {
        fprintf(
            stderr,
            "Could not push to string, there is no space in string with cap "
            "%d\n",
            self->cap);
        fflush(stderr);
        return;
    }

    self->items[self->len] = value;
    self->len += 1;
    return;
}

// ------------------------------------------------------ //
String String_from_cstring(char *buf)
{
    String str   = {.items = buf, .len = 0, .cap = 0};
    U32 index    = 0;
    char current = buf[index];
    while (current) {
        str.len += 1;
        str.cap += 1;
        index += 1;
        current = buf[index];
    }
    return str;
}

// ------------------------------------------------------ //
static void String_print(const String str)
{
    printf("String <\x1b[33m");
    for (int i = 0; i < str.len; i++) {
        char current = str.items[i];
        if (current == '\n') {
            printf("\\n");
            continue;
        }
        printf("%c", current);
    }
    printf("\x1b[0m> len:%u, cap:%u\n", str.len, str.cap);
    return;
}

// ------------------------------------------------------ //
B32 String_equal(const String self, const String needle)
{
    if (self.len != needle.len) {
        return 0;
    }

    for (int i = 0; i < self.len; i++) {
        if (String_get(self, i) != String_get(needle, i)) {
            return 0;
        }
    }

    return 1;
}

// ------------------------------------------------------ //
static void StringSlice_split_to_slices(
    ArrayStringSlice *slices, const StringSlice to_split, const char split_char)
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
    if (start_new) {
        slice.items = &to_split.items[i];
        len         = 0;
        slice.len   = 0;
        start_new   = 0;
    }
    ArrayStringSlice_push(slices, slice);

    return;
}

// ------------------------------------------------------ //
static void StringSlice_print(const StringSlice str)
{
    printf("StringSlice <\x1b[33m");
    for (int i = 0; i < str.len; i++) {
        char current = str.items[i];
        if (current == '\n') {
            printf("\\n");
            continue;
        }
        printf("%c", current);
    }
    printf("\x1b[0m> len:%u\n", str.len);
    return;
}

#ifdef TEST_STRING
int main()
{
    char test[]   = "this is a test!\nWe test here\nwhatever we\nwant to!\n";
    String string = String_from_cstring(test);
    String_print(string);
    StringSlice slice = {.items = string.items, .len = string.len};
    StringSlice_print(slice);
    ArrayStringSlice lines = {
        .len = 0, .cap = 10, .items = malloc(sizeof(ArrayStringSlice) * 10)};
    StringSlice_split_to_slices(&lines, slice, '\n');
    for (U32 i = 0; i < lines.len; i++) {
        StringSlice line = ArrayStringSlice_get_value(&lines, i);
        StringSlice_print(line);
    }

    return 0;
}

#endif /* ifdef TEST_STRING */

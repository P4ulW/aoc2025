#ifndef ARRAY
#define ARRAY
#include "arena.c"
#include <stdint.h>
#include <stdio.h>
typedef uint32_t U32;

#define Array_Prototype(type)                                                  \
  typedef struct Array##type {                                                 \
    U32 len;                                                                   \
    U32 cap;                                                                   \
    type *items;                                                               \
  } Array##type;                                                               \
  void Array##type##_push(Array##type *self, type value);                      \
  Array##type Array##type##_with_capacity(Arena *arena, const U32 capacity);

#define Array_Impl(type)                                                       \
  void Array##type##_push(Array##type *self, type value) {                     \
    if (self->len == self->cap) {                                              \
      fprintf(stderr, "No space left in Array##type\n");                       \
      exit(-1);                                                                \
    }                                                                          \
                                                                               \
    self->items[self->len] = value;                                            \
    self->len += 1;                                                            \
  }                                                                            \
                                                                               \
  Array##type Array##type##_with_capacity(Arena *arena, const U32 capacity) {  \
    type *memory = (type *)Arena_alloc(arena, sizeof(type) * capacity);        \
    Array##type Self = {.len = 0, .cap = capacity, .items = memory};           \
    return Self;                                                               \
  }                                                                            \
                                                                               \
  void Array##type##_put_value(const Array##type *self, const U32 index,       \
                               type value) {                                   \
    if (index >= self->len) {                                                  \
      fprintf(stderr,                                                          \
              "accesssing array outside of bounds for put_value of type "      \
              "%s at index %u!",                                               \
              #type, index);                                                   \
      exit(-1);                                                                \
    }                                                                          \
    self->items[index] = value;                                                \
    return;                                                                    \
  }                                                                            \
                                                                               \
  type Array##type##_get_value(const Array##type *self, const U32 index) {     \
    if (index >= self->len) {                                                  \
      fprintf(stderr,                                                          \
              "accesssing array outside of bounds for get_value of type "      \
              "%s at index %u!",                                               \
              #type, index);                                                   \
      exit(-1);                                                                \
    }                                                                          \
    type out = self->items[index];                                             \
    return out;                                                                \
  }                                                                            \
                                                                               \
  type *Array##type##_get_ref(const Array##type *self, const U32 index) {      \
    if (index >= self->len) {                                                  \
      fprintf(stderr,                                                          \
              "accesssing array outside of bounds for get_ref of type "        \
              "%s at index %u!",                                               \
              #type, index);                                                   \
      exit(-1);                                                                \
    }                                                                          \
    type *out = &self->items[index];                                           \
    return out;                                                                \
  }                                                                            \
                                                                               \
  type Array##type##_remove_at(Array##type *self, const U32 index) {           \
    type out = {0};                                                            \
    if (index >= self->len) {                                                  \
      fprintf(stderr,                                                          \
              "trying to remove remove value out of bound! Array type: %s, "   \
              "Index : %u",                                                    \
              #type, index);                                                   \
      return out;                                                              \
    }                                                                          \
                                                                               \
    out = self->items[index];                                                  \
    for (U32 i = index; i < self->len - 1; i++) {                              \
      self->items[i] = self->items[i + 1];                                     \
    }                                                                          \
    self->len -= 1;                                                            \
    return out;                                                                \
  }                                                                            \
                                                                               \
  type Array##type##_pop(Array##type *self) {                                  \
    type out = {0};                                                            \
    if (!self->len) {                                                          \
      fprintf(stderr, "trying to remove from empty array! Array type: %s",     \
              #type);                                                          \
      return out;                                                              \
    }                                                                          \
                                                                               \
    self->len -= 1;                                                            \
    out = self->items[self->len];                                              \
    return out;                                                                \
  }

#endif /* ifndef ARRAY */

#ifdef ARRAY_TEST

Array_Prototype(int);
Array_Impl(int);

int main() {
  Arena arena = {0};
  Arena_init(&arena, 4096);
  fprintf(stderr, "arena base:%p\n", arena.mem_base);

  Arrayint arr = Arrayint_with_capacity(&arena, 5);
  fprintf(stderr, "array: cap %d len %d\n", arr.cap, arr.len);
  Arrayint_push(&arr, 10);
  Arrayint_push(&arr, 11);
  fprintf(stderr, "array: cap %d len %d\n", arr.cap, arr.len);
  int test = Arrayint_pop(&arr);
  fprintf(stderr, "popped %d\n", test);
  fprintf(stderr, "array: cap %d len %d\n", arr.cap, arr.len);
  Arena_free(&arena);
  return 0;
}

#endif /* ifdef ARRAY_TEST */

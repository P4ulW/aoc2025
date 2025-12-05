#include "types.h"

typedef struct {
  char* items;
  U32 len;
  U32 cap;
} String;

typedef struct {
  char* items;
  U32 len;
} StringSlice;

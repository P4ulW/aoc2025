#include <stddef.h>
#include <stdio.h>
#include "cbase/src/ansi_codes.h"
#include "cbase/src/arena.c"
#include "cbase/src/array.c"
#include "cbase/src/string.c"

#define EXAMPLE 1

#if EXAMPLE == 0
#define FILENAME "test.txt"
#else
#define FILENAME "input.txt"
#endif

int main() {
	return 0;
}

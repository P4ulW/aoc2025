
#ifndef ARENA
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define Kilobytes(value) (value * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)

typedef uint8_t U8;
typedef uint32_t U32;
typedef int32_t I32;

typedef struct Arena {
    U32 capacity;
    U32 size;
    void *mem_base;
    void *mem_current;
} Arena;

typedef struct Temp Temp;
struct Temp {
    Arena *arena;
    U32 pos;
};

void arena_init(Arena *arena, U32 size_in_bytes)
{
    void *base_mem = malloc(size_in_bytes);
    if (base_mem == NULL) {
        printf("Could not alloc arena with size %d\n", size_in_bytes);
        exit(1);
    }

    arena->mem_base    = base_mem;
    arena->mem_current = base_mem;
    arena->size        = 0;
    arena->capacity    = size_in_bytes;
}

void arena_print(const Arena arena)
{
    fprintf(
        stderr,
        "Arena at <%p>, %.2f MB / %.2f MB used\n",
        arena.mem_base,
        (float)arena.size / Megabytes(1),
        (float)arena.capacity / Megabytes(1));
    fflush(stderr);
}

void arena_free(Arena *arena)
{
    free(arena->mem_base);
    arena->capacity    = 0;
    arena->size        = 0;
    arena->mem_base    = NULL;
    arena->mem_current = NULL;
}

void *arena_alloc(Arena *arena, U32 num_bytes)
{
    if (arena->capacity - arena->size - num_bytes < 0) {
        printf("No space in arena to alloc %d bytes\n", num_bytes);
        exit(1);
    }
    U32 total_num_bytes = arena->size;
    U32 padding         = total_num_bytes % 8;

    if ((padding)) {
        arena->mem_current = (U8 *)arena->mem_current + padding;
        arena->size += padding;
    }

    U8 *ptr            = arena->mem_current;
    arena->mem_current = (U8 *)arena->mem_current + num_bytes;
    arena->size += num_bytes;
    return ptr;
}

void *arena_alloc_json(void *user_data, size_t num_bytes)
{
    Arena *arena = (Arena *)user_data;
    void *ptr    = arena_alloc(arena, num_bytes);
    return ptr;
}

void arena_reset(Arena *arena)
{
    arena->size        = 0;
    arena->mem_current = arena->mem_base;
}

void arena_pop_to(Arena *arena, U32 position)
{
    arena->mem_current = (void *)((char *)arena->mem_base + position);
    arena->size        = position;
}

Temp temp_start(Arena *arena)
{
    Temp temp  = {};
    temp.arena = arena;
    temp.pos   = arena->size;
    return temp;
}

void temp_end(Temp temp)
{
    arena_pop_to(temp.arena, temp.pos);
}
#define ARENA
#endif /* ifndef ARENA */

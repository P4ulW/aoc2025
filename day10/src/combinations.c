#include "cbase/src/arena.c"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *base;
    int *index;
    size_t size;
    size_t stride;
    int j;
    int n;
    int k;
    int x;
} Combinations;

Combinations Combinations_init(Arena *arena, const int n, const int k)
{
    Combinations comb = {0};
    comb.k            = k;
    comb.n            = n;
    comb.size         = k + 2;
    comb.stride       = k;
    comb.base         = Arena_alloc(arena, comb.size * sizeof(*comb.index));
    comb.index        = comb.base;

    for (int i = 0; i < k; ++i) comb.index[i] = i;
    comb.index[k]     = n;
    comb.index[k + 1] = 0;
    comb.j            = k - 1;
    return comb;
};

void Combinations_next(Combinations *comb)
{

    if (comb->n <= 0 || comb->k <= 0 || comb->n <= comb->k) {
        comb->index = NULL;
        return;
    }
    while (comb->j >= 0) {
        comb->index[comb->j] = comb->j + 1;
        comb->j--;
        return;
    }

    if (comb->index[0] + 1 < comb->index[1]) {
        comb->index[0]++;
        return;
    }

    comb->j = 0;
    do {
        comb->j++;
        comb->index[comb->j - 1] = comb->j - 1;
    } while ((comb->x = comb->index[comb->j] + 1) == comb->index[comb->j + 1]);

    if (comb->j >= comb->k) {
        comb->index = NULL;
        return;
    }

    comb->index[comb->j--] = comb->x;
    return;
}

#ifdef TEST_COMBINATIONS

int main(int argc, char *argv[])
{
    Arena arena = {0};
    Arena_init(&arena, Kilobytes(20));
    Combinations comb = Combinations_init(&arena, 7, 5);

    while (comb.index) {
        for (size_t i = 0; i < comb.stride; i++) {
            int j = comb.index[i];
            printf("%d ", j);
        }

        printf("\n");
        Combinations_next(&comb);
    }

    return 0;
}

#endif /* ifdef TEST_COMBINATIONS */

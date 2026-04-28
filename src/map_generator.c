#include "map_generator.h"

#include <ace/managers/rand.h>

static tRandManager mapRand;

void mapGeneratorSetSeed(ULONG seed) {
    UWORD seed1 = (UWORD)(seed >> 16);
    UWORD seed2 = (UWORD)seed;

    if (!seed1) {
        seed1 = 0x1234;
    }
    if (!seed2) {
        seed2 = 0x5678;
    }

    randInit(&mapRand, seed1, seed2);
}

ULONG mapGeneratorRand(void) {
    return randUl(&mapRand);
}

int mapGeneratorRandRange(int min, int max) {
    if (min >= max) {
        return min;
    }
    return min + randUwMax(&mapRand, (UWORD)(max - min - 1));
}

#include "map_generator.h"

static ULONG mapSeed = 0x12345678;

void mapGeneratorSetSeed(ULONG seed) {
    mapSeed = seed ? seed : 0x12345678;
}

ULONG mapGeneratorRand(void) {
    mapSeed = mapSeed * 1664525u + 1013904223u;
    return mapSeed;
}

int mapGeneratorRandRange(int min, int max) {
    if (min >= max) {
        return min;
    }
    return min + ((mapGeneratorRand() >> 16) % (max - min));
}

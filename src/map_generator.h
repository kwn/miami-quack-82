#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

#include <ace/types.h>

void mapGeneratorSetSeed(ULONG seed);
ULONG mapGeneratorRand(void);
int mapGeneratorRandRange(int min, int max);
void mapGeneratorGenerateOutdoor(void);
void mapGeneratorGenerateLake(void);

#endif /* MAP_GENERATOR_H */

#include "map_generator.h"

#include "map.h"

void mapGeneratorGenerateLake(void) {
    for (UWORD i = 0; i < MAP_W * MAP_H; ++i) {
        mapData[i] = 0;
    }
}

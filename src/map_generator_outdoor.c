#include "map_generator.h"

#include "map.h"

#define DIRT_FLAG 0x80
#define SMOOTH_FLAG 0x40

#define TILE_AT(x, y) mapData[(y) * width + (x)]
#define SET_TILE(x, y, val) mapData[(y) * width + (x)] = (val)

static UBYTE isInBounds(int x, int y, int width, int height) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

static UBYTE getTileSafe(int x, int y, int width, int height) {
    return isInBounds(x, y, width, height) ? TILE_AT(x, y) : 0;
}

static UBYTE isDirtAt(int x, int y, int width, int height) {
    return (getTileSafe(x, y, width, height) & DIRT_FLAG) != 0;
}

static void drawThickLine(int width, int height, int x0, int y0, int x1, int y1, int radius) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 > y0 ? y0 - y1 : y1 - y0;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1) {
        for (int cy = -radius; cy <= radius; ++cy) {
            for (int cx = -radius; cx <= radius; ++cx) {
                if (cx * cx + cy * cy <= radius * radius) {
                    int px = x0 + cx;
                    int py = y0 + cy;
                    if (isInBounds(px, py, width, height)) {
                        mapData[py * width + px] |= DIRT_FLAG;
                    }
                }
            }
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static UBYTE getAutotile(int x, int y, int width, int height) {
    if (!(TILE_AT(x, y) & DIRT_FLAG)) {
        int r = mapGeneratorRandRange(0, 1000);
        if (r < 500) {
            return 28;
        }
        if (r < 750) {
            return 29;
        }
        if (r < 965) {
            return 30;
        }
        if (r < 970) {
            return 21;
        }
        if (r < 975) {
            return 22;
        }
        if (r < 980) {
            return 23;
        }
        if (r < 985) {
            return 24;
        }
        if (r < 990) {
            return 25;
        }
        if (r < 995) {
            return 26;
        }
        return 27;
    }

    int north = isDirtAt(x, y - 1, width, height) ? 1 : 0;
    int south = isDirtAt(x, y + 1, width, height) ? 1 : 0;
    int west = isDirtAt(x - 1, y, width, height) ? 1 : 0;
    int east = isDirtAt(x + 1, y, width, height) ? 1 : 0;
    int mask = north | (west << 1) | (east << 2) | (south << 3);

    switch (mask) {
        case 0: return 28;
        case 1: return 15;
        case 2: return 9;
        case 3: return 16;
        case 4: return 7;
        case 5: return 14;
        case 6: return 1;
        case 7: return 15;
        case 8: return 1;
        case 9: return 7;
        case 10: return 2;
        case 11: return 9;
        case 12: return 0;
        case 13: return 7;
        case 14: return 1;
        case 15: {
            int nw = isDirtAt(x - 1, y - 1, width, height) ? 1 : 0;
            int ne = isDirtAt(x + 1, y - 1, width, height) ? 1 : 0;
            int sw = isDirtAt(x - 1, y + 1, width, height) ? 1 : 0;
            int se = isDirtAt(x + 1, y + 1, width, height) ? 1 : 0;
            int r;

            if (!nw) {
                return 4;
            }
            if (!ne) {
                return 3;
            }
            if (!sw) {
                return 11;
            }
            if (!se) {
                return 10;
            }

            r = mapGeneratorRandRange(0, 100);
            if (r < 60) {
                return 8;
            }
            if (r < 80) {
                return 17;
            }
            return 18;
        }
    }

    return 8;
}

static UBYTE isGrassTile(UBYTE tile) {
    return tile >= 21 && tile <= 30;
}

void mapGeneratorGenerateOutdoor(void) {
    int width = MAP_W;
    int height = MAP_H;
    int minVisX = 0;
    int maxVisX = width - 1;
    int minVisY = 0;
    int maxVisY = height - 1;
    int nodeCount;
    int nodesX[30];
    int nodesY[30];

    for (int i = 0; i < width * height; ++i) {
        mapData[i] = 0;
    }

    nodeCount = mapGeneratorRandRange(15, 25);
    for (int i = 0; i < nodeCount; ++i) {
        nodesX[i] = mapGeneratorRandRange(minVisX + 10, maxVisX - 10);
        nodesY[i] = mapGeneratorRandRange(minVisY + 10, maxVisY - 10);
    }

    for (int i = 0; i < nodeCount - 1; ++i) {
        drawThickLine(width, height, nodesX[i], nodesY[i], nodesX[i + 1], nodesY[i + 1], mapGeneratorRandRange(2, 4));
    }

    for (int i = 0; i < nodeCount / 2; ++i) {
        int nodeA = mapGeneratorRandRange(0, nodeCount - 1);
        int nodeB = mapGeneratorRandRange(0, nodeCount - 1);
        drawThickLine(width, height, nodesX[nodeA], nodesY[nodeA], nodesX[nodeB], nodesY[nodeB], mapGeneratorRandRange(1, 3));
    }

    for (int y = minVisY + 1; y < maxVisY - 1; ++y) {
        for (int x = minVisX + 1; x < maxVisX - 1; ++x) {
            int dirtCount = 0;
            for (int cy = -1; cy <= 1; ++cy) {
                for (int cx = -1; cx <= 1; ++cx) {
                    if (isDirtAt(x + cx, y + cy, width, height)) {
                        ++dirtCount;
                    }
                }
            }
            if (dirtCount >= 5) {
                TILE_AT(x, y) |= SMOOTH_FLAG;
            }
        }
    }

    for (int i = 0; i < width * height; ++i) {
        mapData[i] = (mapData[i] & SMOOTH_FLAG) ? DIRT_FLAG : 0;
    }

    for (int y = minVisY; y <= maxVisY; ++y) {
        for (int x = minVisX; x <= maxVisX; ++x) {
            UBYTE tile = getAutotile(x, y, width, height);
            SET_TILE(x, y, (TILE_AT(x, y) & DIRT_FLAG) | tile);
        }
    }

    for (int i = 0; i < width * height; ++i) {
        mapData[i] &= ~DIRT_FLAG;
    }

    for (int y = minVisY + 1; y < maxVisY - 3; ++y) {
        for (int x = minVisX + 1; x < maxVisX - 2; ++x) {
            if (mapGeneratorRandRange(0, 1000) < 4) {
                UBYTE canPlace = 1;

                for (int cy = 0; cy < 3 && canPlace; ++cy) {
                    for (int cx = 0; cx < 2; ++cx) {
                        if (!isGrassTile(TILE_AT(x + cx, y + cy))) {
                            canPlace = 0;
                            break;
                        }
                    }
                }

                for (int cy = -1; cy <= 3 && canPlace; ++cy) {
                    for (int cx = -1; cx <= 2; ++cx) {
                        if (cx >= 0 && cx < 2 && cy >= 0 && cy < 3) {
                            continue;
                        }
                        if (!isGrassTile(TILE_AT(x + cx, y + cy))) {
                            canPlace = 0;
                            break;
                        }
                    }
                }

                if (canPlace) {
                    SET_TILE(x, y, 5);
                    SET_TILE(x + 1, y, 6);
                    SET_TILE(x, y + 1, 12);
                    SET_TILE(x + 1, y + 1, 13);
                    SET_TILE(x, y + 2, 19);
                    SET_TILE(x + 1, y + 2, 20);
                }
            }
        }
    }

}

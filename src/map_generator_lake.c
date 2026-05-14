#include "map_generator.h"

#include "map.h"

#define LAKE_TILESET_W 14

#define TILE_WATER 56

#define PIER_FLAG 0x80
#define PLAZA_FLAG 0x40

#define MAP_MARGIN 10
#define MIN_NODE_DIST 8
#define MAX_PIER_LEN 40

#define MAX_NODES 20
#define MAX_EDGES 40

#define TILE_AT(x, y) mapData[(y) * MAP_W + (x)]
#define SET_TILE(x, y, val) mapData[(y) * MAP_W + (x)] = (val)

typedef enum {
    NODE_PLAZA,
    NODE_HOUSE
} tNodeType;

static int nodeX[MAX_NODES];
static int nodeY[MAX_NODES];
static tNodeType nodeType[MAX_NODES];
static int nodeW[MAX_NODES];
static int nodeH[MAX_NODES];
static int nodeCount;

static int edgeA[MAX_EDGES];
static int edgeB[MAX_EDGES];
static int edgePierW[MAX_EDGES];
static int edgeCount;

static UBYTE isPierAt(int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 0;
    return (mapData[y * MAP_W + x] & PIER_FLAG) != 0;
}

static int getPierBaseRow(int x, int y) {
    int n, s, w, e;
    if (mapData[y * MAP_W + x] & PLAZA_FLAG) return 3;

    n = 0; s = 0; w = 0; e = 0;
    while (isPierAt(x, y - n - 1)) ++n;
    while (isPierAt(x, y + s + 1)) ++s;
    while (isPierAt(x - w - 1, y)) ++w;
    while (isPierAt(x + e + 1, y)) ++e;

    if (w + e + 1 > n + s + 3) return 0;
    return 3;
}

static UBYTE getPierTile(int x, int y) {
    int hasN = isPierAt(x, y - 1);
    int hasS = isPierAt(x, y + 1);
    int hasW = isPierAt(x - 1, y);
    int hasE = isPierAt(x + 1, y);
    int baseRow = getPierBaseRow(x, y);
    int row, col;

    if (!hasN) row = 0;
    else row = 1;

    if (!hasW && hasE) col = 0;
    else if (hasW && !hasE) col = 2;
    else col = 1;

    return (UBYTE)((baseRow + row) * LAKE_TILESET_W + 6 + col);
}

static int isWaterAt(int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 0;
    return (mapData[y * MAP_W + x] & PIER_FLAG) == 0;
}

static void placePiles(void) {
    for (int y = 0; y < MAP_H; ++y) {
        for (int x = 0; x < MAP_W; ++x) {
            int baseRow, col;
            if (!(mapData[y * MAP_W + x] & PIER_FLAG)) continue;
            if (!isWaterAt(x, y + 1)) continue;

            baseRow = getPierBaseRow(x, y);
            if (!isWaterAt(x - 1, y + 1) && isPierAt(x - 1, y)) col = 0;
            else if (!isWaterAt(x + 1, y + 1) && isPierAt(x + 1, y)) col = 2;
            else col = 1;

            mapData[(y + 1) * MAP_W + x] = (UBYTE)((baseRow + 2) * LAKE_TILESET_W + 6 + col);
        }
    }
}

static int distBetween(int a, int b) {
    int dx = nodeX[a] - nodeX[b];
    int dy = nodeY[a] - nodeY[b];
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx + dy;
}

static void generateNodes(void) {
    int targetCount = mapGeneratorRandRange(12, 18);
    int candidateW, candidateH;
    nodeCount = 0;

    for (int attempt = 0; attempt < targetCount * 10 && nodeCount < targetCount; ++attempt) {
        int x = mapGeneratorRandRange(MAP_MARGIN, MAP_W - MAP_MARGIN);
        int y = mapGeneratorRandRange(MAP_MARGIN, MAP_H - MAP_MARGIN);

        if (nodeCount < (targetCount * 2) / 3) {
            candidateW = mapGeneratorRandRange(10, 16);
            candidateH = mapGeneratorRandRange(10, 16);
        } else {
            candidateW = 3;
            candidateH = 4;
        }

        int tooClose = 0;
        for (int i = 0; i < nodeCount; ++i) {
            int gapX = nodeX[i] - x;
            int gapY = nodeY[i] - y;
            if (gapX < 0) gapX = -gapX;
            if (gapY < 0) gapY = -gapY;
            gapX -= (nodeW[i] + candidateW) / 2;
            gapY -= (nodeH[i] + candidateH) / 2;
            if (gapX < 0) gapX = 0;
            if (gapY < 0) gapY = 0;
            if (gapX + gapY < MIN_NODE_DIST) {
                tooClose = 1;
                break;
            }
        }
        if (tooClose) continue;

        nodeX[nodeCount] = x;
        nodeY[nodeCount] = y;
        nodeW[nodeCount] = candidateW;
        nodeH[nodeCount] = candidateH;

        if (nodeCount < (targetCount * 2) / 3) {
            nodeType[nodeCount] = NODE_PLAZA;
        } else {
            nodeType[nodeCount] = NODE_HOUSE;
        }

        ++nodeCount;
    }
}

static int edgeExists(int a, int b) {
    for (int i = 0; i < edgeCount; ++i) {
        if ((edgeA[i] == a && edgeB[i] == b) || (edgeA[i] == b && edgeB[i] == a)) {
            return 1;
        }
    }
    return 0;
}

static void addEdge(int a, int b) {
    int dist, w;
    if (a == b || edgeExists(a, b) || edgeCount >= MAX_EDGES) return;
    edgeA[edgeCount] = a;
    edgeB[edgeCount] = b;

    dist = distBetween(a, b);
    if (dist < 25) {
        w = mapGeneratorRandRange(3, 5);
    } else if (dist < 45) {
        w = mapGeneratorRandRange(4, 5);
    } else {
        w = 5;
    }
    edgePierW[edgeCount] = w;
    ++edgeCount;
}

static void generateEdges(void) {
    int inTree[MAX_NODES];
    edgeCount = 0;

    // Prim's MST
    for (int i = 0; i < nodeCount; ++i) inTree[i] = 0;
    inTree[0] = 1;

    for (int added = 1; added < nodeCount; ++added) {
        int bestA = -1, bestB = -1, bestDist = 9999;
        for (int i = 0; i < nodeCount; ++i) {
            if (!inTree[i]) continue;
            for (int j = 0; j < nodeCount; ++j) {
                if (inTree[j]) continue;
                int d = distBetween(i, j);
                if (d < bestDist) {
                    bestDist = d;
                    bestA = i;
                    bestB = j;
                }
            }
        }
        if (bestA >= 0) {
            inTree[bestB] = 1;
            addEdge(bestA, bestB);
        }
    }

    // Extra edges — prefer shorter connections
    int extraCount = nodeCount / 2;
    for (int attempt = 0; attempt < extraCount * 8 && extraCount > 0; ++attempt) {
        int a = mapGeneratorRandRange(0, nodeCount);
        int b = mapGeneratorRandRange(0, nodeCount);
        if (!edgeExists(a, b) && a != b && distBetween(a, b) < 50) {
            addEdge(a, b);
            --extraCount;
        }
    }
}

static void fillRect(int x0, int y0, int w, int h, UBYTE val) {
    for (int y = y0; y < y0 + h; ++y) {
        for (int x = x0; x < x0 + w; ++x) {
            if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H) {
                mapData[y * MAP_W + x] |= val;
            }
        }
    }
}

static void drawLShape(int x0, int y0, int x1, int y1, int width) {
    int halfW = width / 2;
    int dx = x1 - x0;
    int dy = y1 - y0;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    int minX, maxX, minY, maxY, midY, midX;

    if (ady < 3) {
        midY = (y0 + y1) / 2;
        minX = x0 < x1 ? x0 : x1;
        fillRect(minX - halfW, midY - halfW, adx + width, width, PIER_FLAG);
        return;
    }
    if (adx < 3) {
        midX = (x0 + x1) / 2;
        minY = y0 < y1 ? y0 : y1;
        fillRect(midX - halfW, minY - halfW, width, ady + width, PIER_FLAG);
        return;
    }

    if (mapGeneratorRandRange(0, 2) == 0) {
        minX = x0 < x1 ? x0 : x1;
        maxX = x0 > x1 ? x0 : x1;
        fillRect(minX - halfW, y0 - halfW, maxX - minX + width, width, PIER_FLAG);

        minY = y0 < y1 ? y0 : y1;
        maxY = y0 > y1 ? y0 : y1;
        fillRect(x1 - halfW, minY - halfW, width, maxY - minY + width, PIER_FLAG);
    } else {
        minY = y0 < y1 ? y0 : y1;
        maxY = y0 > y1 ? y0 : y1;
        fillRect(x0 - halfW, minY - halfW, width, maxY - minY + width, PIER_FLAG);

        minX = x0 < x1 ? x0 : x1;
        maxX = x0 > x1 ? x0 : x1;
        fillRect(minX - halfW, y1 - halfW, maxX - minX + width, width, PIER_FLAG);
    }
}

static void drawPier(int x0, int y0, int x1, int y1, int width) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    if (dx + dy > MAX_PIER_LEN) {
        int mx = (x0 + x1) / 2 + mapGeneratorRandRange(-3, 3);
        int my = (y0 + y1) / 2 + mapGeneratorRandRange(-3, 3);
        drawLShape(x0, y0, mx, my, width);
        drawLShape(mx, my, x1, y1, width);
    } else {
        drawLShape(x0, y0, x1, y1, width);
    }
}

void mapGeneratorGenerateLake(void) {
    // Phase 1: fill with water
    for (UWORD i = 0; i < MAP_W * MAP_H; ++i) {
        mapData[i] = TILE_WATER;
    }

    // Phase 2: generate nodes
    generateNodes();

    // Phase 3: generate edges (MST + extras)
    generateEdges();

    // Phase 4: draw plazas as pier + plaza flags
    for (int i = 0; i < nodeCount; ++i) {
        if (nodeType[i] == NODE_PLAZA) {
            fillRect(
                nodeX[i] - nodeW[i] / 2, nodeY[i] - nodeH[i] / 2,
                nodeW[i], nodeH[i], PIER_FLAG | PLAZA_FLAG
            );
        }
    }

    // Phase 5: draw pier connections
    for (int i = 0; i < edgeCount; ++i) {
        drawPier(
            nodeX[edgeA[i]], nodeY[edgeA[i]],
            nodeX[edgeB[i]], nodeY[edgeB[i]],
            edgePierW[i]
        );
    }

    // Phase 6: autotile piers (keep PIER_FLAG so neighbors can still detect)
    for (int y = 0; y < MAP_H; ++y) {
        for (int x = 0; x < MAP_W; ++x) {
            if (mapData[y * MAP_W + x] & PIER_FLAG) {
                mapData[y * MAP_W + x] = PIER_FLAG | getPierTile(x, y);
            }
        }
    }

    // Phase 7: place piles on water below pier edges
    placePiles();

    // Phase 8: clear flags
    for (UWORD i = 0; i < MAP_W * MAP_H; ++i) {
        mapData[i] &= ~PIER_FLAG;
    }
}

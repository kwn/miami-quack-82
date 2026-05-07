#ifndef GAME_MATH_UTILS_H
#define GAME_MATH_UTILS_H

/**
 * Shared vector helpers (same semantics as original game/).
 * Keep inline-only — no separate .c file.
 */

#define DIR3_RIGHT 0
#define DIR3_LEFT  1
#define DIR3_UP    2

static inline int mathApproxVectorLength(int dx, int dy) {
    int absDx = dx > 0 ? dx : -dx;
    int absDy = dy > 0 ? dy : -dy;
    int max = absDx > absDy ? absDx : absDy;
    int min = absDx < absDy ? absDx : absDy;
    return max + ((min + (min << 1)) >> 3);
}

/** 3-way facing: right / left / up cone (matches original GetDirection3FromVector). */
static inline int mathDirection3FromVector(int dx, int dy) {
    if (dx == 0 && dy == 0) {
        return DIR3_RIGHT;
    }
    if (dy < 0) {
        int absDx = dx < 0 ? -dx : dx;
        int absDy = -dy;
        if (absDx < ((absDy * 7) >> 2)) {
            return DIR3_UP;
        }
    }
    return dx >= 0 ? DIR3_RIGHT : DIR3_LEFT;
}

#endif /* GAME_MATH_UTILS_H */

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

/** 18 directions clockwise from up, matching the original weapon frame layout. */
static inline int mathDirection18FromVector(int dx, int dy) {
    int absDx = dx >= 0 ? dx : -dx;
    int absDy = dy >= 0 ? dy : -dy;

    if (absDx == 0 && absDy == 0) {
        return 0;
    }

    if (absDy >= absDx) {
        if (dy < 0) {
            if (absDx * 5 <= absDy) {
                return dx >= 0 ? 0 : 16;
            }
            if (dx > 0) {
                return absDx * 3 <= absDy * 2 ? 1 : 2;
            }
            return absDx * 3 <= absDy * 2 ? 15 : 14;
        }

        if (absDx * 5 <= absDy) {
            return dx >= 0 ? 8 : 17;
        }
        if (dx > 0) {
            return absDx * 3 <= absDy * 2 ? 7 : 6;
        }
        return absDx * 3 <= absDy * 2 ? 9 : 10;
    }

    if (dx > 0) {
        if (absDy * 5 <= absDx) {
            return 4;
        }
        return dy < 0
            ? (absDy * 3 <= absDx * 2 ? 3 : 2)
            : (absDy * 3 <= absDx * 2 ? 5 : 6);
    }

    if (absDy * 5 <= absDx) {
        return 12;
    }
    return dy < 0
        ? (absDy * 3 <= absDx * 2 ? 13 : 14)
        : (absDy * 3 <= absDx * 2 ? 11 : 10);
}

#endif /* GAME_MATH_UTILS_H */

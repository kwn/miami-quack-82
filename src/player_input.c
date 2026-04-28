#include "player_input.h"

#include <ace/managers/key.h>

void playerInputGetMovement(int *dx, int *dy) {
    static int prevLeft;
    static int prevRight;
    static int prevUp;
    static int prevDown;
    static int prefX;
    static int prefY;

    int left = keyCheck(KEY_A) || keyCheck(KEY_LEFT);
    int right = keyCheck(KEY_D) || keyCheck(KEY_RIGHT);
    int up = keyCheck(KEY_W) || keyCheck(KEY_UP);
    int down = keyCheck(KEY_S) || keyCheck(KEY_DOWN);

    if (left && !prevLeft) {
        prefX = -1;
    }
    if (right && !prevRight) {
        prefX = 1;
    }
    if (!left && right) {
        prefX = 1;
    }
    if (!right && left) {
        prefX = -1;
    }

    if (up && !prevUp) {
        prefY = -1;
    }
    if (down && !prevDown) {
        prefY = 1;
    }
    if (!up && down) {
        prefY = 1;
    }
    if (!down && up) {
        prefY = -1;
    }

    *dx = 0;
    *dy = 0;

    if (prefX == -1 && left) {
        *dx = -1;
    }
    if (prefX == 1 && right) {
        *dx = 1;
    }
    if (prefY == -1 && up) {
        *dy = -1;
    }
    if (prefY == 1 && down) {
        *dy = 1;
    }

    prevLeft = left;
    prevRight = right;
    prevUp = up;
    prevDown = down;
}

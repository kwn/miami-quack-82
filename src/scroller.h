#ifndef SCROLLER_H
#define SCROLLER_H

#include <ace/managers/state.h>

/**
 * XY tile-buffer scroller demo state.
 *
 * Map: MAP_TILES_X × MAP_TILES_Y tiles, each TILE_SIZE × TILE_SIZE pixels.
 * Controls: arrow keys / WASD scroll the camera, ESC exits.
 *
 * To remove this state from the project, delete scroller.c and scroller.h
 * and replace the stateChange() call in main.c.
 */
extern tState scrollerState;

#endif /* SCROLLER_H */

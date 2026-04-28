#ifndef SCROLLER_H
#define SCROLLER_H

#include <ace/types.h>
#include <ace/utils/extview.h>

/**
 * XY tile-buffer scroller used by gameState.
 */
tVPort *scrollerCreate(tView *view);
void scrollerMoveCamera(WORD dx, WORD dy);
void scrollerSetCamera(UWORD x, UWORD y);
UWORD scrollerGetCameraX(void);
UWORD scrollerGetCameraY(void);
void scrollerProcess(void);
void scrollerDestroy(void);

#endif /* SCROLLER_H */

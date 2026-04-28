#ifndef SCROLLER_H
#define SCROLLER_H

#include <ace/types.h>
#include <ace/utils/bitmap.h>
#include <ace/utils/extview.h>

/**
 * XY tile-buffer scroller used by gameState.
 */
tVPort *scrollerCreate(tView *view);
void scrollerMoveCamera(WORD dx, WORD dy);
void scrollerSetCamera(UWORD x, UWORD y);
void scrollerRedrawAll(void);
UWORD scrollerGetCameraX(void);
UWORD scrollerGetCameraY(void);
tBitMap *scrollerGetFrontBuffer(void);
tBitMap *scrollerGetBackBuffer(void);
tBitMap *scrollerGetPristineBuffer(void);
UWORD scrollerGetBufferAvailHeight(void);
UWORD scrollerGetWorldWidth(void);
UWORD scrollerGetWorldHeight(void);
void scrollerDestroy(void);

#endif /* SCROLLER_H */

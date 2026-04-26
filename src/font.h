#ifndef GAME_FONT_H
#define GAME_FONT_H

#include <ace/types.h>
#include <ace/utils/bitmap.h>
#include <ace/utils/font.h>

void gameFontDrawStr(
    const tFont *font,
    tBitMap *dest,
    tTextBitMap *textBitmap,
    UWORD x,
    UWORD y,
    const char *text,
    UBYTE color,
    UBYTE shadowColor
);

#endif /* GAME_FONT_H */

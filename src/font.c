#include "font.h"

static void drawTextLayer(tBitMap *dest, tTextBitMap *textBitmap, WORD x, WORD y, UBYTE color) {
    if (x < 0 || y < 0) {
        return;
    }
    fontDrawTextBitMap(dest, textBitmap, (UWORD)x, (UWORD)y, color, FONT_COOKIE);
}

void gameFontDrawStr(
    const tFont *font,
    tBitMap *dest,
    tTextBitMap *textBitmap,
    UWORD x,
    UWORD y,
    const char *text,
    UBYTE color,
    UBYTE shadowColor
) {
    fontFillTextBitMap(font, textBitmap, text);

    drawTextLayer(dest, textBitmap, (WORD)x - 1, (WORD)y + 1, shadowColor);
    drawTextLayer(dest, textBitmap, (WORD)x,     (WORD)y + 1, shadowColor);
    drawTextLayer(dest, textBitmap, (WORD)x + 1, (WORD)y + 1, shadowColor);
    drawTextLayer(dest, textBitmap, (WORD)x,     (WORD)y + 2, shadowColor);
    drawTextLayer(dest, textBitmap, (WORD)x,     (WORD)y,     color);
}

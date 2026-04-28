#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include <ace/types.h>

void gameCameraCreate(void);
void gameCameraSetFocus(UWORD worldX, UWORD worldY);
void gameCameraProcess(void);
void gameCameraDestroy(void);

#endif /* GAME_CAMERA_H */

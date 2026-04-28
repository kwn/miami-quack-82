#ifndef AIM_H
#define AIM_H

#include <ace/types.h>
#include <ace/utils/extview.h>

void aimCreate(tView *view);
void aimProcess(void);
void aimDestroy(void);
UWORD aimGetX(void);
UWORD aimGetY(void);

#endif /* AIM_H */

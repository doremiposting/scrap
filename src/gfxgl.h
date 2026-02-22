#ifndef GFXGL_H
#define GFXGL_H

#include "gfxobj.h"

extern int pausesim, wiremesh, doprofile;

void drawm(const Mesh *m);
void resizegl();
void ginit();
void render();
void gkill();

#endif /* GFXGL_H */

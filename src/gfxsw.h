#ifndef GFXSW_H
#define GFXSW_H

#include <stdint.h>
#include "gfxobj.h"

extern int pausesim, wiremesh, doprofile;
extern uint32_t *framebuffer;
extern int fbwidth, fbheight;

void ginit();
void render();
void gkill();
void resizegfx(int ww, int wh);

#endif /* GFXSW_H */

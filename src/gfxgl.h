#ifndef GFXGL_H
#define GFXGL_H
#include "gfxobj.h"
#include "gfxttf.h"
#include "main.h"

TEMP
#define TREEREACH 1.5f

enum {
  PAN = 1,
  ROTATE,
  ZOOM
};
extern int wiremesh;
extern int animate;
extern int showcoll;
extern int scrw, scrh;
TEMP extern int treectr;
extern TTFAtlas hudfont;
extern float ctl[3];
extern float crt[2];
void update(int state, int ox, int nx, int oy, int ny);
void glinit();
void glkill();
void glreshape(int width, int height);
void drawmesh(const Mesh *m);
void render();
#endif /* GFXGL_H */

#ifndef GFXGL_H
#define GFXGL_H
#include "gfxobj.h"
#include "gfxttf.h"
#include "main.h"

enum {
  PAN = 1,
  ROTATE,
  ZOOM
};
enum {
  LIGHT_FULLBRIGHT = 0,
  LIGHT_NIGHT,
  LIGHT_DAY
};
extern int wiremesh;
extern int animate;
extern int showcoll;
extern int lightmode;
extern int scrw, scrh;
extern TTFAtlas hudfont;
extern float ctl[3];
extern float crt[2];
void update(int state, int ox, int nx, int oy, int ny);
void glinit();
void glkill();
void glreshape(int width, int height);
void drawmesh(const Mesh *m);
void render();

void dropselecteditem();
void cyclelighting();
#endif /* GFXGL_H */

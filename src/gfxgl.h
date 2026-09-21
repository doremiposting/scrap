#ifndef GL14_H
#define GL14_H
#include "gfxobj.h"
enum {
  PAN = 1,
  ROTATE,
  ZOOM
};
extern int wiremesh;
extern int animate;
extern float ctl[3];
extern float crt[2];
void update(int state, int ox, int nx, int oy, int ny);
void glinit();
void glkill();
void glreshape(int width, int height);
void drawmesh(const Mesh *m);
void render();
#endif /* GL14_H */

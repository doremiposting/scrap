#ifndef GFXGL_H
#define GFXGL_H

typedef struct {
  float x1, y1, x2, y2, x3, y3;
} Tri2df;

void ginit();
void render();
void gkill();

#endif /* GFXGL_H */

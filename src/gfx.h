#ifndef GFXH_H
#define GFXH_H
#include "main.h"

typedef struct {
  int x1, y1, x2, y2, x3, y3;
} Tri2d;

int intri2d(Tri2d t, int x, int y);
int tri2drbow(Tri2d t, int x, int y);
int tri2dc(Tri2d t, int x, int y, int r, int g, int b);
int tri2drbary(Tri2d t, int x, int y);

#endif /* GFXH_H */

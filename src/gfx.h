#ifndef GFXH_H
#define GFXH_H
#include "main.h"

typedef struct {
    int x1, y1, x2, y2, x3, y3;
} Tri2d;

forceinline double tri2darea(int x1, int y1, int x2, int y2, int x3, int y3);
int intri2d(Tri2d t, int x, int y);
int tri2drbow(Tri2d t, int x, int y);
int tri2dc(Tri2d t, int x, int y, int r, int g, int b);

#endif /* GFXH_H */
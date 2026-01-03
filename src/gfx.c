#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "gfx.h"

forceinline
double
tri2darea(int x1, int y1, int x2, int y2, int x3, int y3) {
   return fabs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2.0);
}

int
intri2d(Tri2d t, int x, int y) {
  return (tri2darea(t.x1, t.y1, t.x2, t.y2, t.x3, t.y3) ==
    (tri2darea(x, y, t.x2, t.y2, t.x3, t.y3) +
    tri2darea(t.x1, t.y1, x, y, t.x3, t.y3) +
    tri2darea(t.x1, t.y1, t.x2, t.y2, x, y)));
}
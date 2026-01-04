#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "gfx.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

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

int
tri2duv(Tri2d t, int x, int y) {
  int color;
  int r, g, b, a;
  int d1, d2, d3;
  float maxdist;
  a = 0xFF;
  /* ASSUME p1 = red, p2 = green, p3 = blue */
  d1 = (int)(sqrt(pow((x - t.x1),2) + pow((y - t.y1),2)));
  d2 = (int)(sqrt(pow((x - t.x2),2) + pow((y - t.y2),2)));
  d3 = (int)(sqrt(pow((x - t.x3),2) + pow((y - t.y3),2)));

  maxdist = fmax(fmax(d1, d2), d3);
  if (maxdist == 0) { maxdist = 1; }
  
  r = (int)(0xFF - (d1 * 0xFF / maxdist));
  g = (int)(0xFF - (d2 * 0xFF / maxdist));
  b = (int)(0xFF - (d3 * 0xFF / maxdist));

  r %= 0xFF; g %= 0xFF; b %= 0xFF;

  color =  ((a << 24) | (r << 16) | (g << 8) | (b));
  return color;
}
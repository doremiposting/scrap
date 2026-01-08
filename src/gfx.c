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

int
tri2drbow(Tri2d t, int x, int y) {
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
  
  r = (int)(0xFF - (d1 * 0xFF / maxdist)) % 0xFF;
  g = (int)(0xFF - (d2 * 0xFF / maxdist)) % 0xFF;
  b = (int)(0xFF - (d3 * 0xFF / maxdist)) % 0xFF;

  color =  ((a << 24) | (r << 16) | (g << 8) | (b));
  return color;
}

int
mix2colors(int c1, int c2, int u1, int det) {
  int r1, g1, b1, r2, g2, b2, r3, g3, b3, u2, a;
  a = 0xFF; /* TODO: Don't assume alpha. Maybe another function for variable alpha? */
  r1 = c1&(0x00FF0000)>>(16);
  g1 = c1&(0x0000FF00)>>(8);
  b1 = c1&(0x000000FF)>>(4);
  r2 = c2&(0x00FF0000)>>(16);
  g2 = c2&(0x0000FF00)>>(8);
  b2 = c2&(0x000000FF)>>(4);

  if (!(det)) {
    u2 = det - u1;
    r3 = (r1*u1 + r2*u2)/det;
    g3 = (g1*u1 + g2*u2)/det;
    b3 = (b1*u1 + b2*u2)/det;

    return ((a << 24) | (r3 << 16)| (g3 << 8) | (b3));
  } else { return 0; }
}

int
mix3colors(int c1, int c2, int c3, int u1, int u2, int det) {
  int r1, g1, b1, r2, g2, b2, r3, g3, b3, u3, r4, g4, b4, a;
  a = 0xFF; /* TODO: Don't assume alpha. Maybe another function for variable alpha? */
  r1 = c1&(0x00FF0000)>>(16);
  g1 = c1&(0x0000FF00)>>(8);
  b1 = c1&(0x000000FF)>>(4);
  r2 = c2&(0x00FF0000)>>(16);
  g2 = c2&(0x0000FF00)>>(8);
  b2 = c2&(0x000000FF)>>(4);
  r3 = c3&(0x00FF0000)>>(16);
  g3 = c3&(0x0000FF00)>>(8);
  b3 = c3&(0x000000FF)>>(4);

  if (!(det)) {
    u3 = det - u1 - u2;
    r4 = (r1*u1 + r2*u2 + r3*u3)/det;
    g4 = (g1*u1 + g2*u2 + g3*u3)/det;
    b4 = (b1*u1 + b2*u2 + b3*u3)/det;

    return ((a << 24) | (r4 << 16)| (g4 << 8) | (b4));
  } else { return 0; }
}

int
tri2dc(Tri2d t, int x, int y, int r, int g, int b) {
  int color;
  int pr, pg, pb, a;
  int d1, d2, d3;
  float maxdist;
  a = 0xFF;
  /* ASSUME p1 = red, p2 = green, p3 = blue */
  d1 = (int)(sqrt(pow((x - t.x1),2) + pow((y - t.y1),2)));
  d2 = (int)(sqrt(pow((x - t.x2),2) + pow((y - t.y2),2)));
  d3 = (int)(sqrt(pow((x - t.x3),2) + pow((y - t.y3),2)));

  maxdist = fmax(fmax(d1, d2), d3);
  if (maxdist == 0) { maxdist = 1; }
  
  /* TODO: Band-aid fix just to test animation */
  pr = mix2colors(((int)(0xFF - (d1 * 0xFF / maxdist)) % 0xFF), r, d1, (int)maxdist);
  pg = mix2colors(((int)(0xFF - (d2 * 0xFF / maxdist)) % 0xFF), b, d2, (int)maxdist);
  pb = mix2colors(((int)(0xFF - (d3 * 0xFF / maxdist)) % 0xFF), g, d3, (int)maxdist);

  color =  ((a << 24) | (pr << 16) | (pg << 8) | (pb));
  return color;
}
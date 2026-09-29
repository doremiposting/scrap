#include <math.h>
#include "gpcoll.h"

int
aabbcomp (
    float ax, float ay, float az, const Aabbshape *a,
    float bx, float by, float bz, const Aabbshape *b,
    float *pushx, float *pushy, float *pushz
    ) {
  float dx, dy, dz;
  float ox, oy, oz;

  dx = bx - ax; dy = by - ay; dz = bz - az;
  ox = (a->hx + b->hx) - fabsf(dx);
  oy = (a->hy + b->hy) - fabsf(dy);
  oz = (a->hz + b->hz) - fabsf(dz);
  if (ox <= 0 || oy <= 0 || oz <= 0) { return 0; }

  *pushx = *pushy = *pushz = 0;
  if (ox <= oy && ox <= oz) {
    *pushx = (dx < 0) ? ox : -ox;
  } else if (oy <= ox && oy <= oz) {
    *pushy = (dy < 0) ? oy : -oy;
  } else {
    *pushz = (dz < 0) ? oz : -oz;
  }
  return 1;
}

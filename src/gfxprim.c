#include <stdlib.h>
#include <string.h>

#include "gfxobj.h"
#include "gfxprim.h"

/* NOTE: parameters are half-extents. buildabox(1,1,1)
 * gives a 2x2x2 cube centered on its own origin. */
Mesh *
buildabox(float hx, float hy, float hz) {
  Mesh *m;
  Vertex *v;
  unsigned int *idx;
  int i;

  static const float sx[8] = {-1,1,1,-1,-1,1,1,-1};
  static const float sy[8] = {-1,-1,1,1,-1,-1,1,1};
  static const float sz[8] = {-1,-1,-1,-1,1,1,1,1};
  static const unsigned int boxidx[36] = {
    0,3,2, 0,2,1, /* -Z */
    4,5,6, 4,6,7, /* +Z */
    0,4,7, 0,7,3, /* -X */
    1,2,6, 1,6,5, /* +X */
    0,1,5, 0,5,4, /* -Y */
    3,7,6, 3,6,2  /* +Y */
  };

  v = calloc(8, sizeof(Vertex));
  idx = calloc(36, sizeof(unsigned int));
  for (i = 0; i < 8; i++) {
    v[i].x = sx[i] * hx;
    v[i].y = sy[i] * hy;
    v[i].z = sz[i] * hz;
  }
  memcpy(idx, boxidx, sizeof(boxidx));
  m = calloc(1, sizeof(Mesh));
  m->v = v;
  m->cnt = 8;
  m->idx = idx;
  m->idxc = 36;
  return m;
}

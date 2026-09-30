#include <stdlib.h>
#if defined(__linux__)
#include <stddef.h>
#endif
#include <math.h>

struct Terrainvertex {
  float x,y,z;
};
struct Tvfieldtri {
  struct Terrainvertex a,b,c;
};
#include "gfxterrain.h"
const int TERRAIN_WIDTH = 128;
const int TERRAIN_HEIGHT = 128;
const int TERRAIN_STRIDE = 2*TERRAIN_HEIGHT;
const float TERRAIN_SPACING = 1.0f;
TV *tvfield;
unsigned int *triia;
size_t triiacnt;
TVFT *ttris;

float
terrainheight(int x, int y) {
  return (sin(x*0.1) * cos(y*0.1));
}
float
terrainheightf(float x, float y) {
  return (sinf(x*0.1) * cosf(y*0.1));
}

TV *
newtvfield() {
  TV *t;
  t = calloc(4 * TERRAIN_WIDTH * TERRAIN_HEIGHT + 1, sizeof(TV));
  if (!t) { return NULL; }
  else { return t; }
}

unsigned int *
newtvindexarray() {
  unsigned int *t;
  t = calloc(6 * 4 * TERRAIN_WIDTH * TERRAIN_HEIGHT + 1, sizeof(unsigned int));
  if (!t) { return NULL; }
  else { return t; }
}

void
destroytvfield(TV *t) { if (t) { free(t); } }

void
buildtvs() {
  int i, j;
  for (i = -TERRAIN_HEIGHT; i < TERRAIN_HEIGHT; i++) {
    for (j = -TERRAIN_WIDTH; j < TERRAIN_WIDTH; j++) {
      /* XXX: Don't forget that Z is not up,
       * no matter how much your instincts fight you... */
      tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].x = j*TERRAIN_SPACING;
      tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].z = i*TERRAIN_SPACING;
      tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].y = terrainheight(i, j);
    }
  }
}

TVFT *
newtriangles() {
  TVFT *t;
  t = calloc(4 * 2 * TERRAIN_WIDTH * TERRAIN_HEIGHT + 1, sizeof(TVFT));
  if (!t) { return NULL; }
  else { return t;}
}

void
destroytriangles(TVFT *t) { if (t) { free(t); } }

/* XXX:
IF THE TV LOOKS LIKE
A---B
|   |
C---D
WE NEED TO TRAVERSE TRIANGLES AS
A---B
| / |
C---D
A->B->C, THEN B->D->C
XXX: ACTUALLY STRIKE THAT, YOU HAVE TO DO IT THE OTHER WAY AROUND
C->B->A, THEN C->D->B
OTHERWISE YOU TURN ON BACK-FACE CULLING AND THE FUCKIN THIS ONLY
VISIBLE FROM BELOW, LMFAO
*/
void
buildia() {
  int i, j, k, base;
  for (i = -TERRAIN_HEIGHT; i < TERRAIN_HEIGHT-1; i++) {
    for (j = -TERRAIN_WIDTH; j < TERRAIN_WIDTH-1; j++) {
      k = (i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH);
      base = 6*k;
      triia[base] = k+TERRAIN_STRIDE;
      triia[base+1] = k+1;
      triia[base+2] = k;
      triia[base+3] = k+TERRAIN_STRIDE;
      triia[base+4] = k+1+TERRAIN_STRIDE;
      triia[base+5] = k+1;
      triiacnt += 6;
    }
  }
}
void
buildtvfts() {
  int i, j, k;
  for (i = -TERRAIN_HEIGHT; i < TERRAIN_HEIGHT; i++) {
    for (j = -TERRAIN_WIDTH; j < TERRAIN_WIDTH; j++) {
      k = 0;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].a.x =
        tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].x;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].a.y =
        tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].y;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].a.z =
        tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].z;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].b.x =
        tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)+1].x;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].b.y =
        tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)+1].y;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].b.z =
        tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)+1].z;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].c.x =
        tvfield[((i+TERRAIN_HEIGHT)+1)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].x;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].c.y =
        tvfield[((i+TERRAIN_HEIGHT)+1)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].y;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].c.z =
        tvfield[((i+TERRAIN_HEIGHT)+1)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].z;
      k = 1;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].a.x =
        tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)+1].x;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].a.y =
        tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)+1].y;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].a.z =
        tvfield[(i+TERRAIN_HEIGHT)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)+1].z;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].b.x =
        tvfield[((i+TERRAIN_HEIGHT)+1)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)+1].x;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].b.y =
        tvfield[((i+TERRAIN_HEIGHT)+1)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)+1].y;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].b.z =
        tvfield[((i+TERRAIN_HEIGHT)+1)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)+1].z;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].c.x =
        tvfield[((i+TERRAIN_HEIGHT)+1)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].x;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].c.y =
        tvfield[((i+TERRAIN_HEIGHT)+1)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].y;
      ttris[2*((i+TERRAIN_HEIGHT)*TERRAIN_STRIDE+(j+TERRAIN_WIDTH))+k].c.z =
        tvfield[((i+TERRAIN_HEIGHT)+1)*TERRAIN_STRIDE + (j+TERRAIN_WIDTH)].z;
    }
  }
}

/* NOTE: Returns 0 if (x,z) is outside the mesh. */
/* NOTE: Splits here must match buildia() in winding. */
int
terrainheightat(float x, float z, float *h) {
  int cx, cz, base;
  float fx, fz, h00, h10, h01, h11;

  fx = x / TERRAIN_SPACING; fz = z / TERRAIN_SPACING;
  cx = (int)floorf(fx); cz = (int)floorf(fz);
  if (cx < -TERRAIN_WIDTH || cx > TERRAIN_WIDTH-2) { return 0; }
  if (cz < -TERRAIN_HEIGHT || cz > TERRAIN_HEIGHT-2) { return 0; }
  fx -= (float)cx; fz -= (float)cz;
  base = (cz + TERRAIN_HEIGHT) * TERRAIN_STRIDE + (cx + TERRAIN_WIDTH);
  h00 = tvfield[base].y;
  h10 = tvfield[base+1].y;
  h01 = tvfield[base + TERRAIN_STRIDE].y;
  h11 = tvfield[base + TERRAIN_STRIDE+1].y;
  if (fx + fz <= 1.0f) {
    *h = h00 + fx*(h10-h00) + fz*(h01-h00);
  } else {
    *h = h11 + (1.0f-fx)*(h01-h11) + (1.0f-fz)*(h10-h11);
  }
  return 1;
}

int
terrainpushout(float x, float feety, float z, float *pushy) {
  float ground;

  if (!terrainheightat(x, z, &ground)) { return 0; }
  if (feety >= ground) { return 0; }
  *pushy = ground - feety;
  return 1;
}

void
terrbuildup() {
  tvfield = newtvfield();
  buildtvs();
  triia = newtvindexarray();
  triiacnt = 0;
  buildia();
  /*
  ttris = newtriangles();
  buildtvfts();
  */
}

void
terrteardown() {
  destroytvfield(tvfield);
  /*
  destroytriangles(ttris);
  */
}


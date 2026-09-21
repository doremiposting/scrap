#ifndef TERRAIN_H
#define TERRAIN_H

extern const int TERRAIN_WIDTH;
extern const int TERRAIN_HEIGHT;
extern const int TERRAIN_STRIDE;
extern const float TERRAIN_SPACING;

struct Terrainvertex {
  float x,y,z;
};
struct Tvfieldtri {
  struct Terrainvertex a,b,c;
};
typedef struct Terrainvertex TV;
typedef struct Tvfieldtri TVFT;
extern TV *tvfield;
extern unsigned int *triia;
#if defined(__linux__)
#include <stddef.h>
#endif
extern size_t triiacnt;

void terrbuildup();
void terrteardown();

/* XXX: I don't think we need to expose these functions, OpenGL doesn't care.
TV newtvfield();
void destroytvfield(TV t);
void buildtvs();
TVFT newtriangles();
void destroytriangles(TVFT t);
void buildtvfts();
*/

#endif /* TERRAIN_H */

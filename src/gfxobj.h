#ifndef GFXOBJ_H
#define GFXOBJ_H

typedef struct {
  float x, y, z;
  float nx, ny, nz;
  float u, v;
} Vertex;

typedef struct {
  Vertex *v;
  unsigned int cnt;
} Mesh;

typedef struct { float x, y, z; } Vec3f;
typedef struct { float u, v; } Vec2f;

Mesh *loadobj(const char *fn);

#endif /* GFXOBJ_H */

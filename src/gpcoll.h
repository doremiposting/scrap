#ifndef GPCOLL_H
#define GPCOLL_H

typedef enum {
  SHAPE_NONE,
  SHAPE_AABB,
  SHAPE_END
} Shapeflvr;

typedef struct {
  float hx, hy, hz;
} Aabbshape;

typedef struct {
  Shapeflvr kind;
  /* union { */
    Aabbshape aabb;
  /* } as ; */
  float offx, offy, offz;
} Collshape;

int aabbcomp(
    float ax, float ay, float az, const Aabbshape *a,
    float bx, float by, float bz, const Aabbshape *b,
    float *pushx, float *pushy, float *pushz
);

#endif /* GPCOLL_H */

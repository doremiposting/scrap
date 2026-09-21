#ifndef GPPLAYER_H
#define GPPLAYER_H

#include "gpchar.h"
/* TODO: This will need to be a md3 later on for animation support... */
#include "gfxobj.h"

typedef enum { CAMERA_FOLLOW, CAMERA_FP, CAMERA_FREECAM, CAMERA_END } Cameramode;
typedef struct Camerastruct {
  Cameramode cm;
  float x, y, z;
  float pitch, /* up-down */ yaw, /* lateral side-to-side */ roll; /* circular rotation */
} Camera;
typedef struct Playerstruct {
  unsigned int id;
/* TODO: This will need to be a md3 later on for animation support... */
  Mesh *m;
  Camera *view;
  float x, y, z;
  float dx, dy, dz;
  float pitch, /* up-down */ yaw, /* lateral side-to-side */ roll; /* circular rotation */
} Player;
extern Player *P;
#define CDX(P) (P->view->x - P->x)
#define CDY(P) (P->view->y - P->y)
#define CDZ(P) (P->view->z - P->z)

unsigned int playerbuildup(const char *meshfp);
void playerteardown(unsigned int id);
void playermove2(float x, float y, float z);
void playermovealong(float dx, float dy, float dz); 
void playerrot2(float p, float y, float r);
void playerrotalong(float dp, float dy, float dr);
void projectcamera();
float projectorg();
void playerintegrate(float dt, float ax, float ay, float az);
Camera * buildupcam(float x, float y, float z, float p, float yaw, float r, Cameramode cm);
void teardowncam(Camera *c);
void cammove2(Camera *c, float x, float y, float z);
void cammovealong(Camera *c, float dx, float dy, float dz);
void orbitcam(Camera *c, float dth, float r);
void camlookat(Camera *c, float p, float y, float r);
void camrot2(Camera *c, float p, float y, float r);
void camrotalong(Camera *c, float dp, float dy, float dr);

#endif /* GPPLAYER_H */

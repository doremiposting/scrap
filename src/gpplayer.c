#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* TODO: This will need to be a md3 later on for animation support... */
#include "gfxobj.h"
#include "gpchar.h"
#include "gfxterrain.h"

#include "gpplayer.h"
/* TODO: If we ever have multiplayer support, this will need to be an array of... */
Player *P;

unsigned int
playerbuildup(const char *meshfp) {
  /* TODO: If we ever have multiplayer support, this will need to be a
   * pointer to a new struct of. For now, just make a player with id 0
   * using the global *P */
  P = calloc(1, sizeof(Player));
  P->id = 0;
  /* TODO: This will need to be a md3 later on for animation support... */
  P->m = loadobj(meshfp);
  P->foot = meshfootoffset(P->m);
  P->mv = PLAYER_STANDING;
  /* TODO: When this fails to load, 0 gets passed to drawm and then segfault.
   * We need a way to fail loudly... */
  P->view = buildupcam(0, 3, 5, 0, 0, 0, CAMERA_FREECAM);
  /* P->x = P->y = P->z = 0; */
  P->x = P->z = 0; P->y = 3;
  P->pitch = P->yaw = P->pitch = 0;
  return P->id;
}
void
playerteardown(unsigned int id) {
  /* TODO: If we ever have multiplayer support, this will need to be a
   * pointer to a new struct of. For now, ignore id, because it'll always
   * be 0, and teardown the global player struct *P */
  (void)(id);
  teardowncam(P->view);
  killmesh(P->m);
  free(P);
}
void
playermove2(float x, float y, float z) {
  P->x = x; P->y = y; P->z = z;
}
void
playermovealong(float dx, float dy, float dz) {
  P->x += dx; P->y += dy; P->z += dz;
}
void playerheight(float y) { P->y = y; }
void
playerclamptoterrain() {
  int trix, triz;
  float ground;
  trix = (int)floorf(P->x); triz = (int)floorf(P->z);
  ground = tvfield[(triz+TERRAIN_HEIGHT)*TERRAIN_STRIDE
      + (trix+TERRAIN_WIDTH)].y;
  if (P->y - P->foot < ground) {
    P->y = ground + P->foot; P->dy = 0;
  } else { P->dy -= 0.5; }
}
void
playerintegrate(float dt, float ax, float ay, float az) {
  float speed, scale, damp;
  const float MAX = 280.0f, FRICTION = 10.0f;
  P->dx += ax; P->dy += ay; P->dz += az;
  speed = sqrt(P->dx*P->dx + P->dy*P->dy + P->dz*P->dz);
  if (speed > MAX) {
    scale = MAX / speed;
    P->dx *= scale; P->dy *= scale; P->dz *= scale;
  }
  P->x += P->dx * dt; P->y += P->dy * dt; P->z += P->dz * dt;
  damp = fmaxf(0, 1-FRICTION*dt);
  P->dx *= damp; P->dy *= damp; P->dz *= damp;
  playerclamptoterrain();
}
void
playerrot2(float p, float y, float r) {
  P->pitch = p; P->yaw = y; P->roll = r;
}
void
playerrotalong(float dp, float dy, float dr) {
  P->pitch += dp; P->yaw += dy; P->roll += dr;
}
float
terrainfloor(float x, float z) {

}
void
projectcamera() {
  
}
float
projectorg() {
  return atan2f(P->z, P->x);
}
static void 
nmlzcmraproj(float ndx, float ndz) {
  float h;
  h = sqrtf(CDX(P)*CDX(P) + CDZ(P)*CDZ(P));
  ndx = CDZ(P) / h; ndz = CDZ(P) / h;
}
Camera *
buildupcam(float x, float y, float z, float p, float yaw, float r, Cameramode cm) {
  Camera *c;
  c = calloc(1, sizeof(Camera));
  c->cm = cm;
  c->x = x; c->y = y; c->z = z;
  c->pitch = p; c->yaw = yaw; c->roll = r;
  return c;
}
void teardowncam(Camera *c) { free(c); }
void
cammove2(Camera *c, float x, float y, float z) {
  c->x = x; c->y = y; c->z = z;
}
void
cammovealong(Camera *c, float dx, float dy, float dz) {
  c->x += dx; c->y += dy; c->z += dz;
}
void
camlookat(Camera *c, float x, float y, float z) {
  float dx, dy, dz, hypo;
  dx = x - c->x; dy = y - c->y; dz = z - c->z;
  c->yaw = atan2f(dx, dz);
  hypo = sqrtf(dx*dx + dz*dz);
  c->pitch = atan2f(-dy, hypo);
}
void
orbitcam(Camera *c, float dth, float r) {
  P->view->yaw += dth;
  P->view->x = P->x + cosf(P->view->yaw) * r;
  P->view->y = P->y + 3.0f;
  P->view->z = P->z + sinf(P->view->yaw) * r;
  projectcamera();
  camlookat(P->view, P->x, P->y, P->z);
}
void
camrot2(Camera *c, float p, float y, float r) {
  c->pitch = p; c->yaw = y; c->roll = r;
}
void
camrotalong(Camera *c, float dp, float dy, float dr) {
  c->pitch += dp; c->yaw += dy; c->roll += dr;
}

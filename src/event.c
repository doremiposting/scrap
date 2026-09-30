#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "event.h"
#include "gpchar.h"
#include "gpplayer.h"

char *evtable;
typedef enum {
  AXIS_X = 0, AXIS_Y, AXIS_Z, AXIS_NONE
} Acceldirs;
float accel[3];
typedef struct {
  float xscale;
  float yadj;
  float zscale;
} Shfactor;

Shfactor shoulderhang;

void
buildevtbl() {
  evtable = calloc((size_t)END_OF_EVENT_ENUM, sizeof(char));
  accel[0] = 0; accel[1] = 0; accel[2] = 0;
  shoulderhang.xscale = 12.0f;
  shoulderhang.zscale = 12.0f;
  shoulderhang.yadj = 12.0f;
}

void
killevtbl() {
  free(evtable);
}

void
setevent(Scrapevent e, char v) {
  evtable[e] = v;
}

void
handlephysics(double dt) {
  int i, j, hit;
  float px, py, pz, sx, sy, sz, pushx, pushy, pushz;
  playerintegrate(dt, accel[0], accel[1], accel[2]);
  accel[0] = accel[1] = accel[2] = 0;
  playercollide();
}

void
modshfctr(float dxs, float dym, float dzs) {
  shoulderhang.xscale += dxs;
  shoulderhang.yadj += dym;
  shoulderhang.zscale += dzs;
}

void
handleglobalevents(double dt) {
  static int onceonchange = 1;
  if (evtable[W_HELD] || evtable[S_HELD]) {
    if (evtable[W_HELD]) {
      accel[AXIS_Z] = fminf(280.0f, accel[AXIS_Z]+(100*cosf(P->yaw)*dt));
      accel[AXIS_X] = fminf(280.0f, accel[AXIS_X]+(100*sinf(P->yaw)*dt));
    }
    if (evtable[S_HELD]) {
      accel[AXIS_Z] = fmaxf(-280.0f, accel[AXIS_Z]+(-100*cosf(P->yaw)*dt));
      accel[AXIS_X] = fmaxf(-280.0f, accel[AXIS_X]+(-100*sinf(P->yaw)*dt));
    }
  }
  if (evtable[A_HELD] || evtable[D_HELD]) {
    if (evtable[A_HELD]) {
      accel[AXIS_Z] = fminf(280.0f, accel[AXIS_Z]+(100*-sinf(P->yaw)*dt));
      accel[AXIS_X] = fminf(280.0f, accel[AXIS_X]+(100*cosf(P->yaw)*dt));
    }
    if (evtable[D_HELD]) {
      accel[AXIS_Z] = fmaxf(-280.0f, accel[AXIS_Z]+(-100*-sinf(P->yaw)*dt));
      accel[AXIS_X] = fmaxf(-280.0f, accel[AXIS_X]+(-100*cosf(P->yaw)*dt));
    }
  }
  
  if (P->view->cm == CAMERA_FOLLOW) {
    if (onceonchange) {
      P->view->yaw = P->yaw + M_PI;
      onceonchange = 0;
    }
    projectcamera();
    cammove2(P->view,
        P->x - sinf(P->view->yaw) * shoulderhang.xscale,
        P->y + tanf(P->view->pitch) * shoulderhang.yadj,
        P->z + cosf(P->view->yaw) * shoulderhang.zscale
    );
  }
  if (P->view->cm == CAMERA_FREECAM && !onceonchange) {
    onceonchange = 1;
  }
}

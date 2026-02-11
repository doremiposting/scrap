#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gfxobj.h"

#define LINEWIDTH 512

static int
saferead(FILE *f, char *buf, int sz) {
  return fgets(buf, sz, f) != NULL;
}

static void
objcnt(FILE *f, int *vc, int *vtc, int *vnc, int *fc) {
  char line[LINEWIDTH];
  *vc = *vtc = *vnc = *fc = 0;

  while (saferead(f, line, sizeof(line))) {
         if (!(strncmp(line, "v ",  2))) { (*vc)++; }
    else if (!(strncmp(line, "vt ", 3))) { (*vtc)++; }
    else if (!(strncmp(line, "vn ", 3))) { (*vnc)++; }
    else if (!(strncmp(line, "f ",  2))) { (*fc)++; }
  }

  rewind(f);
}

static int
parsevtx(const char *s, int *vi, int *ti, int *ni) {
  *ti = *ni = -1;
  return sscanf(s, "%d/%d/%d", vi, ti, ni);
}

Mesh *
loadobj(const char *fn) {
  FILE *f;
  int i, vc, vtc, vnc, fc, pi, ti, ni, oi;
  int tvi[3], tti[3], tni[3];
  char line[LINEWIDTH];
  Vec3f *pos, *norm, p, n;
  Vec2f *uv, t;
  Vertex *out, *v;
  Mesh *m;
  char *s;
  f = fopen(fn, "r");
  if (!f) { return NULL; }

  objcnt(f, &vc, &vtc, &vnc, &fc);
  pos = calloc(vc, sizeof(Vec3f));
  norm = calloc(vnc, sizeof(Vec3f));
  uv = calloc(vtc, sizeof(Vec2f));

  out = calloc(3*fc, sizeof(Vertex));
  pi = ni = ti = oi = 0;

  while (saferead(f, line, sizeof(line))) {
    if (!(strncmp(line, "v ", 2))) {
      sscanf(line + 2, "%f %f %f",
          &pos[pi].x,
          &pos[pi].y,
          &pos[pi].z);
      pi++;
    }
    else if (!(strncmp(line, "vn ", 3))) {
      sscanf(line + 3, "%f %f %f",
          &norm[ni].x,
          &norm[ni].y,
          &norm[ni].z);
      ni++;
    }
    else if (!(strncmp(line, "vt ", 3))) {
      sscanf(line + 3, "%f %f",
          &uv[ti].u,
          &uv[ti].v);
      ti++;
    }
    else if (!(strncmp(line, "f ", 2))) {
      s = strtok(line+2, " \t\n");
      for (i = 0; i < 3; i++) {
        parsevtx(s, &tvi[i], &tti[i], &tni[i]);
        s = strtok(NULL, " \t\n");
      }
      for (i = 0; i < 3; i++) {
        v = &out[oi++];
        p = pos[tvi[i] - 1];
        v->x = p.x; v->y = p.y; v->z = p.z;

        if (tni[i] > 0) {
          n = norm[tni[i] - 1];
          v->nx = n.x; v->ny = n.y; v->nz = n.z;
        } else { v->nx = v->ny = v->nz = 0.0f; }
        if (tti[i] > 0) {
          t = uv[tti[i] - 1];
          v->u = t.u; v->v = t.v;
        } else { v->u = v->v = 0.0f; }
      }
    }
  }

  fclose(f);
  free(pos);
  free(norm);
  free(uv);
  
  m = calloc(1, sizeof(Mesh));
  m->v = out;
  m->cnt = oi;
  return m;
}

void
killmesh(Mesh *m) {
  free(m->v);
  free(m);
}

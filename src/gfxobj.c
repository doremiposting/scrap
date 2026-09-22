#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "gfxobj.h"

#define LINEWIDTH 512

static int
saferead(FILE *f, char *buf, int sz) {
  return fgets(buf, sz, f) != NULL;
}

static void
parsef(const char *st, int *i) {
  char *s;
  unsigned int n;
  n = 0;
  s = strtok(st+2, " \t\n");
  if (s) {
    do {
      n++;
      s = strtok(NULL, " \t\n");
    } while (s);
  }
  if (n >= 3) { *i += (3 * (n-2)); }
}

static void
objcnt(FILE *f, int *vc, int *vtc, int *vnc, int *fc, unsigned int *idxc) {
  char line[LINEWIDTH];
  *vc = *vtc = *vnc = *fc = *idxc = 0;

  while (saferead(f, line, sizeof(line))) {
         if (!(strncmp(line, "v ",  2))) { (*vc)++; }
    else if (!(strncmp(line, "vt ", 3))) { (*vtc)++; }
    else if (!(strncmp(line, "vn ", 3))) { (*vnc)++; }
    else if (!(strncmp(line, "f ",  2))) {
      (*fc)++;
      parsef(line, idxc);
    }
  }

  rewind(f);
}

static int
parsevtx(const char *s, int *vi, int *ti, int *ni) {
  *vi = *ti = *ni = 0;
  if (!s) { return 0; }

  if (sscanf(s, "%d/%d/%d", vi, ti, ni) == 3) { return 3; }
  if (sscanf(s, "%d//%d", vi, ni) == 2) { return 2; }
  if (sscanf(s, "%d/%d", vi, ti) == 2) { return 2; }
  if (sscanf(s, "%d", vi) == 1) { return 1; }
  return 0;
}

float
meshfootoffset(Mesh *m) {
  float miny;
  size_t i;
  miny = FLT_MAX;
  for (i = 0; i < m->cnt ; i++) {
    if (m->v[i].y < miny) { miny = m->v[i].y; }
  }
  return -miny;
}

Mesh *
loadobj(const char *fn) {
  FILE *f;
  unsigned int idxc, idxi, idxbase;
  int i, vc, vtc, vnc, fc, pi, ti, ni, oi;
  int tvi[3], tti[3], tni[3];
  int v0[3], vprev[3], vcurr[3];
  char line[LINEWIDTH];
  Vec3f *pos, *norm, p, n;
  Vec2f *uv, t;
  Vertex *out, *v;
  Mesh *m;
  unsigned int *idx;
  char *s;
  size_t outcap;
  f = fopen(fn, "r");
  if (!f) { return NULL; }

  objcnt(f, &vc, &vtc, &vnc, &fc, &idxc);
  pos = calloc((size_t)(vc), sizeof(Vec3f));
  norm = calloc((size_t)(vnc), sizeof(Vec3f));
  uv = calloc((size_t)(vtc), sizeof(Vec2f));
  idx = calloc((size_t)(idxc), sizeof(unsigned int));

  outcap = (size_t)(3*fc);
  out = calloc(outcap, sizeof(Vertex));
  pi = ni = ti = oi = idxi = 0;

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
      if (!s) { continue; }
      parsevtx(s, &v0[0], &v0[1], &v0[2]);
      s = strtok(NULL, " \t\n");
      if (!s) { continue; }
      parsevtx(s, &vprev[0], &vprev[1], &vprev[2]);
      s = strtok(NULL, " \t\n");
      while (s) {
        parsevtx(s, &vcurr[0], &vcurr[1], &vcurr[2]);
        if (oi + 3 > (int)(outcap)) {
          outcap *= 2;
          out = realloc(out, outcap * sizeof(Vertex));
        }
        tvi[0] = v0[0]; tvi[1] = vprev[0]; tvi[2] = vcurr[0];
        tti[0] = v0[1]; tti[1] = vprev[1]; tti[2] = vcurr[1];
        tni[0] = v0[2]; tni[1] = vprev[2]; tni[2] = vcurr[2];
        /* TODO: Index deduplication */
        idxbase = (unsigned int)oi;
        idx[idxi] = idxbase; idx[idxi+1] = idxbase+1; idx[idxi+2] = idxbase+2;
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
				vprev[0] = vcurr[0];
				vprev[1] = vcurr[1];
				vprev[2] = vcurr[2];
        idxi += 3;
        s = strtok(NULL, " \t\n");
      }
    }
  }

  fclose(f);
  free(pos);
  free(norm);
  free(uv);
  
  m = calloc(1, sizeof(Mesh));
  m->v = out;
  m->cnt = (unsigned int)(oi);
  m->idx = idx;
  m->idxc = idxc;
  return m;
}

void
killmesh(Mesh *m) {
  free(m->v);
  free(m);
}

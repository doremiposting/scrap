#include <stdio.h>
#include <stdlib.h>

#include "gfxobj.h"

static int
saferead(FILE *f, char *buf, int sz) {
  return fgets(buf, size, f) != NULL;
}

static void
objcnt(FILE *f, int *vc, int *vtc, int *vnc, int *fc) {
  char line[512];
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

}

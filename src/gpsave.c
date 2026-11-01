#include <stdio.h>
#include <string.h>

#include "gpplayer.h"
#include "gpsave.h"

#define SAVEMAGIC "SCRP"
#define SAVEVERSION 1u

static void putu8(FILE *f, unsigned char v) { fputc(v, f); }

static void
putu32(FILE *f, unsigned int v) {
  putu8(f, (unsigned char)(v & 0xff));
  putu8(f, (unsigned char)((v>>8) & 0xff));
  putu8(f, (unsigned char)((v>>16) & 0xff));
  putu8(f, (unsigned char)((v>>24) & 0xff));
}

static void
putf32(FILE *f, float v) {
  unsigned int u;
  memcpy(&u, &v, sizeof(u));
  putu32(f, u);
}

static int
getu8(FILE *f, unsigned char *v) {
  int c;
  c = fgetc(f);
  if (c == EOF) { return 0; }
  *v = (unsigned char)c;
  return 1;
}

static int
getu32(FILE *f, unsigned int *v) {
  unsigned char b[4];
  int i;
  for (i = 0; i < 4; i++) {
    if (!getu8(f, &b[i])) { return 0; }
  }
  *v = (unsigned int)b[0] | ((unsigned int)b[1]<<8) |
    ((unsigned int)b[2]<<16) | ((unsigned int)b[3]<<24);
  return 1;
}

static int
getf32(FILE *f, float *v) {
  unsigned int u;
  if (!getu32(f, &u)) { return 0; }
  memcpy(v, &u, sizeof(u));
  return 1;
}

static void
putchunkhdr(FILE *f, const char *tag, unsigned int len) {
  fwrite(tag, 1, 4, f);
  putu32(f, len);
}

static int
getchunkhdr(FILE *f, char tag[4], unsigned int *len) {
  if (fread(tag, 1, 4, f) != 4) { return 0; }
  return getu32(f, len);
}

int
savegame(const char *fn) {
  FILE *f;
  int i;
  f = fopen(fn, "wb");
  if (!f) {
    fprintf(stderr, "savegame: couldn't open %s for writing\n", fn);
    return 0;
  }
  fwrite(SAVEMAGIC, 1, 4, f);
  putu32(f, SAVEVERSION);

  putchunkhdr(f, "POSN", 12);
  putf32(f, P->x); putf32(f, P->y); putf32(f, P->z);

  putchunkhdr(f, "ROTN", 12);
  putf32(f, P->pitch); putf32(f, P->yaw); putf32(f, P->roll);

  for (i = 0; i < PLEVEL_END; i++) {
    putchunkhdr(f, "STAT", 7);
    putu8(f, (unsigned char)i);
    putu32(f, P->S[i]);
    putu8(f, P->Cl[i]);
    putu8(f, P->Ml[i]);
  }

  putchunkhdr(f, "END ", 0);
  fclose(f);
  return 1;
}

int
loadgame(const char *fn) {
  FILE *f;
  char magic[4], tag[4];
  unsigned int version, len, xp;
  unsigned char statid, curlvl, maxlvl;

  f = fopen(fn, "rb");
  if (!f) {
    fprintf(stderr, "loadgame: couldn't open %s\n", fn);
    return 0;
  }
  if (fread(magic, 1, 4, f) != 4 || memcmp(magic, SAVEMAGIC, 4) != 0) {
    fprintf(stderr, "loadgame: %s isn't a scrap save file\n", fn);
    fclose(f);
    return 0;
  }
  if (!getu32(f, &version) || version != SAVEVERSION) {
    fprintf(stderr, "loadgame: %s unreadable (bad/unsupported version)\n", fn);
    fclose(f);
    return 0;
  }

  for (;;) {
    if (!getchunkhdr(f, tag, &len)) {
      fprintf(stderr, "loadgame: %s truncated (no EnD chunk)\n", fn);
      fclose(f);
      return 0;
    }
    if (!memcmp(tag, "END ", 4)) { break; }
    if (!memcmp(tag, "POSN", 4) && len == 12) {
      getf32(f, &P->x); getf32(f, &P->y); getf32(f, &P->z);
    } else if (!memcmp(tag, "ROTN", 4) && len == 12) {
      getf32(f, &P->pitch); getf32(f, &P->yaw); getf32(f, &P->roll);
    } else if (!memcmp(tag, "STAT", 4) && len == 7) {
      getu8(f, &statid); getu32(f, &xp); getu8(f, &curlvl); getu8(f, &maxlvl);
      if (statid < PLEVEL_END) {
        P->S[statid] = xp;
        P->Cl[statid] = curlvl;
        P->Ml[statid] = maxlvl;
      }
    } else {
      /* unrecognized tag or known tag with unexpected length */
      fseek(f, (long)len, SEEK_CUR);
    }
  }
  fclose(f);
  return 1;
}

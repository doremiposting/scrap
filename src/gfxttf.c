#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#elif defined(__MSC_VER)
#pragma warning(push)
#pragma warning(disable:4505) /* unreferenced local func */
#pragma warning(disable:4100) /* unreferenced formal prmtr */
#endif

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(__MSC_VER)
#pragma warning(pop)
#endif

#include "gfxttf.h"

#define ATLASW 512
#define ATLASH 512

int
ttfbuildatlas(TTFAtlas *a, const char *path, float pxheight) {
  FILE *fp;
  long flen;
  unsigned char *fbuf, *bitmap;
  stbtt_pack_context pc;
  stbtt_packedchar *cd;
  stbtt_fontinfo fi;
  int ascent, descent, linegap;
  float scale;

  fp = fopen(path, "rb");
  if (!fp) { return -1; }
  fseek(fp, 0, SEEK_END); flen = ftell(fp); rewind(fp);
  fbuf = calloc(flen, sizeof(char));
  if (!fbuf || fread(fbuf, 1, (size_t)flen, fp) != (size_t)flen * sizeof(char)) {
    fclose(fp); free(fbuf); return -1; 
  }
  fclose(fp);

  bitmap = calloc(ATLASW * ATLASH, sizeof(char));
  cd = calloc(TTF_NUMCHARS, sizeof(*cd));
  if (!bitmap || !cd) { free(fbuf); free(bitmap); free(cd); return -1; }
  if (!stbtt_PackBegin(&pc, bitmap, ATLASW, ATLASH, 0, 1, NULL)) {
    free(fbuf); free(bitmap); free(cd); return -1;
  }
  stbtt_PackSetOversampling(&pc, 2, 2);
  if (!stbtt_PackFontRange(&pc, fbuf, 0, pxheight,
        TTF_FIRSTCHAR, TTF_NUMCHARS, cd)) {
    free(fbuf); free(bitmap); free(cd); return -1;
  }
  stbtt_PackEnd(&pc);
#if 0
  {
    FILE *dbg = fopen("/tmp/atlas.pgm", "wb");
    if (dbg) {
      fprintf(dbg, "P5\n%d %d\n 255\n", ATLASW, ATLASH);
      fwrite(bitmap, 1, ATLASW * ATLASH, dbg);
      fclose(dbg);
    }
  }
#endif
  if (!stbtt_InitFont(&fi, fbuf, stbtt_GetFontOffsetForIndex(fbuf, 0))) {
    free(fbuf); free(bitmap); free(cd); return -1;
  }
  stbtt_GetFontVMetrics(&fi, &ascent, &descent, &linegap);
  scale = stbtt_ScaleForPixelHeight(&fi, pxheight);

  glGenTextures(1, &a->tex);
  glBindTexture(GL_TEXTURE_2D, a->tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  /* XXX: GL_INTENSITY deprecated starting in gl3.x, will need to replace if upgrading. */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, ATLASW, ATLASH, 0,
      GL_LUMINANCE, GL_UNSIGNED_BYTE, bitmap);
  /* TODO: We should allow the player to turn off linear filtering... */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  a->texw = ATLASW; a->texh = ATLASH;
  a->pxheight = pxheight;
  a->ascent = ascent * scale;
  a->descent = descent * scale;
  a->linegap = linegap * scale;
  a->chardata = cd;

  free(fbuf); free(bitmap);
  return 0;
}

void
ttffreeatlas(TTFAtlas *a) {
  glDeleteTextures(1, &a->tex);
  free(a->chardata);
  /* XXX: We don't own it, so don't free() it. */
  memset(a, 0, sizeof(*a));
}

float
ttftextw(const TTFAtlas *a, const char *s) {
  stbtt_packedchar *cd;
  const unsigned char *p;
  float w;
  cd = (stbtt_packedchar *)a->chardata;
  w = 0.0f;
  for (p = (const unsigned char *)s; *p; p++) {
    if (*p < TTF_FIRSTCHAR || *p >= TTF_FIRSTCHAR + TTF_NUMCHARS) {
      continue;
    }
    w += cd[*p - TTF_FIRSTCHAR].xadvance;
  }
  return w;
}

void
ttfdrawtxt(const TTFAtlas *a, float x, float y, const char *s) {
  stbtt_packedchar *cd;
  stbtt_aligned_quad q;
  const unsigned char *p;
  cd = (stbtt_packedchar *)a->chardata;
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, a->tex);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBegin(GL_QUADS);
    for (p = (const unsigned char *)s; *p; p++) {
      if (*p < TTF_FIRSTCHAR || *p >= TTF_FIRSTCHAR + TTF_NUMCHARS) {
        continue;
      }
      stbtt_GetPackedQuad(cd, a->texw, a->texh, *p - TTF_FIRSTCHAR,
          &x, &y, &q, 1);
      glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
      glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
      glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
      glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
    }
  glEnd();
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
}

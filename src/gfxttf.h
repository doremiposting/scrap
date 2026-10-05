#ifndef GFXTTF_H
#define GFXTTF_H
#include <GL/gl.h>

#define TTF_FIRSTCHAR 32
#define TTF_NUMCHARS 95 /* ' ' @ 32 thru '~' @ 126 */

typedef struct {
  GLuint tex;
  int texw, texh;
  float pxheight;
  float ascent, descent, linegap;
  void *chardata; /* stbtt_packedchar[TTF_NUMCHARS]; */
} TTFAtlas;

int ttfbuildatlas(TTFAtlas *a, const char *path, float pxheight);
void ttffreeatlas(TTFAtlas *a);
float ttftextw(const TTFAtlas *a, const char *s);
void ttfdrawtxt(const TTFAtlas *a, float x, float y, const char *s);

#endif /* GFXTTF_H */

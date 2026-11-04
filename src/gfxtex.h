#ifndef GFXTEX_H
#define GFXTEX_H

typedef struct {
  unsigned int id;
  int w, h;
} Texture;

int loadtexture(const char *fn, Texture *out);
void killtexture(Texture *t);
void drawicon(const Texture *t, float x, float y, float w, float h);

#endif /* GFXTEX_H */

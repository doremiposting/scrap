#include <stdio.h>
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(__MSC_VER)
#pragma warning(pop)
#endif

#include "gfxtex.h"

int
loadtexture(const char *fn, Texture *out) {
  unsigned char *data;
  int channels;
  GLenum format;

  data = stbi_load(fn, &out->w, &out->h, &channels, 0);
  if (!data) {
    fprintf(stderr, "loadtexture: couldn't load %s: %s\n",
        fn, stbi_failure_reason());
    return 0;
  }
  switch (channels) {
    case 1: format = GL_LUMINANCE; break;
    case 2: format = GL_LUMINANCE_ALPHA; break;
    case 3: format = GL_RGB; break;
    case 4: format = GL_RGBA; break;
    default:
      fprintf(stderr, "loadtexture: %s has unsupported channel count %d\n",
          fn, channels);
      stbi_image_free(data);
      return 0;
  }
  glGenTextures(1, &out->id);
  glBindTexture(GL_TEXTURE_2D, out->id);
  /* XXX: GL assumes 4-byte-aligned rows, make sure source image is multiple
   * of 4 or shearing will occur. */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, out->w, out->h, 0,
      format, GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);
  return 1;
}

void
killtexture(Texture *t) {
  glDeleteTextures(1, &t->id);
}

void
drawicon(const Texture *t, float x, float y, float w, float h) {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor3f(1.0f, 1.0f, 1.0f);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, t->id);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
  glTexCoord2f(1.0f, 0.0f); glVertex2f(x+w, y);
  glTexCoord2f(1.0f, 1.0f); glVertex2f(x+w, y+h);
  glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y+h);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

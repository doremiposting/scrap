#ifndef SFX_H
#define SFX_H

typedef struct {
  short *pcm;
  size_t frames;
  int rate;
  int chnls;
} Soundfx;

typedef enum {
  SFX_BGM,
  SFX_BOOM,
  SFX_COUNT
} SfxID;

typedef struct {
  int on;
  int cut;
} Triggersfx;

typedef struct {
  Soundfx *snd;
  SfxID id;
  size_t position;
  float volume;
  int active;
  int loop;
} Voice;

#endif /* SFX_H */

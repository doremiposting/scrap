#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "sfxal.h"

#define OALSAMPLERATE 44100
/* TODO: Can't we anchor this to M_PI instead? */
#define OALTWOPI 6.283185307179586f

static ALCdevice *dev;
static ALCcontext *ctx;

static int
alcheckerror(const char *where) {
  ALenum err;
  err = alGetError();
  if (err == AL_NO_ERROR) { return 0; }
  fprintf(stderr, "OpenAL error at %s: %s\n", where, alGetString(err));
  return 1;
}

static int
alccheckerror(ALCdevice *d, const char *where) {
  ALenum err;
  err = alcGetError(d);
  if (err == ALC_NO_ERROR) { return 0; }
  fprintf(stderr, "OpenAL ALC error at %s: %s\n", where, alcGetString(d, err));
  return 1;
}

int
alinit() {
  dev = NULL; ctx = NULL;
  dev = alcOpenDevice(NULL);
  if (!dev) {
    fprintf(stderr, "alinit: alcOpenDevice found no device\n");
    return 0;
  }
  ctx = alcCreateContext(dev, NULL);
  if (alccheckerror(dev, "alcCreateContext") || !ctx) {
    alcCloseDevice(dev);
    dev = NULL;
    return 0;
  }
  if (!alcMakeContextCurrent(ctx)) {
    alccheckerror(dev, "alcMakeContextCurrent");
    alcDestroyContext(ctx);
    alcCloseDevice(dev);
    ctx = NULL; dev = NULL;
    return 0;
  }
  return 1;
}

void
alkill() {
  alcMakeContextCurrent(NULL);
  if (ctx) {
    alcDestroyContext(ctx);
    ctx = NULL;
  }
  if (dev) {
    alcCloseDevice(dev);
    dev = NULL;
  }
}

int
playtesttone(float freq, float duration) {
  size_t samplecnt;
  int16_t *samples;
  size_t i;
  float t, s;
  ALuint buffer;
  ALuint source;
  ALint state;
  struct timespec polldelay;

  samplecnt = (size_t)(duration * (float)OALSAMPLERATE);
  samples = calloc(samplecnt, sizeof(int16_t));
  if (!samples) {
    fprintf(stderr, "playtesttone: couldn't get memory for test tone!\n");
    return 0;
  }

  for (i = 0; i < samplecnt; i++) {
    t = (float)i / (float)OALSAMPLERATE;
    s = sinf(OALTWOPI * freq * t);
    samples[i] = (int16_t)(s * 32767.0f * 0.5f);
  }

  alGenBuffers(1, &buffer);
  if (alcheckerror("alGenBuffers")) {
    free(samples);
    return 0;
  }

  alBufferData(buffer, AL_FORMAT_MONO16, samples,
      (ALsizei)(samplecnt * sizeof(int16_t)), OALSAMPLERATE);
  free(samples);
  if (alcheckerror("alBufferData")) {
    alDeleteBuffers(1, &buffer);
    return 0;
  }
  alGenSources(1, &source);
  if (alcheckerror("alGenSources")) {
    alDeleteBuffers(1, &buffer);
    return 0;
  }

  alSourcei(source, AL_BUFFER, (ALint)buffer);
  alSourcePlay(source);
  if (alcheckerror("alSourcePlay")) {
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    return 0;
  }

  polldelay.tv_sec = 0;
  polldelay.tv_nsec = 10000000;

  do {
    nanosleep(&polldelay, NULL);
    alGetSourcei(source, AL_SOURCE_STATE, &state);
  } while (state == AL_PLAYING);
  
  alDeleteSources(1, &source);
  alDeleteBuffers(1, &buffer);
  return 1;
}


#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <mpg123.h>
#include <AudioUnit/AudioUnit.h>

#include "sfx.h"
#include "sfxca.h"
#include "main.h"

#define MAXVCS 32
#define CHANNELS 2

static AudioUnit outputunit;
static Soundfx *s[SFX_COUNT];
static Triggersfx triggers[SFX_COUNT];
static Voice voices[MAXVCS];
static unsigned int voicecnt;
static pthread_mutex_t sfxmutex;

static Soundfx *
loadsnd(const char *path) {
  mpg123_handle *mh;
  Soundfx *ss;
  unsigned char *a, b[8192];
  size_t cap, done;
  long rate;
  int err, chnls, enc, r;
  mh = mpg123_new(NULL, &err);
  a = NULL;
  cap = 0;
  ss = calloc(1, sizeof(Soundfx));
  mpg123_open(mh, path);
  mpg123_getformat(mh, &rate, &chnls, &enc);
  mpg123_format_none(mh);
  mpg123_format(mh, rate, chnls, MPG123_ENC_SIGNED_16);
  ss->rate = (int)rate;
  ss->chnls = chnls;
  while ((r = mpg123_read(mh, b, sizeof(b), &done)) != MPG123_DONE) {
    if (r == MPG123_OK && done > 0) {
      a = realloc(a, cap+done);
      memcpy(a+cap, b, done);
      cap += done;
    }
  }
  ss->pcm = (short *)a;
  ss->frames = cap / ((size_t)chnls * sizeof(short));
  mpg123_close(mh);
  mpg123_delete(mh);
  return ss;
}

static void
mixaudio(void *buf, unsigned int nframes) {
  short *out;
  unsigned int v, f;
  int ch, mixed;
  long mixable, remaining;
  size_t mixi, srci;
  Voice *voice;
  Soundfx *sfx;
  unsigned int i;
  out = (short *)buf;
  memset(buf, 0, nframes * CHANNELS * sizeof(int16_t));
  pthread_mutex_lock(&sfxmutex);
  for (i = 0; i < SFX_COUNT; i++) {
    if (!triggers[i].on) { continue; }
    sfx = s[i];
    voice = NULL;
    for (v = 0; v < MAXVCS; v++) {
      if (voices[v].id == (SfxID)i && voices[v].active) {
        voice = &voices[v];
        break;
      }
    }
    if (voice && triggers[i].cut) { voice->position = 0; }
    if (!voice) {
      for (v = 0; v < MAXVCS; v++) {
        if (!voices[v].active) {
          voice = &voices[v];
          voice->id = (SfxID)i;
          voice->active = 1;
          voice->position = 0;
          if (v >= voicecnt) { voicecnt = v+1; }
          break;
        }
      }
    }
    triggers[i].on = 0;
  }
  pthread_mutex_unlock(&sfxmutex);
  for (v = 0; v < voicecnt; v++) {
    voice = &voices[v];
    if (!voice->active) { continue; }
    sfx = s[voice->id];
    if (!sfx || !sfx->pcm) { voice->active = 0; continue; }
    mixable = (long)nframes;
    remaining = (long)(sfx->frames - voice->position);
    if (mixable > remaining) { mixable = remaining; }
    for (f = 0; (long)f < mixable; f++) {
      for (ch = 0; ch < CHANNELS; ch++) {
        mixi = (size_t)f * CHANNELS + (size_t)ch;
        srci = (voice->position + f) * CHANNELS + (size_t)ch;
        mixed = out[mixi] + sfx->pcm[srci];
        if (mixed > 32767) { mixed = 32767; }
        if (mixed < -32768) { mixed = -32768; }
        out[mixi] = (short)mixed;
      }
    }
    voice->position += (size_t)mixable;
    if (voice->position >= sfx->frames) { voice->active = 0; }
  }
}

static OSStatus
rendercb(void *ref, AudioUnitRenderActionFlags *flags,
    const AudioTimeStamp *ts, UInt32 bus, UInt32 nframes,
    AudioBufferList *bufs) {
  mixaudio(bufs->mBuffers[0].mData, nframes);
  return noErr;
}

void
sfxinit() {
  AudioComponent comp;
  AudioComponentDescription desc = {
    .componentType = kAudioUnitType_Output,
    .componentSubType = kAudioUnitSubType_DefaultOutput,
    .componentManufacturer = kAudioUnitManufacturer_Apple,
  };
  comp = AudioComponentFindNext(NULL, &desc);
  AudioComponentInstanceNew(comp, &outputunit);
  AudioStreamBasicDescription fmt = {
    .mSampleRate = 44100,
    .mFormatID = kAudioFormatLinearPCM,
    .mBitsPerChannel = 16,
    .mChannelsPerFrame = 2,
    .mFramesPerPacket = 1,
    .mBytesPerFrame = sizeof(int16_t) * 2,
    .mBytesPerPacket = sizeof(int16_t) * 2,
  };
  AudioUnitSetProperty(outputunit,
      kAudioUnitProperty_StreamFormat,
      kAudioUnitScope_Input, 0,
      &fmt, sizeof(fmt));
  AURenderCallbackStruct cb = { rendercb, NULL };
  AudioUnitSetProperty(outputunit,
      kAudioUnitProperty_SetRenderCallback,
      kAudioUnitScope_Input, 0,
      &cb, sizeof(cb));
  AudioUnitInitialize(outputunit);
  AudioOutputUnitStart(outputunit);
}

void
triggersfx(SfxID id, int cut) {
  pthread_mutex_lock(&sfxmutex);
  if (!triggers[id].on) { triggers[id].on = 1; triggers[id].cut = cut; }
  pthread_mutex_unlock(&sfxmutex);
}

void
sfxkill() {
  AudioOutputUnitStop(outputunit);
  AudioUnitUninitialize(outputunit);
  AudioComponentInstanceDispose(outputunit);
}

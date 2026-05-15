#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

#include "main.h"
#include "wrl.h"
#include "gfxrl.h"
#include "sfx.h"

int WWIDTH, WHEIGHT;
struct timespec thene, thenr, nowe, nowr;
long long elapsede, elapsedr;

void
flipbfrs() { 
  SwapScreenBuffer();
}

void
winit() {
  WWIDTH = 800; WHEIGHT = 600;
  SetConfigFlags(FLAG_WINDOW_RESIZEABLE);
  InitWindow(WWIDTH, WHEIGHT, "scrap");
  SetExitKey(0);
  ginit();
}

void
winloop() {
  int quit;
  quit = 0;
  GETNS(thene); GETNS(thenr);
  while (!quit && !WindowShouldClose()) {
    PollWindowInputs();
    if (IsWindowResized()) {
      WWIDTH = GetScreenWidth();
      WHEIGHT = GetScreenHeight();
      resizerl();
    }
    if (IsKeyPressed(KEY_F)) { doprofile = !doprofile; }
    if (IsKeyPressed(KEY_W)) { wiremesh = !wiremesh; }
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_PAUSE)) { pausesim = !pausesim; }
    if (IsKeyPressed(KEY_Q) || IsKeypressed(KEY_ESCAPE)) { quit = 1; }
    GETNS(nowe);
    elapsede = DIFFNS(thene, nowe);
    if (elapsede > EVTICKNS) { GETNS(thene); }
    GETNS(nowr);
    elapsedr = DIFFNS(thenr, nowr);
    if (elapsedr > GFXTICKNS) {
      GETNS(thenr);
      render();
      flipbfrs();
    }
  }
}

void
wkill() {
  gkill();
  CloseWindow();
}

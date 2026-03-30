#ifndef WQUARTZ_H
#define WQUARTZ_H

#include <X11/Xlib.h>
#include <time.h>

extern Display *display;
extern Window window;
extern struct timespec thene, thenr, nowe, nowr, frmst, frmend;
extern int WWIDTH, WHEIGHT;
extern long long elapsede, elapsedr;

void flipbfrs();
void winit();
void winloop();
void wkill();

#endif /* WQUARTZ_H */

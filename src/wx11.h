#ifndef WX11_H
#define WX11_H

#include <X11/Xlib.h>
extern Display *display;
extern Window window;

#include <time.h>
extern struct timespec thene, thenr, nowe, nowr, frmst, frmend;

extern int WWIDTH, WHEIGHT;
extern long long elapsede, elapsedr;

void winit();
void winloop();
void wkill();

#endif /* WX11_H */

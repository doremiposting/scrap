#ifndef WCOCOA_H
#define WCOCOA_H

#include <time.h>
extern struct timespec thene, thenr, nowe, nowr, frmst, frmend;

extern int WWIDTH, WHEIGHT;
extern long long elapsede, elapsedr;

void flipbfrs();
void winit();
void winloop();
void wkill();

#endif /* WCOCOA_H */

#ifndef EVENT_H
#define EVENT_H

typedef enum {
  W_HELD,
  A_HELD,
  S_HELD,
  D_HELD,
  END_OF_EVENT_ENUM
} Scrapevent;

extern char *evtable;

void buildevtbl();
void killevtbl();
void setevent(Scrapevent e, char v);
void modshfctr(float dxs, float dym, float dzs);
void handlephysics(double dt);
void handleglobalevents(double dt);

#endif /* EVENT_H */

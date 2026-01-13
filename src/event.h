#ifndef EVENT_H
#define EVENT_H

#include "main.h"

#define NUMEVS 2

typedef enum {
  SOMEEV,
  NOTANEV
} Event;

#define BUFFERSZ 8

typedef struct {
  Event q[BUFFERSZ];
  int head;
  int tail;
  size_t count;
} Evbuffer;

void initbuffer(Evbuffer *b);
int buffull(Evbuffer *b);
int bufempty(Evbuffer *b);
int bufqueue(Evbuffer *b, Event e);
int bufdeq(Evbuffer *b, Event *e);

typedef void (*evhandler)(void);
void execev(Event e);

#endif /* EVENT_H */
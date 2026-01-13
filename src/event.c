#include <stdlib.h>

#include "event.h"

void
initbuffer(Evbuffer *b) {
  b->head = 0;
  b->tail = 0;
  b->count = 0;
}

int
buffull(Evbuffer *b) { return (b->count == BUFFERSZ ? 1 : 0); }

int
bufempty(Evbuffer *b) { return (b->count == 0 ? 1 : 0); }

int
bufqueue(Evbuffer *b, Event e) {
  if (buffull(b)) { return 0; }
  b->q[b->head] = e;
  b->head = (b->head + 1) % BUFFERSZ;
  b->count++;
  return 1;
}

int
bufdeq(Evbuffer *b, Event *e) {
  if (bufempty(b)) { return 0; }
  *e = b->q[b->tail];
  b->tail = (b->tail + 1) % BUFFERSZ;
  b->count--;
  return 1;
}

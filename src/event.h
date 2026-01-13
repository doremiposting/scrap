#ifndef EVENT_H
#define EVENT_H

#include "main.h"

typedef enum {
  SOMEEV,
  NOTANEV
} Event;

typedef struct {
  Event *q;
  size_t length;
  size_t capacity;
} Evqueue;

#define addevent(xs, x) da_append(xs, x)

int handlenext(Evqueue *q);

#endif /* EVENT_H */
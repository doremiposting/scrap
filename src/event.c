#include <stdlib.h>

#include "event.h"

/* #define addevent(xs, x) da_append(xs, x) */

int
handlenext(Evqueue *q) {
  /* TODO: Events don't just need to be popped off the top of the queue, */
  /* They need to be "dispatched" back to the gfx handler to do the thing. */
  return 0;
}
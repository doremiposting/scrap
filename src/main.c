#include <stdio.h>

#ifdef __APPLE__
#include "wcocoa.h"
#else
#include "wx11.h"
#include "gfxgl.h"
#include "sfxpa.h"
#endif /* */

#include "vm.h"

int
main(int argc, char *argv[]) {
  if (argc > 1) { printf("%s\n", argv[1]); }
  vmtest();
  /* TODO: sfx initialization and destruction should be handled in main.c and not gfx*.c */
  winit();
  winloop();
  wkill();
  return 0;
}

#include <stdio.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#if defined(__APPLE__)
  #define CC "/opt/homebrew/opt/llvm@20/Toolchains/LLVM20.1.8.xctoolchain/usr/bin/clang"
  #define forceinline inline __attribute__((__always_inline__))
/*
#elif defined(_MSC_VER)
  #define CC "msvc"
  #define forceinline __forceinline
*/
#elif defined(__GNUC__)
  #define CC "gcc"
  #define forceinline inline __attribute__((__always_inline__))
#endif

Cmd cmd = {0};

static forceinline
int
boilerplate() {
	cmd_append(&cmd, CC, "-Wall", "-Wextra", "-Wformat", "-Wformat=2",
		"-Wconversion", "-Wsign-conversion", "-Werror=format-security",
		"-Wimplicit-fallthrough", "-Werror=implicit",
		"-Werror=incompatible-pointer-types", "-Werror=int-conversion",
    #ifndef __APPLE__
		"-Wtrampolines", "-fzero-init-padding-bits=all", "-Wbidi-chars=any",
		#endif
	);
}

static forceinline
int
compilefile(const char *fn, const char *out) {
  cmd_append(&cmd,
		"-std=c2y", "-g", "-c", "-o", out, fn
  );
	if (!cmd_run(&cmd)) { return 1; }
}

int
main(int argc, char *argv[]) {
	GO_REBUILD_URSELF(argc, argv); 

  boilerplate();
  cmd_append(&cmd, "-lm");
  compilefile("src/gfx.c", "build/gfx.o");

  boilerplate();
  #ifdef __APPLE__
  cmd_append(&cmd, "-I/opt/X11/include");
  #else
  cmd_append(&cmd, "-lX11");
  #endif
  compilefile("src/gfx11.c", "build/gfx11.o");

  boilerplate();
  compilefile("src/game.c", "build/game.o");
  boilerplate();
  compilefile("src/main.c", "build/main.o");
	cmd_append(&cmd, CC, "-g", "-fPIE", "-pie", "-o", "scrap",
    "-lX11", "-L/opt/X11/lib/",
    "build/main.o", "build/gfx11.o", "build/game.o", "build/gfx.o");
	if (!cmd_run(&cmd)) { return 1; }
}

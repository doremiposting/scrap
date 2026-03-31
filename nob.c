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
#elif defined(__linux__)
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
    #if defined(__linux__)
		"-Wtrampolines", "-fzero-init-padding-bits=all", "-Wbidi-chars=any",
    "-D_POSIX_C_SOURCE=200809L", "-D_GNU_SOURCE"
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

  if (!mkdir_if_not_exists("./build/")) { return 1; }

  boilerplate();
  cmd_append(&cmd, "-lm");
  compilefile("src/gfx.c", "build/gfx.o");

  #ifdef __linux__
  boilerplate();
  cmd_append(&cmd, "-lX11");
  cmd_append(&cmd, "-lGLX");
  cmd_append(&cmd, "-D_GNU_SOURCE");
  compilefile("src/wx11.c", "build/wx11.o");
  #else
  boilerplate();
  /* TODO: Go back to using cocoa... someday...
  cmd_append(&cmd, "-framework", "Cocoa");
  cmd_append(&cmd, "-framework", "OpenGL");
  compilefile("src/wcocoa.c", "build/wcocoa.o");
  */
  cmd_append(&cmd, "-I/opt/X11/include");
  cmd_append(&cmd, "-L/opt/X11/lib");
  cmd_append(&cmd, "-lX11");
  compilefile("src/wquartz.c", "build/wquartz.o");
  #endif

  boilerplate();
  #ifdef __linux__
  cmd_append(&cmd, "-lX11");
  cmd_append(&cmd, "-lGLX");
  cmd_append(&cmd, "-D_GNU_SOURCE");
  #elif defined(__APPLE__)
  cmd_append(&cmd, "-I/opt/X11/include");
  cmd_append(&cmd, "-L/opt/X11/lib");
  cmd_append(&cmd, "-lX11");
  #endif 
  compilefile("src/gfxsw.c", "build/gfxsw.o");
  
  boilerplate();
  #ifdef __APPLE__
  cmd_append(&cmd, "-I/opt/X11/include");
  cmd_append(&cmd, "-DGL_SILENCE_DEPRECATION");
  #elifdef __linux__
  cmd_append(&cmd, "-lGL");
  cmd_append(&cmd, "-D_GNU_SOURCE");
  cmd_append(&cmd, "-lGLX");
  #endif
  compilefile("src/gfxgl.c", "build/gfxgl.o");

  boilerplate();
  #ifdef __APPLE__
  cmd_append(&cmd, "-I/opt/X11/include");
  cmd_append(&cmd, "-DGL_SILENCE_DEPRECATION");
  cmd_append(&cmd, "-framework", "OpenGL");
  #elif defined(__linux__)
  cmd_append(&cmd, "-lX11");
  cmd_append(&cmd, "-D_GNU_SOURCE");
  cmd_append(&cmd, "-lGL");
  #endif
  compilefile("src/gfxobj.c", "build/gfxobj.o");

  boilerplate();
  compilefile("src/game.c", "build/game.o");
  boilerplate();
  compilefile("src/main.c", "build/main.o");
  boilerplate();
  compilefile("src/event.c", "build/event.o");
#if defined(__linux__) 
  boilerplate();
  cmd_append(&cmd, "-lpthread");
  cmd_append(&cmd, "-D_GNU_SOURCE");
  compilefile("src/sfxalsa.c", "build/sfxalsa.o");
#elif defined(__APPLE__)
  boilerplate();
  cmd_append(&cmd, "-framework", "AudioToolbox", "-framework", "AudioUnit",
      "-framework", "CoreAudio");
  compilefile("src/sfxcore.c", "build/sfxcore.o");
#endif

  boilerplate();
  compilefile("src/vm.c", "build/vm.o");

#if defined(__linux__)
	cmd_append(&cmd, CC, "-g", "-fPIE", "-pie", "-o", "scrap",
    "-lX11", "-L/opt/X11/lib/", "-lm", "-lGL", "-lGLX", "-lasound", "-lmpg123", "-lpthread",
    "build/main.o", "build/gfxgl.o", "build/game.o", "build/gfx.o", "build/event.o", "build/sfxalsa.o",
    "build/gfxobj.o", "build/wx11.o", "build/vm.o");
	if (!cmd_run(&cmd)) { return 1; }
#elif defined(__APPLE__)
  /* TODO: Fix wcocoa & opengl on mac
	cmd_append(&cmd, CC, "-g", "-fPIE", "-pie", "-o", "scrap-cocoa",
    "-lX11", "-L/opt/X11/lib/", "-lm", "-lGL", "-framework", "OpenGL", "-framework", "Cocoa", "-lpthread",
    "-framework", "AudioToolbox", "-framework", "AudioUnit", "-framework", "CoreAudio",
    "build/main.o", "build/gfxgl.o", "build/game.o", "build/gfx.o", "build/event.o", "build/sfxcore.o",
    "build/gfxobj.o", "build/wcocoa.o");
  */
	cmd_append(&cmd, CC, "-g", "-fPIE", "-pie", "-o", "scrap-cocoa",
    "-L/opt/X11/lib", "-lX11", "-lm",
    "-framework", "AudioToolbox", "-framework", "AudioUnit", "-framework", "CoreAudio",
    "build/main.o", "build/game.o", "build/gfx.o", "build/event.o", "build/sfxcore.o",
    "build/gfxobj.o", "build/wquartz.o", "build/gfxsw.o", "build/vm.o");
	if (!cmd_run(&cmd)) { return 1; }
#endif
}

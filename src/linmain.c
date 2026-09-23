#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <time.h>

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glext.h>

#include "gfxgl.h"
#include "event.h"
#include "gpplayer.h"

#define UNUSED(x) (void)(x)
#define GETNS(ts) (clock_gettime(CLOCK_MONOTONIC, &ts))
#define DIFFNS(start, end) \
      ((int64_t)((end).tv_sec - (start).tv_sec) * 1000000000LL + \
      ((end).tv_nsec - (start).tv_nsec))

int screen;
XWindowAttributes wa = {0};
XSetWindowAttributes swa = {0};
XVisualInfo *vi;
XImage *i;
Atom wmdelwin;
GC gc;
GLXContext glc;
Colormap cmap;
Display *display;
Window window;
struct timespec thene, thenr, nowe, nowr, frmst, frmend;
int WWIDTH, WHEIGHT;
struct timespec tthen, tnow;
long long dtns, physat;
int perfstat;

static int visattribs[] = {
  GLX_RGBA,
  GLX_DOUBLEBUFFER,
  GLX_DEPTH_SIZE, 24,
  GLX_RED_SIZE, 8,
  GLX_GREEN_SIZE, 8,
  GLX_BLUE_SIZE, 8,
  None
};

void
x11init() {
  WWIDTH = 800;
  WHEIGHT = 600;
  display = XOpenDisplay(NULL);
  if (!display) { fprintf(stderr, "Couldn't open display!\n"); exit(1);}
  screen = DefaultScreen(display);
  vi = glXChooseVisual(display, screen, visattribs);
  if (!vi) { fprintf(stderr, "No suitable GLX visual\n"); exit(1); }
  cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.border_pixel = 0;
  swa.event_mask  = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask |
      ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
  window = XCreateWindow(
    display, RootWindow(display, vi->screen),
    0, 0, (unsigned int)WWIDTH, (unsigned int)WHEIGHT,
    0, vi->depth, InputOutput, vi->visual,
    CWColormap | CWBorderPixel | CWEventMask, &swa
  );
  wmdelwin = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmdelwin, 1);
  XStoreName(display, window, "scrap");
  XMapWindow(display, window);
  XSync(display, 0);
  glc = glXCreateContext(display, vi, NULL, 1);
  if (!glc) { fprintf(stderr, "Cannot create ogl context!\n"); exit(1); }
  glXMakeCurrent(display, window, glc);
  fprintf(stderr, "GL Renderer: %s\n" "GL version: %s\n",
    glGetString(GL_RENDERER), glGetString(GL_VERSION));
  animate = 1;
  wiremesh = 1;
  perfstat = 1;
  glinit();
  GETNS(tthen); GETNS(tnow); GETNS(thenr); GETNS(nowr); GETNS(frmst); GETNS(frmend);
  XFree(vi);
}

void
x11kill() {
  glkill();
  glXMakeCurrent(display, None, NULL);
  glXDestroyContext(display, glc);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}

int
main(int argc, char *argv[]) {
  XEvent ev;
  int doexit;
  double dt;
  long long nowft;
  double bestfps, worstfps, nowfps;
  const long long physdt = 1000000000LL / 120;
  const float physdtf = (float)(1.0 / 120.0);
  GLuint state;
  int omx, mx, omy, my;
  int syncyaw;
  omx = mx = omy = my = 0;
  state = 0;
  doexit = 0;
  physat = 0;
  bestfps = 0.0; worstfps = DBL_MAX; nowft = 0; nowfps = 0.0;
  syncyaw = 0;
  x11init();
  buildevtbl();
  glreshape(WWIDTH, WHEIGHT);
  while (!doexit) {
    GETNS(frmst);
    while (XPending(display) > 0) {
      XNextEvent(display, &ev);
      switch (ev.type) {
        case ConfigureNotify:
          WWIDTH = ev.xconfigure.width;
          WHEIGHT = ev.xconfigure.height;
          glreshape(ev.xconfigure.width, ev.xconfigure.height);
          break;
        case KeyPress:
          switch (XLookupKeysym(&ev.xkey, 0)) {
            case 'i':
              wiremesh = !wiremesh;
              break;
            case XK_space:
              if (P->mv != PLAYER_JUMPING) {
                P->mv = PLAYER_JUMPING;
                P->dy = 20.0f;
              }
              break;
            case 'p':
              animate = !animate;
              break;
            case 'q':
            case XK_Escape:
              doexit = 1;
              break;
            case 'c':
              if (P->view->cm == CAMERA_FOLLOW) { P->view->cm = CAMERA_FREECAM; }
              else if (P->view->cm == CAMERA_FREECAM) {
                P->view->cm = CAMERA_FOLLOW;
                syncyaw = 0;
              }
              break;
            case 'w':
              setevent(W_HELD, 1);
              break;
            case 's':
              setevent(S_HELD, 1);
              break;
            case 'a':
              setevent(A_HELD, 1);
              break;
            case 'd':
              setevent(D_HELD, 1);
              break;
            }
            break;
          case KeyRelease:
            switch (XLookupKeysym(&ev.xkey, 0)) {
              case 'w':
                setevent(W_HELD, 0);
                break;
              case 's':
                setevent(S_HELD, 0);
                break;
              case 'a':
                setevent(A_HELD, 0);
                break;
              case 'd':
                setevent(D_HELD, 0);
                break;
            }
            break;
          case ButtonPress:
            mx = ev.xbutton.x;
            my = ev.xbutton.y;
            if (ev.xbutton.button == Button1) { state |= PAN; }
            if (ev.xbutton.button == Button3) { state |= ROTATE; }
            XGrabPointer(display, window, True,
              ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
              GrabModeAsync, GrabModeAsync,
              None, None, CurrentTime
            );
            break;
          case ButtonRelease:
            if (ev.xbutton.button == Button1) { state &= ~PAN; }
            if (ev.xbutton.button == Button3) { state &= ~ROTATE; }
            if (!state) { XUngrabPointer(display, CurrentTime); }
            break;
          case MotionNotify:
            omx = mx; omy = my;
            mx = ev.xmotion.x; my = ev.xmotion.y;
            if (P->view->cm == CAMERA_FREECAM) { update(state, omx, mx, omy, my); }
            else if (P->view->cm == CAMERA_FOLLOW) {
              if (state & ROTATE) {
                if (!syncyaw) {
                  P->yaw = M_PI - P->view->yaw;
                  syncyaw = 1;
                }
                P->view->yaw += (mx-omx) * 0.01f; 
                P->yaw -= (mx-omx) * 0.01f;
                P->view->pitch += (my-omy) * 0.005f;
              }
              if (state & PAN) {
                P->view->yaw += (mx-omx) * 0.01f; 
                P->view->pitch += (my-omy) * 0.005f;
                syncyaw = 0;
              }
            }
            if (P->yaw > M_PI*2) { P->yaw -= (float)(M_PI*2); }
            if (P->yaw < -M_PI*2) { P->yaw += (float)(M_PI*2); }
            if (P->view->yaw > M_PI*2) { P->view->yaw -= (float)(M_PI*2); }
            if (P->view->yaw < -M_PI*2) { P->view->yaw += (float)(M_PI*2); }
            break;
          case ClientMessage: {
            if ((Atom) ev.xclient.data.l[0] == wmdelwin) { doexit = 1; }
            break;
          default:
            /* if (ev.type == CompletionType) {} */
            break;
          }
      }
    }
    GETNS(frmend); GETNS(tnow);
    dtns = DIFFNS(tthen, tnow);
    physat += dtns;
    if (physat > physdt) {
      handlephysics(physdtf);
      physat -= physdt;
    }
    dt = (double)((tnow.tv_sec - tthen.tv_sec) + (double)(tnow.tv_nsec - tthen.tv_nsec)/1000000000.0);
    handleglobalevents(dt);
    GETNS(tthen);
    if (perfstat) { GETNS(thenr); }
    render();
    glXSwapBuffers(display, window);
    if (perfstat) {
      GETNS(nowr);
      nowft = DIFFNS(thenr, nowr); nowfps = (1000000000.0 /((double)(nowft)));
      if (bestfps < nowfps) { bestfps = nowfps; }
      if (worstfps > nowfps) { worstfps = nowfps; }
      fprintf(stderr, "\rPy: %.2f Cy: %.2f, FR: %.2f FPS (best: %.2f, worst %.2f), FT: %lld ns", P->yaw, P->view->yaw, nowfps, bestfps, worstfps, nowft);
    }
  }
  killevtbl();
  x11kill();
  fprintf(stderr, "\rDone.                                                        \n");
  fprintf(stderr, "Framerates: best: %.2f, worst: %.2f\n", bestfps, worstfps);
}

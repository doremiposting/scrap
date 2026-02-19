#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "gfx.h"
#include "gfxgl.h"
#include "gfxobj.h"
#include "sfxalsa.h"
#include "event.h"

int WWIDTH;
int WHEIGHT;

Display *display;
Window window;
XWindowAttributes wa = {0};
XSetWindowAttributes swa;
XVisualInfo *vi;
XImage *i;
GC gc;
Atom wmdelwin;
Tri2df tri;
float a;
float cx, cy, mag, rad, dx, dy;
int mmx, mmy;
double da;
GLXContext glc;
Colormap cmap;
Mesh *tp;
static float fovy;
int pausesim, wiremesh;

typedef GLXContext (*glXCreateContextAttribsARBProc)(
    Display*, GLXFBConfig, GLXContext, Bool, const int*);

#define EVTICKNS 600000000LL
#define GFXTICKNS 16666667LL
#define GETNS(ts) (clock_gettime(CLOCK_MONOTONIC, &ts))
#define DIFFNS(start, end) \
    ((int64_t)((end).tv_sec - (start).tv_sec) * 1000000000LL + \
     ((end).tv_nsec - (start).tv_nsec))

void lookat(float ex, float ey, float ez,
             float cx, float cy, float cz,
             float ux, float uy, float uz) {
  float fx = cx - ex;
  float fy = cy - ey;
  float fz = cz - ez;
  float fl = sqrtf(fx*fx + fy*fy + fz*fz);
  fx /= fl; fy /= fl; fz /= fl;

  float ul = sqrtf(ux*ux + uy*uy + uz*uz);
  ux /= ul; uy /= ul; uz /= ul;

  float sx = fy*uz - fz*uy;
  float sy = fz*ux - fx*uz;
  float sz = fx*uy - fy*ux;

  float ux2 = sy*fz - sz*fy;
  float uy2 = sz*fx - sx*fz;
  float uz2 = sx*fy - sy*fx;

  float m[16] = {
    sx,  ux2, -fx, 0,
    sy,  uy2, -fy, 0,
    sz,  uz2, -fz, 0,
    0,   0,    0,  1
  };

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMultMatrixf(m);
  glTranslatef(-ex, -ey, -ez);
}

void
drawm(const Mesh *m) {
  unsigned int i;
  wiremesh ? glPolygonMode(GL_FRONT, GL_LINE)
    : glPolygonMode(GL_FRONT, GL_FILL);
  glBegin(GL_TRIANGLES);
  for (i = 0; i < m->cnt; i++) {
    glNormal3f(m->v[i].nx, m->v[i].ny, m->v[i].nz);
    glTexCoord2f(m->v[i].u, m->v[i].v);
    glVertex3f(m->v[i].x, m->v[i].y, m->v[i].z);
  }
  glEnd();
}

void
resizegl() {
  float ar, fh, fw;
  if (WHEIGHT == 0) { WHEIGHT = 1; }
  glViewport(0, 0, WWIDTH, WHEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  ar = (float)WWIDTH/(float)WHEIGHT;
  fh = tanf(fovy * 0.5f * ((float)(M_PI) / 180.f)) * 0.1f;
  fw = fh * ar;
  glFrustum(-fw, fw, -fh, fh, 0.1f, 100.0f);

  glMatrixMode(GL_MODELVIEW);
}

void
ginit() {
  float top, bottom, right, left;
  double fovyrad;
  int fbcnt;
  GLXFBConfig *fbcs, bestfbc;
  int visattribs[] = {
    GLX_X_RENDERABLE, 1,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_BLUE_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_RED_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_DOUBLEBUFFER, 1,
    None
  };
  a = 0.0f;
  da = 60.0f; /* The sw render logic uses radians, opengl uses degrees. */
  cx = 0.0f;
  cy = 0.0f;
  dx = 0.05f;
  dy = 0.05f;
  mmx = 1;
  mmy = 1;
  rad = 0.75;
  tri.x1 = 0.0f; tri.y1 = 100.0f;
  tri.x2 = -75.0f; tri.y2 = -50.0f;
  tri.x3 = 75.0f; tri.y3 = -50.0f;
  WWIDTH = 800;
  WHEIGHT = 600;

  tp = loadobj("assets/teapot.obj");
  /* tp = loadobj("assets/teapottri.obj"); */

  display = XOpenDisplay(NULL);
  if (!display) { fprintf(stderr, "ERROR: Couldn't open display!\n"); exit(1); }
  fbcs = glXChooseFBConfig(display, DefaultScreen(display), visattribs, &fbcnt);
  fprintf(stderr, "fbcs available: %d\n", fbcnt);
  if (!fbcs || fbcnt == 0) { fprintf(stderr, "No valid fbconfig found!\n"); exit(1); }
  bestfbc = fbcs[0];
  /* GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None}; */
  /* vi = glXChooseVisual(display, DefaultScreen(display), att); */
  vi = glXGetVisualFromFBConfig(display, bestfbc);
  if (!vi) { printf("No valid visual found\n"); return; }
  else { printf("visual: %ld\n", vi->visualid); }
  cmap = XCreateColormap(display, XRootWindow(display, vi->screen), vi->visual, AllocNone);
  wmdelwin = XInternAtom(display, "WM_DELETE_WINDOW", False);
  swa.colormap = cmap; swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
  window = XCreateWindow(
    display,
    XRootWindow(display, vi->screen),
    0, 0,
    (unsigned int)WWIDTH, (unsigned int)WHEIGHT, 0,
    vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa
    );
  XGetWindowAttributes(display, window, &wa);
  XSetWMProtocols(display, window, &wmdelwin, 1);
  XSelectInput(display, window, swa.event_mask|PointerMotionMask);
  XStoreName(display, window, "Scrap");
  glc = glXCreateNewContext(display, bestfbc, GLX_RGBA_TYPE, NULL, True);
  if (!glc) {
    glc = glXCreateContext(display, vi, NULL, True);
    if (!glc) {
      fprintf(stderr, "GLX context could not be created!\n");
    }
  }
  printf("Context created: %p  Is direct? %s\n",
       glc, glXIsDirect(display, glc) ? "yes" : "no (indirect)");
  //glXWaitX();
  //XSync(display, 0);
  glXMakeCurrent(display, window, glc) ? printf("bound gl context to window\n") : printf("Could not make gl context current\n");
  glViewport(0, 0, (int)WWIDTH, (int)WHEIGHT);
  fovy = 60.0f;
  fovyrad = fovy * (M_PI / 180.0f);
  top = tanf((float)fovyrad * 0.5f) * 0.1f;
  bottom = -top;
  right = top * ((float)WWIDTH / (float)WHEIGHT);
  left = -right;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(left, right, bottom, top, 0.1f, 1000.0f); // TODO: Update frustum on window resize
  //glOrtho(0, WWIDTH, 0, WHEIGHT, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  XMapWindow(display, window);
  XFree(fbcs);
  XFree(vi);
  initsfx();
}

void
render() {
  XEvent ev;
  int quit, doprofile;
  struct timespec thene, thenr, nowe, nowr, frmst, frmend;
  long long elapsede, elapsedr;
  quit = 0;
  GETNS(thene); GETNS(thenr);
  GETNS(frmst); GETNS(frmend);
  pausesim = 0; wiremesh = 0; doprofile = 1;
  while (!quit) {
    /* TODO: Somehow we need to translate engine inputs, handled immediately, */
    /* into game inputs, handled on a per-tick basis. */
    while (XPending(display) > 0) {
      XNextEvent(display, &ev);
      switch (ev.type) {
        case ConfigureNotify:
          WWIDTH = ev.xconfigure.width;
          WHEIGHT = ev.xconfigure.height;
          resizegl();
          break;
        case KeyPress:
        switch (XLookupKeysym(&ev.xkey, 0)) {
          case 'f':
            doprofile = !doprofile;
            break;
          case 'w':
            wiremesh = !wiremesh;
            break;
          case 'q':
          case XK_Escape:
            quit = 1;
            break;
          case 'p':
          case XK_Pause:
            pausesim = !pausesim;
            break;
          default:
            break;
        }
        break;
        case MotionNotify: { /* event.xmotion.x, event.xmotion.y */ }
        break;
        case ClientMessage: {
          if ((Atom) ev.xclient.data.l[0] == wmdelwin) { quit = 1; }
        }
        break;
        default:
          /* if (ev.type == CompletionType) { } */
        break;

      }
    }
    GETNS(nowe);
    elapsede = DIFFNS(thene, nowe);
    if (elapsede > EVTICKNS) { GETNS(thene); }

    GETNS(nowr);
    elapsedr = DIFFNS(thenr, nowr);
    if (elapsedr > GFXTICKNS) {
      if (doprofile) { GETNS(frmst); }
      thenr = nowr;
			glClearColor(0.39f, 0.58f, 0.92f, 1.0f);
			/* glClearColor(0.0f, 0.0f, 0.0f, 1.0f); */
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
			lookat(3.0, 3.0, 3.0, 0.0, 0.0, -4.5, 0, 1, 0);
			glPushMatrix();
      glTranslatef(0.0f, 0.0f, -4.5f); 
      glBegin(GL_LINES);
				glColor3f(1,0,0);
				glVertex3f(0,0,-0.001f);
				glVertex3f(10,0,-0.002f);
				glColor3f(0,1,0);
				glVertex3f(0,0,-0.001f);
				glVertex3f(0,10,-0.002f);
				glColor3f(0,0,1);
				glVertex3f(0,0,-0.001f);
				glVertex3f(0,0,10);
      glEnd();
			glPopMatrix();
      glPushMatrix();
      glTranslatef(cx, cy, -5.0f); 
      glRotatef(a, 0.0f, 1.0f, 0.0f);
      drawm(tp);
      glPopMatrix();
			glXSwapBuffers(display, window);
      XSync(display, 0);
      glFlush();
      if (!pausesim) {
        a += 3.0f;
        cx += (dx*(float)mmx);
        if (cx > 3.8f-rad) { cx = 3.8f-rad; mmx *= -1; triggersfx(SFX_BOOM, 1); } if (cx < -3.8f-rad) { cx = -3.8f-rad; mmx *= -1; triggersfx(SFX_BOOM, 1); }
        cy += (dx*(float)mmy);
        if (cy > 2.8f-rad) { cy = 2.8f-rad; mmy *= -1; triggersfx(SFX_BOOM, 1); } if (cy < -2.8f-rad) { cy = -2.8f - rad; mmy *= -1; triggersfx(SFX_BOOM, 1); }
      }
      if (doprofile) {
        GETNS(frmend);
        fprintf(stderr, "\rFPS: %.2f, FT: %lld us", (1000000000.0 /((double)(elapsedr))), DIFFNS(frmst, frmend)/1000);
        fflush(stderr);
      }
    }
  }
}

void
gkill() {
  fprintf(stderr, "\rDone.                    \n");
  /* TODO: free() roundup from ginit(). */
	glXMakeCurrent(display, None, NULL);
	glXDestroyContext(display, glc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
  killsfx();
  killmesh(tp);
}

void foo() { printf("hi!\n"); }

static const evhandler evdispatch[NUMEVS] = {
  [SOMEEV] = foo,
  [NOTANEV] = NULL
};

/* TODO: Make an extra file with an extra dispatcher using */
/* __attribute__(weak) flags as a fallback if one doesn't exist/fails */
void
execev(Event e) {
  if (e < NUMEVS && evdispatch[e]) {
    evdispatch[e]();
  }
}

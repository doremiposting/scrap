#include <windows.h>
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif
#include <profileapi.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "gfxgl.h"
#include "gpplayer.h"
#include "gpchar.h"
#include "event.h"

HPALETTE hpalette;
/* XXX: Don't forget that these are a struct and not integers!
 * FUCK YOU Bill Gates! */
LARGE_INTEGER now, thenf, freq;
double dt, physat;
int perfstat;

LONG WINAPI WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
HWND createoglwin(char *title, int x, int y, int width, int height, BYTE type, DWORD flags);

int WINAPI
WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR pcmdline, int ncmdshow) {
  HDC hdc;
  HGLRC hrc;
  HWND hwnd;
  MSG msg;
  RECT r;
  int doquit;
  LARGE_INTEGER thenr, nowr;
  long long nowft;
  double bestfps, worstfps, nowfps;
  const float physdt = 1.0 / 120.0;
  (void)hinstance; (void)hprevinstance; (void)pcmdline;

  hwnd = createoglwin("scrap", 0, 0, 800, 600, PFD_TYPE_RGBA, 0);
  if (hwnd == NULL) { exit(1); }
  hdc = GetDC(hwnd);
  hrc = wglCreateContext(hdc);
  wglMakeCurrent(hdc, hrc);
  glinit();
  buildevtbl();
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&now);
  QueryPerformanceCounter(&thenf);
  GetClientRect(hwnd, &r);
  glreshape(r.right, r.bottom);
  ShowWindow(hwnd, ncmdshow);
  physat = 0;
  doquit = 0;
  bestfps = 0.0; worstfps = DBL_MAX; nowft = 0; nowfps = 0.0;
  while (!doquit) {
    /* XXX: Use NULL instead of hwnd (for some reason) if you want events to resolve. */
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) { doquit = 1; break; }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (!doquit) {
      QueryPerformanceCounter(&now);
      dt = (double)(now.QuadPart - thenf.QuadPart) / (double)freq.QuadPart;
      QueryPerformanceCounter(&thenf);
      physat += dt;
      if (physat > physdt) {
        handlephysics(physdt);
        physat -= physdt;
      }
      handleglobalevents(dt);
      if (perfstat) { QueryPerformanceCOunter(&thenr); }
      render();
      SwapBUffers(wglGetCurrentDC());
      if (perfstat) {
        QueryPerformanceCounter(&nowr);
        nowft = (long long)((nowr.QuadPart - thenr.QuadPart) * 1000000000LL / freq.QuadPart);
        nowfps = 1000000000.0 / (double)nowft;
        if (bestfps < nowfps) { bestfps = nowfps; }
        if (worstfps > nowfps) { worstfps = nowfps; }
        fprintf(stderr, "\rFR: %.2f FPS (best: %.2f, worst %.2f), FT: %lld ns",
          nowfps, bestfps, worstfps, nowft);
      }
    }
  }
  killevtbl();
  glkill();
  wglMakeCurrent(NULL, NULL);
  ReleaseDC(hwnd, hdc);
  wglDeleteContext(hrc);
  DestroyWindow(hwnd);
  if (hpalette) { DeleteObject(hpalette); }
  fprintf(stderr, "\rDone.                                                        \n");
  fprintf(stderr, "Framerates: best: %.2f, worst: %.2f\n", bestfps, worstfps);
  return msg.wParam;
}

LONG WINAPI
WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
  PAINTSTRUCT ps;
  static int mbl = 0;
  static int mbr = 0;
  static GLuint state = 0;
  static int syncyaw = 0;
  static int omx, omy, mx, my;
  switch (umsg) {
    case WM_KEYDOWN:
      switch (wparam) {
        case VK_ESCAPE: /* ESC */
        case 'Q':
          PostQuitMessage(0);
          break;
        case VK_SPACE:
          if (P->mv != PLAYER_JUMPING) {
            P->mv = PLAYER_JUMPING;
            P->dy = 20.0f;
          }
        case 'P':
          animate = !animate;
          break;
        case 'V':
          showcoll = !showcoll;
          break;
        case 'W':
          setevent(W_HELD, 1);
          break;
        case 'S':
          setevent(S_HELD, 1);
          break;
        case 'A':
          setevent(A_HELD, 1);
          break;
        case 'D':
          setevent(D_HELD, 1);
          break;
        case 'I':
          wiremesh = !wiremesh;
          break;
        case 'L':
          camlookat(P->view, P->posx, P->posy, P->posz);
          fprintf(stderr, "Yaw: %f Pitch: %f\n", P->view->yaw, P->view->pitch);
          break;
        case 'C':
          if (P->view->cm == CAMERA_FOLLOW) { P->view->cm = CAMERA_FREECAM; }
          else if (P->view->cm == CAMERA_FREECAM) {
            P->view->cm = CAMERA_FOLLOW;
            syncyaw = 0;
          }
          break;
      }
      return 0;
      break;
    case WM_KEYUP:
      switch (wparam) {
        case 'W':
          setevent(W_HELD, 0);
          break;
        case 'S':
          setevent(S_HELD, 0);
          break;
        case 'A':
          setevent(A_HELD, 0);
          break;
        case 'D':
          setevent(D_HELD, 0);
          break;
      }
      return 0;
      break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
      SetCapture(hwnd);
      mx = GET_X_LPARAM(lparam);
      my = GET_Y_LPARAM(lparam);
      if (umsg == WM_LBUTTONDOWN) { state |= PAN; }
      if (umsg == WM_RBUTTONDOWN) { state |= ROTATE; }
      return 0;
      break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      ReleaseCapture();
      state = 0;
      return 0;
      break;
    case WM_MOUSEMOVE:
      if (state) {
        omx = mx; omy = my;
        mx = GET_X_LPARAM(lparam); my = GET_Y_LPARAM(lparam);
        if (P->view->cm == CAMERA_FREECAM) {
          update((int)state, omx, mx, omy, my);
        } else if (P->view->cm == CAMERA_FOLLOW) {
          if (state & ROTATE) {
            if (!syncyaw) {
              P->yaw = (float)M_PI - P->view->yaw;
              syncyaw = 1;
            }
            P->view->yaw += (float)(mx - omx) * 0.01f;
            P->yaw -= (float)(mx - omx) * 0.01f;
            P->view->pitch += (float)(my - omy) * 0.005f;
          }
          if (state & PAN) {
            P->view->yaw += (float)(mx - omx) * 0.01f;
            P->view->pitch += (float)(my - omy) * 0.01f;
            syncyaw = 0;
          }
        }
        if (P->yaw > M_PI*2) { P->yaw -= (float)(M_PI*2); }
        if (P->yaw < -M_PI*2) { P->yaw += (float)(M_PI*2); }
        if (P->view->yaw > M_PI*2) { P->view->yaw -= (float)(M_PI*2); }
        if (P->view->yaw < -M_PI*2) { P->view->yaw += (float)(M_PI*2); }
      }
      return 0;
      break;
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
      break;
    case WM_SIZE:
      glreshape(LOWORD(lparam), HIWORD(lparam));
      PostMessage(hwnd, WM_PAINT, 0, 0);
      return 0;
      break;
    case WM_PAINT:
      render();
      BeginPaint(hwnd, &ps);
      EndPaint(hwnd, &ps);
      return 0;
      break;
  }
  return DefWindowProc(hwnd, umsg, wparam, lparam);
}

HWND
createoglwin(char *title, int x, int y, int width, int height, BYTE type, DWORD flags) {
  int pf;
  HDC hdc;
  HWND hwnd;
  WNDCLASS wc;
  PIXELFORMATDESCRIPTOR pfd;
  static HINSTANCE hinstance = 0;
  animate = 1;
  wiremesh = 0;
  showcoll = 0;
  perfstat = 1;

  /* Only refister once, using hinstance as a flag */
  if (!hinstance) {
    hinstance = GetModuleHandle(NULL);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)WindowProc;
    wc.cbClsExtra = 0;
    wc.hInstance = hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "scrap";
    if (!RegisterClass(&wc)) {
      MessageBox(NULL, "RegisterClass() failed to register window class.", "Error", MB_OK);
      return NULL;
    }
  }
  hwnd = CreateWindow("scrap", title, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                        x, y, width, height, NULL, NULL, hinstance, NULL);
  if (hwnd == NULL) {
      MessageBox(NULL, "CreateWindow() failed.", "Error", MB_OK);
      return NULL;
  }
  hdc = GetDC(hwnd);
  memset(&pfd, 0, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | flags;
  pfd.iPixelType = type;
  pfd.cColorBits = 32;
  pf = ChoosePixelFormat(hdc, &pfd);
  if (pf == 0) {
      MessageBox(NULL, "ChoosePixelFormat() failed to find suitable pixel format.", "Error", MB_OK);
      return 0;
  }
  if (!SetPixelFormat(hdc, pf, &pfd)) {
      MessageBox(NULL, "SetPixelFormat() failed to set selected format", "Error", MB_OK);
      return 0;
  }
  DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
  ReleaseDC(hwnd, hdc);
  return hwnd;
}

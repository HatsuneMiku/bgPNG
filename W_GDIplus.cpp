/*
  W_GDIplus.cpp

  rc W_GDIplus.rc
  cl W_GDIplus.cpp W_GDIplus.res kernel32.lib user32.lib ole32.lib gdi32.lib \
    gdiplus.lib dwmapi.lib -DUNICODE
*/

#include <windows.h>
#include <gdiplus.h>
#include <dwmapi.h>

#define CLASS_NAME L"W_GDIPLUS"
#define APP_NAME L"W_GDIPLUS"
#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 640

Gdiplus::Image *LoadFromResource(HINSTANCE hinst, LPCTSTR name, LPCTSTR typ)
{
  HRSRC hres = FindResource(hinst, name, typ);
  if(!hres) return NULL;
  DWORD siz = SizeofResource(hinst, hres);
  if(!siz) return NULL;
  HGLOBAL hdata = LoadResource(hinst, hres);
  if(!hdata) return NULL;
  const void *pdata = LockResource(hdata);
  if(!pdata) return NULL;
  HGLOBAL hbuf = GlobalAlloc(GMEM_MOVEABLE, siz);
  if(hbuf){
    void *pbuf = GlobalLock(hbuf);
    if(pbuf){
      CopyMemory(pbuf, pdata, siz);
      IStream *s = NULL;
      if(CreateStreamOnHGlobal(hbuf, FALSE, &s) == S_OK){
        Gdiplus::Image *img = Gdiplus::Image::FromStream(s);
        s->Release();
        if(img){
          if(img->GetLastStatus() == Gdiplus::Ok){
            GlobalUnlock(hbuf); // Is it ok to unlock handle here ?
            GlobalFree(hbuf); // Is it ok to free handle here ?
            return img;
          }
          delete img;
        }
      }
      GlobalUnlock(hbuf);
    }
    GlobalFree(hbuf);
    hbuf = NULL;
  }
  return NULL;
}

BOOL SetThrough(HWND hwnd, int through, int l, int r, int t, int b)
{
  BOOL en;
  if(DwmIsCompositionEnabled(&en) == S_OK)
    if(en){
      MARGINS margins = {l, r, t, b};
      if(!through) margins = MARGINS();
      return DwmExtendFrameIntoClientArea(hwnd, &margins) == S_OK;
    }
  return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static int through = 0;
  static Gdiplus::Image *img = NULL;
  // HINSTANCE hinst = ((LPCREATESTRUCT)lparam)->hInstance; // WM_CREATE only
  // HINSTANCE hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
  HINSTANCE hinst = GetModuleHandle(NULL);
  switch(msg){
  case WM_CREATE:
    SetThrough(hwnd, 1, -1, -1, -1, -1);
    img = LoadFromResource(hinst, L"png01", L"IMAGE");
    if(!img) MessageBox(hwnd, L"cannot load IMAGE", APP_NAME,
      MB_OK | MB_ICONEXCLAMATION);
    return FALSE;
  case WM_DWMCOMPOSITIONCHANGED: // Aero enabled or disabled
    SetThrough(hwnd, 1, -1, -1, -1, -1);
    return FALSE;
  case WM_LBUTTONDOWN:
    SetThrough(hwnd, through = 1 - through, 40, 40, 80, 80);
    return FALSE;
  case WM_RBUTTONDOWN:
    SetThrough(hwnd, 1, -1, -1, -1, -1);
    return FALSE;
  case WM_PAINT:{
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    RECT r;
    GetClientRect(hwnd, &r);
    Gdiplus::Graphics g(ps.hdc);
    g.Clear(Gdiplus::Color(128, 224, 224, 224)); // ARGB
    int x = (r.right - r.left - img->GetWidth()) / 2;
    int y = (r.bottom - r.top - img->GetHeight()) / 2;
    g.DrawImage(img, x, y, img->GetWidth(), img->GetHeight());
    EndPaint(hwnd, &ps);
    } return FALSE;
  case WM_DESTROY:
    if(img){ delete img; img = NULL; }
    PostQuitMessage(0);
    return FALSE;
  }
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

BOOL RegCls(HINSTANCE hinst)
{
  WNDCLASSEX wc;
  memset(&wc, 0, sizeof(wc));
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hinst;
  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
  wc.hIconSm = wc.hIcon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = CLASS_NAME;
  return RegisterClassEx(&wc);
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR cmd, int ncmd)
{
  MSG msg;
  if(!RegCls(hinst)) return 1;
  // GDI+ must be initialized before create window (use GDI+ at WM_CREATE)
  ULONG_PTR token;
  Gdiplus::GdiplusStartupInput gsi;
  if(Gdiplus::GdiplusStartup(&token, &gsi, NULL) != Gdiplus::Ok) return 1;
  int x = (GetSystemMetrics(SM_CXSCREEN) - WINDOW_WIDTH) / 2;
  int y = (GetSystemMetrics(SM_CYSCREEN) - WINDOW_HEIGHT) / 2;
  HWND hwnd = CreateWindow(CLASS_NAME, APP_NAME,
    // WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    WS_POPUP | WS_VISIBLE,
    x, y, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hinst, NULL);
  if(hwnd){
    ShowWindow(hwnd, ncmd);
    UpdateWindow(hwnd);
    while(GetMessage(&msg, NULL, 0, 0) > 0){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  Gdiplus::GdiplusShutdown(token);
  return msg.wParam;
}

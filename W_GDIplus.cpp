/*
  W_GDIplus.cpp

  rc W_GDIplus.rc
  cl W_GDIplus.cpp W_GDIplus.res kernel32.lib user32.lib ole32.lib gdi32.lib \
    gdiplus.lib -DUNICODE
*/

#include <windows.h>
#include <gdiplus.h>

#define CLASS_NAME L"W_GDIPLUS"
#define APP_NAME L"W_GDIPLUS"
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 768

Gdiplus::Bitmap *LoadImageFromResource(HINSTANCE hinst,
  LPCTSTR name, LPCTSTR typ)
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
MessageBox(NULL, L"stream", APP_NAME, MB_OK);
        Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromStream(s);
        s->Release();
if(!bitmap) MessageBox(NULL, L"bitmap==NULL", APP_NAME, MB_OK);
        if(bitmap){
          if(bitmap->GetLastStatus() == Gdiplus::Ok){
            GlobalUnlock(hbuf);
            GlobalFree(hbuf);
            return bitmap;
          }
          delete bitmap;
        }
      }
      GlobalUnlock(hbuf);
    }
    GlobalFree(hbuf);
    hbuf = NULL;
  }
  return NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static Gdiplus::Bitmap *bitmap = NULL;
  // HINSTANCE hinst = ((LPCREATESTRUCT)lparam)->hInstance; // WM_CREATE only
  HINSTANCE hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
  switch(msg){
  case WM_CREATE:
    bitmap = LoadImageFromResource(hinst, L"png01", L"IMAGE");
    if(!bitmap) MessageBox(hwnd, L"cannot load resource", APP_NAME,
      MB_OK | MB_ICONEXCLAMATION);
    return FALSE;
  case WM_LBUTTONDOWN:
    MessageBox(hwnd, L"WM_LBUTTON", APP_NAME, MB_OK | MB_ICONINFORMATION);
    return FALSE;
  case WM_DESTROY:
    if(bitmap){ delete bitmap; bitmap = NULL; }
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
  if(!RegCls(hinst)) return 1;
  int x = (GetSystemMetrics(SM_CXSCREEN) - WINDOW_WIDTH) / 2;
  int y = (GetSystemMetrics(SM_CYSCREEN) - WINDOW_HEIGHT) / 2;
  HWND hwnd = CreateWindow(CLASS_NAME, APP_NAME,
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    x, y, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hinst, NULL);
  if(!hwnd) return 1;
  ShowWindow(hwnd, ncmd);
  UpdateWindow(hwnd);
  ULONG_PTR token;
  Gdiplus::GdiplusStartupInput gsi;
  if(Gdiplus::GdiplusStartup(&token, &gsi, NULL) != Gdiplus::Ok) return 1;
  MSG msg;
  while(GetMessage(&msg, NULL, 0, 0) > 0){
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  Gdiplus::GdiplusShutdown(token);
  return msg.wParam;
}

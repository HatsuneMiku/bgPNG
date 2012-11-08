/*
  bgPlate.cpp

  rc bgPlate.rc
  cl bgPlate.cpp bgPlate.res kernel32.lib user32.lib ole32.lib gdi32.lib \
    gdiplus.lib dwmapi.lib shell32.lib shlwapi.lib -DUNICODE -EHsc
*/

#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <gdiplus.h>
#include <dwmapi.h>
#include <shlwapi.h>
#include <sstream>
#include <string>

using namespace std;

#define NAMEBUFSIZ 512
#define CLASS_NAME L"bgPlate"
#define APP_NAME L"bgPlate"
#define APP_ICON_RCID L"icon01"
#define APP_ICON_TYPE L"IMAGE"
#define MSG_ERR_GDIPLUS_STARTUP L"cannot startup GDIplus"
#define MSG_ERR_LOAD_ICON L"cannot load ICON"
#define MSG_ERR_CONVERT_ICON L"cannot convert ICON"
#define MSG_ERR_GET_HICON L"cannot get HICON"
#define MSG_ERR_REGISTER_CLASS L"cannot register window class"
#define MSG_ERR_CREATE_WINDOW L"cannot create window"
#define MSG_ERR_LOAD_IMAGE L"cannot load IMAGE"
#define MSG_ERR_USAGE L"Usage: bgPlate HWND IMAGEFILE"
#define MSG_ERR_TARGET L"target HWND may not be a hex number"
#define MSG_ERR_FILE L"IMAGEFILE does not exist"
#define MSG_ERR_DIR L"IMAGEFILE is a directory"
#define MSG_ERR_HWND L"target HWND may not be a top window"
#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 640

int ErrMsg(int result, LPCTSTR msg, ULONG_PTR token=NULL, HWND hwnd=NULL)
{
  MessageBox(hwnd, msg, APP_NAME, MB_OK | MB_ICONEXCLAMATION);
  if(token) Gdiplus::GdiplusShutdown(token);
  return result;
}

int ParseCommandLine(HWND hwnd, HWND *target, wstring *filename)
{
  wstring tgt;
  int argc = 0;
  LPWSTR *argv = CommandLineToArgvW(GetCommandLine(), &argc);
  if(argv && argc >= 3){
    // for(int i = 0; i < argc;) MessageBox(NULL, argv[i++], APP_NAME, MB_OK);
    tgt = argv[1];
    filename->assign(argv[2]);
    LocalFree(argv);
  }
  if(argc < 3) return ErrMsg(1, MSG_ERR_USAGE, NULL, hwnd); // after LocalFree
  wistringstream iss(tgt);
  unsigned long t = 0;
  iss >> hex >> t;
  if(!(*target = (HWND)t)) return ErrMsg(1, MSG_ERR_TARGET, NULL, hwnd);
  LPCWSTR fn = filename->c_str();
  if(!PathFileExists(fn)) return ErrMsg(1, MSG_ERR_FILE, NULL, hwnd);
  if(PathIsDirectory(fn)) return ErrMsg(1, MSG_ERR_DIR, NULL, hwnd);
  return 0;
}

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

BOOL SetThrough(HWND hwnd, int through, int l=-1, int r=-1, int t=-1, int b=-1)
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
  static int state = 0;
  static int through = 0;
  static Gdiplus::Image *img = NULL;
  static UINT_PTR timerid = 1001;
  static HWND target = 0;
  static wstring filename, classname, windowtext;
  // HINSTANCE hinst = ((LPCREATESTRUCT)lparam)->hInstance; // WM_CREATE only
  // HINSTANCE hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
  HINSTANCE hinst = GetModuleHandle(NULL);
  switch(msg){
  case WM_CREATE:
    SetThrough(hwnd, 1);
    img = LoadFromResource(hinst, APP_ICON_RCID, APP_ICON_TYPE);
    if(!img) ErrMsg(FALSE, MSG_ERR_LOAD_IMAGE, NULL, hwnd);
    SetTimer(hwnd, timerid, 50, NULL);
    return FALSE;
  case WM_DWMCOMPOSITIONCHANGED: // Aero enabled or disabled
    SetThrough(hwnd, 1);
    return FALSE;
  case WM_LBUTTONDOWN:
    SetThrough(hwnd, through = 1 - through, 40, 40, 80, 80);
    return FALSE;
  case WM_RBUTTONDOWN:
    SetThrough(hwnd, 1);
    return FALSE;
  case WM_PAINT:{
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    RECT r;
    GetClientRect(hwnd, &r);
    Gdiplus::Graphics g(ps.hdc);
    g.Clear(Gdiplus::Color(64, 255, 255, 255)); // ARGB
    int x = (r.right - r.left - img->GetWidth()) / 2;
    int y = (r.bottom - r.top - img->GetHeight()) / 2;
    if(img) g.DrawImage(img, x, y, img->GetWidth(), img->GetHeight());
    EndPaint(hwnd, &ps);
    } return FALSE;
  case WM_TIMER:
    if(!state){ // *** re-entry here with state==1 when calling ErrMsg() ***
      state = 1;
      if(ParseCommandLine(hwnd, &target, &filename)){
        DestroyWindow(hwnd);
        return FALSE;
      }
      WCHAR clsname[NAMEBUFSIZ + 1], wintext[NAMEBUFSIZ + 1];
      clsname[0] = 0, wintext[0] = 0;
      if(!GetClassNameW(target, clsname, NAMEBUFSIZ)
      || !GetWindowTextW(target, wintext, NAMEBUFSIZ)){
        ErrMsg(FALSE, MSG_ERR_HWND, NULL, hwnd);
        DestroyWindow(hwnd);
        return FALSE;
      }
      classname.assign(clsname), windowtext.assign(wintext);
      if(img){ delete img; img = NULL; }
      img = Gdiplus::Image::FromFile(filename.c_str());
      if(!img || img->GetLastStatus() != Gdiplus::Ok)
        img = (Gdiplus::Image *)ErrMsg(NULL, MSG_ERR_LOAD_IMAGE, NULL, hwnd);
      InvalidateRect(hwnd, NULL, TRUE);
    }else{
      if(!target || !img) return FALSE;
      RECT tr, sr;
      if(!GetWindowRect(target, &tr)) return FALSE; // Destroy on state==0
      GetWindowRect(hwnd, &sr);
      HWND tmp1 = FindWindowEx(GetDesktopWindow(), hwnd,
        classname.c_str(), windowtext.c_str());
      HWND tmp2 = FindWindowEx(GetDesktopWindow(), hwnd, NULL, NULL);
#if 0
      if(tmp1 != target)
        SetWindowPos(target, hwnd, 0, 0, 0, 0,
          SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
      if(!memcmp(&tr, &sr, sizeof(RECT)) && tmp2 == target) return FALSE;
#endif
      if(!memcmp(&tr, &sr, sizeof(RECT)) && tmp1 == target) return FALSE;
      SetWindowPos(hwnd, target, // it will be ignored by SWP_NOZORDER
        tr.left, tr.top, tr.right - tr.left, tr.bottom - tr.top,
        SWP_NOACTIVATE|SWP_SHOWWINDOW); // SWP_NOZORDER
      SetWindowPos(target, hwnd, 0, 0, 0, 0,
        SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    }
    return FALSE;
  case WM_DESTROY:
    if(img){ delete img; img = NULL; }
    KillTimer(hwnd, timerid);
    PostQuitMessage(0);
    return FALSE;
  }
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

BOOL RegCls(HINSTANCE hinst, HICON hicon)
{
  WNDCLASSEX wc;
  memset(&wc, 0, sizeof(wc));
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hinst;
  wc.hIcon = hicon; // LoadIcon(NULL, IDI_WINLOGO);
  wc.hIconSm = wc.hIcon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = CLASS_NAME;
  return RegisterClassEx(&wc);
}

HICON CreateIconFromResource(HINSTANCE hinst, LPCTSTR name, LPCTSTR typ)
{
  HICON hicon = NULL;
  Gdiplus::Image *img = LoadFromResource(hinst, APP_ICON_RCID, APP_ICON_TYPE);
  if(!img) return (HICON)ErrMsg(NULL, MSG_ERR_LOAD_ICON);
  int w = img->GetWidth(), h = img->GetHeight();
  Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(w, h, img->GetPixelFormat());
  if(!bmp) return (HICON)ErrMsg(NULL, MSG_ERR_CONVERT_ICON);
  Gdiplus::Color bkc(0, 255, 255, 255); // ARGB
  { // convert Gdiplus::Image to Gdiplus::Bitmap using 'GDI+' Graphics Object
    Gdiplus::Graphics g(bmp); // g must be free here (before delete bmp)
    g.Clear(bkc);
    g.DrawImage(img, 0, 0, w, h);
  }
#if 0 // create Icon using 'GDI'
  HWND hwnd = GetDesktopWindow();
  HDC hdc = GetDC(hwnd);
  HBITMAP hbitmap = NULL;
  bmp->GetHBITMAP(bkc, &hbitmap); // create (must DeleteObject later)
  if(!hbitmap) return (HICON)ErrMsg(NULL, MSG_ERR_CONVERT_ICON);
  HBITMAP hbmMask = CreateCompatibleBitmap(hdc, w, h);
  if(!hbmMask) return (HICON)ErrMsg(NULL, MSG_ERR_CONVERT_ICON);
  ICONINFO ii = {0}; // with clear hotspots (x, y)
  ii.fIcon = TRUE; // not Cursor
  ii.hbmColor = hbitmap; // will be copied
  ii.hbmMask = hbmMask; // will be copied
  hicon = CreateIconIndirect(&ii);
  DeleteObject(hbmMask);
  DeleteObject(hbitmap);
  ReleaseDC(hwnd, hdc);
#else // create Icon using 'GDI+'
  bmp->GetHICON(&hicon); // create (must DestroyIcon later)
#endif
  delete bmp; bmp = NULL;
  delete img; img = NULL;
  return hicon; // must DestroyIcon later
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR cmd, int ncmd)
{
  // GDI+ must be initialized before create window (use GDI+ at WM_CREATE)
  ULONG_PTR token;
  Gdiplus::GdiplusStartupInput gsi;
  if(Gdiplus::GdiplusStartup(&token, &gsi, NULL) != Gdiplus::Ok)
    return ErrMsg(1, MSG_ERR_GDIPLUS_STARTUP);
  HICON hicon = CreateIconFromResource(hinst, APP_ICON_RCID, APP_ICON_TYPE);
  if(!hicon) return ErrMsg(1, MSG_ERR_GET_HICON, token);
  if(!RegCls(hinst, hicon)) return ErrMsg(1, MSG_ERR_REGISTER_CLASS, token);
  int w = WINDOW_WIDTH, h = WINDOW_HEIGHT;
  int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
  int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
  HWND hwnd = CreateWindow(CLASS_NAME, APP_NAME, WS_POPUP | WS_VISIBLE,
    x, y, w, h, NULL, NULL, hinst, NULL);
  if(!hwnd) return ErrMsg(1, MSG_ERR_CREATE_WINDOW, token);
  ShowWindow(hwnd, ncmd);
  UpdateWindow(hwnd);
  MSG msg;
  while(GetMessage(&msg, NULL, 0, 0) > 0){
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  DestroyIcon(hicon);
  Gdiplus::GdiplusShutdown(token);
  return msg.wParam;
}

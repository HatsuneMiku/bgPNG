/*
  C_GDIplus.c

  cl C_GDIplus.c kernel32.lib user32.lib gdi32.lib

  late binding LoadLibrary("gdiplus.dll") instead of link gdiplus.lib
*/

#include <windows.h>

#define PNGTEST L".\\resource\\\x7a93\x8fba.png" // 窓辺.png in unicode

typedef ULONG DebugEventLevel;
typedef VOID (WINAPI *DebugEventProc)(DebugEventLevel level, CHAR *message);
typedef struct tagGdiplusStartupInput { // There is a constructor for C++
  UINT32 GdiplusVersion;            // UINT32         1
  DebugEventProc DebugEventCallback;// DebugEventProc NULL
  BOOL SuppressBackgroundThread;    // BOOL           FALSE
  BOOL SuppressExternalCodecs;      // BOOL           FALSE
} GdiplusStartupInput;
typedef ULONG Status;
typedef Status (WINAPI *NotificationHookProc)(ULONG_PTR *token);
typedef VOID (WINAPI *NotificationUnhookProc)(ULONG_PTR token);
typedef struct tagGdiplusStartupOutput { // There is a constructor for C++
  NotificationHookProc NotificationHook;
  NotificationUnhookProc NotificationUnhook;
} GdiplusStartupOutput;
typedef Status (WINAPI *tGdiplusStartup)(               // GpStatus
  ULONG_PTR *token,                 // ULONG_PTR *
  GdiplusStartupInput *inputbuf,    // GdiplusStartupInput *
  GdiplusStartupOutput *outputbuf   // GdiplusStartupOutput *
);
typedef VOID (WINAPI *tGdiplusShutdown)(                // void
  ULONG_PTR token                   // ULONG_PTR
);
typedef Status (WINAPI *tGdipCreateBitmapFromFile)(     // GpStatus
  WCHAR *filename,                  // GDIPCONST WCHAR *
  ULONG **bitmap                    // GpBitmap **
);
typedef Status (WINAPI *tGdipCreateHBITMAPFromBitmap)(  // GpStatus
  ULONG *bitmap,                    // GpBitmap *
  HBITMAP *hbmReturn,               // HBITMAP *
  LONG background                   // ARGB
);
typedef Status (WINAPI *tGdipDisposeImage)(             // GpStatus
  ULONG *image                      // GpImage *
);

tGdiplusStartup GdiplusStartup;
tGdiplusShutdown GdiplusShutdown;
tGdipCreateBitmapFromFile GdipCreateBitmapFromFile;
tGdipCreateHBITMAPFromBitmap GdipCreateHBITMAPFromBitmap;
tGdipDisposeImage GdipDisposeImage;

ULONG_PTR loadfunctions(HMODULE hmodule)
{
  ULONG_PTR result = 1;
  if(!hmodule) return 0;
#define LOADFUNC(f) (result*=(ULONG_PTR)(f=(t##f)GetProcAddress(hmodule,#f)))
  LOADFUNC(GdiplusStartup);
  LOADFUNC(GdiplusShutdown);
  LOADFUNC(GdipCreateBitmapFromFile);
  LOADFUNC(GdipCreateHBITMAPFromBitmap);
  LOADFUNC(GdipDisposeImage);
  return result;
}

ULONG loadimage(WCHAR *filename)
{
  ULONG_PTR token;
  GdiplusStartupInput gsi = {1, NULL, FALSE, FALSE};
  if(!GdiplusStartup(&token, &gsi, 0)){
    ULONG *bitmap;
    if(!GdipCreateBitmapFromFile(filename, &bitmap)){
      HBITMAP hbmp;
      GdipCreateHBITMAPFromBitmap(bitmap, &hbmp, 0);
      if(hbmp){
        BITMAP bmp;
        HWND hwnd = GetDesktopWindow();
        HDC hdc = GetDC(hwnd);
        HDC hmemdc = CreateCompatibleDC(hdc);
        HBITMAP holdbmp = (HBITMAP)SelectObject(hmemdc, hbmp);
        GetObject(hbmp, sizeof(BITMAP), &bmp);
        BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, hmemdc, 0, 0, SRCCOPY);
        SelectObject(hmemdc, holdbmp);
        DeleteDC(hmemdc);
        ReleaseDC(hwnd, hdc);
        DeleteObject(hbmp);
      }
      GdipDisposeImage(bitmap);
    }
    GdiplusShutdown(token);
  }
  return 0;
}

int main(int ac, char **av)
{
  HMODULE hmodule = LoadLibrary("gdiplus.dll");
  if(loadfunctions(hmodule)) loadimage(PNGTEST);
  FreeLibrary(hmodule);
  return 0;
}

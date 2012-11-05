/*
  W_GDIplus.c

  rc W_GDIplus.rc
  cl W_GDIplus.c W_GDIplus.res kernel32.lib user32.lib gdi32.lib -DUNICODE
  late binding LoadLibrary("gdiplus.dll") instead of link gdiplus.lib
*/

#include <windows.h>

#define CLASS_NAME L"W_GDIPLUS"
#define APP_NAME L"W_GDIPLUS"
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 768

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch(msg){
  case WM_LBUTTONDOWN:
    MessageBox(hwnd, L"WM_LBUTTON", APP_NAME, MB_OK);
    return FALSE;
  case WM_DESTROY:
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
  if(!RegCls(hinst)){
    return 1;
  }else{
    int x = (GetSystemMetrics(SM_CXSCREEN) - WINDOW_WIDTH) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - WINDOW_HEIGHT) / 2;
    HWND hwnd = CreateWindow(CLASS_NAME, APP_NAME,
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      x, y, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hinst, NULL);
    if(!hwnd){
      return 1;
    }else{
      MSG msg;
      ShowWindow(hwnd, SW_SHOW);
      UpdateWindow(hwnd);
      while(GetMessage(&msg, NULL, 0, 0) > 0){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      return 0;
    }
  }
}

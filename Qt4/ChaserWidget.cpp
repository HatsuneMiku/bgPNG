/*
  ChaserWidget.cpp
*/

#include "ChaserWidget.h"

#include <windows.h>

using namespace std;

ChaserWidget::ChaserWidget(const QString &name,
  QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags),
  self_name(name), hwnd(0), prev_window(0), dragging(false)
{
  char *work[] = {self_name.toAscii().data(), "msime", "Program Manager"};
  size_t sz = sizeof(work) / sizeof(work[0]);
  exclude.reserve(sz);
  for(size_t i = 0; i < sz; i++) exclude.push_back(string(work[i]));
}

void ChaserWidget::drawXORrect(ulong w)
{
  RECT r;
  BOOL result = GetClientRect((HWND)w, &r);
  if(result){
    HDC dc = GetDC((HWND)w);
    HPEN pen = (HPEN)CreatePen(PS_SOLID, 8, RGB(255, 0, 0));
    HPEN op = (HPEN)SelectObject(dc, pen);
    // HBRUSH br = (HBRUSH)CreateHatchBrush(HS_BDIAGONAL, RGB(0, 0, 0));
    HBRUSH br = (HBRUSH)CreateSolidBrush(RGB(15, 15, 15)); // xor
    HBRUSH ob = (HBRUSH)SelectObject(dc, br);
    int om = SetBkMode(dc, TRANSPARENT);
    int orop = SetROP2(dc, R2_XORPEN);
    Rectangle(dc, r.left, r.top, r.right, r.bottom);
    SetROP2(dc, orop);
    SetBkMode(dc, om);
    SelectObject(dc, ob);
    DeleteObject(br);
    SelectObject(dc, op);
    DeleteObject(pen);
    ReleaseDC((HWND)w, dc);
  }
}

int ChaserWidget::cmpWindowName(const char *buf)
{
  for(size_t i = 0; i < exclude.size(); i++) if(buf == exclude[i]) return 0;
  return 1;
}

void ChaserWidget::mouseMoveEvent(QMouseEvent *ev)
{
  // if(ev->button() != Qt::LeftButton) return;
  if(!dragging) dragging = true;
  QPoint p = ev->globalPos();
  // cout << p.x() << ", " << p.y() << endl;
  HWND w = GetTopWindow(GetDesktopWindow());
  while(w){
    if(IsWindowVisible(w)){
      RECT r;
      BOOL result = GetWindowRect(w, &r);
      if(result
      && QRect(r.left, r.top, r.right - r.left, r.bottom - r.top).contains(p)){
        if(w == (HWND)prev_window) break;
        if(prev_window) drawXORrect(prev_window);
        char buf[1024];
        if(GetWindowTextA(w, buf, sizeof(buf))){
          if(!cmpWindowName(buf)){
            prev_window = hwnd = 0;
            scls = QString("");
            swnd = QString("");
            emit clear();
            break;
          }
          drawXORrect(prev_window = hwnd = (ulong)w);
          char cls[1024];
          if(!GetClassNameA(w, cls, sizeof(cls))) cls[0] = '\0';
          scls = QString::fromLocal8Bit(cls); // fromAscii(cls);
          swnd = QString::fromLocal8Bit(buf); // fromAscii(buf);
          emit hover(hwnd);
        }
        break;
      }
    }
    w = GetWindow(w, GW_HWNDNEXT);
  }
}

void ChaserWidget::mousePressEvent(QMouseEvent *ev)
{
  // if(ev->button() == Qt::RightButton) releaseMouse();
}

void ChaserWidget::mouseReleaseEvent(QMouseEvent *ev)
{
  if(dragging){
    dragging = false;
    if(prev_window){
      drawXORrect(prev_window);
      prev_window = 0;
    }
    if(hwnd){ emit dropped(hwnd); }
  }
}

void ChaserWidget::paintEvent(QPaintEvent *ev)
{
  if(pixmap.isNull()) return;
  QPainter p(this);
  // p.setBackgroundMode(Qt::TransparentMode); // default
  int w = width(), h = height();
  double q = (double)w / (double)h;
  double r = (double)pixmap.width() / (double)pixmap.height();
  int dst_w = r <= q ? (int)(r * h) : w;
  int dst_h = r <= q ? h : (int)(w / r);
#if 0
  p.drawPixmap(w - dst_w, h - dst_h, dst_w, dst_h,
    pixmap, 0, 0, pixmap.width(), pixmap.height());
#else
  p.drawPixmap(w - dst_w, h - dst_h,
    pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
#endif
}

void ChaserWidget::setPixmap(const QPixmap &pm)
{
  pixmap = pm;
  setUpdatesEnabled(true);
  update(); // repaint();
}

/*
  ChaserWidget.cpp
*/

#include "ChaserWidget.h"

#include <windows.h>

using namespace std;

ChaserWidget::ChaserWidget(const char *name,
  QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags),
  self_name(name), hwnd(0), prev_window(0)
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

int ChaserWidget::cmpWindowName(char *buf)
{
  for(size_t i = 0; i < exclude.size(); i++)
    if(!strncmp(buf, exclude[i].c_str(), strlen(exclude[i].c_str()))) return 0;
  return 1;
}

void ChaserWidget::mouseMoveEvent(QMouseEvent *ev)
{
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
            emit clear();
            break;
          }
          drawXORrect(prev_window = hwnd = (ulong)w);
          ostringstream oss;
          oss << setw(8) << setfill('0') << hex << right << hwnd;
          oss << " w[" << buf << "]";
          char cls[1024];
          if(GetClassNameA(w, cls, sizeof(cls))){
            oss << " c[" << cls << "]";
          }
          emit handle(QString(oss.str().c_str()));
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
  if(prev_window){
    drawXORrect(prev_window);
    prev_window = 0;
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

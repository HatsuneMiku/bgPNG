/*
  bgPNG.cpp
*/

#include "bgPNG.h"

using namespace std;

bgPNG::bgPNG(QThread *thread, int n) :
  QWidget(0, Qt::FramelessWindowHint), // fource set parent = 0
  th(thread), num(n), stat(0)
{
  qDebug("[bgPNG created: %d, %08x]", num, (uint)th->currentThreadId());
  // Widgets must be created in the GUI thread
  // Widgets cannot be moved to a new thread
  // this->moveToThread(th);
  setAttribute(Qt::WA_TranslucentBackground);
  move(40, 20);
  resize(400, 200);
  show();
}

bgPNG::~bgPNG()
{
  qDebug("[bgPNG deleted: %d, %08x]", num, (uint)th->currentThreadId());
}

void bgPNG::paintEvent(QPaintEvent *ev)
{
  QPainter p(this);
  p.setBackgroundMode(Qt::TransparentMode);
  p.setBackground(QBrush(Qt::darkGreen, Qt::DiagCrossPattern));
  p.eraseRect(rect());
  p.setBrush(QBrush(Qt::cyan, Qt::CrossPattern));
  p.setPen(QPen(Qt::white, 5));
  p.drawRect(rect());
}

void bgPNG::get(const QString &s)
{
  qDebug("[bgPNG get: %d, %08x]", num, (uint)th->currentThreadId());
  stat = 1;
  cout << qPrintable(s) << endl;
}

void bgPNG::getb(const QByteArray &b)
{
  qDebug("[bgPNG getb: %d, %08x]", num, (uint)th->currentThreadId());
  stat = 1;
  vector<uchar> v(b.size()); // filled by '\0' x n
  for(QByteArray::ConstIterator it = b.begin(); it != b.end(); it++)
    v.push_back(*it);
  for(vector<uchar>::iterator it = v.begin(); it != v.end(); it++)
    cout << setw(2) << setfill('0') << hex << right << (uint)*it << ", ";
  cout << endl;
}

void bgPNG::proc(void)
{
  volatile static int c = 0;
  qDebug("[bgPNG proc: %d, %08x, %02d]", num, (uint)th->currentThreadId(), c);
  if(++c < 100) return;
  if(c > 100) c = 0; // check after increment twice because singleton
  setUpdatesEnabled(true);
  update(); // repaint();
  if(stat){
    stat = 0;
    emit done(); // must be called once because this object will be deleted
  }
}

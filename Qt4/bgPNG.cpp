/*
  bgPNG.cpp
*/

#include "bgPNG.h"

using namespace std;

bgPNG::bgPNG(QThread *thread, int n) : QObject(), // fource set parent = 0
  th(thread), num(n), stat(0)
{
  qDebug("[bgPNG created: %d, %08x]", num, (uint)th->currentThreadId());
  this->moveToThread(th);
}

bgPNG::~bgPNG()
{
  qDebug("[bgPNG deleted: %d, %08x]", num, (uint)th->currentThreadId());
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
  qDebug("[bgPNG proc: %d, %08x]", num, (uint)th->currentThreadId());
  if(stat){
    stat = 0;
    emit done(); // must be called once because this object will be deleted
  }
}

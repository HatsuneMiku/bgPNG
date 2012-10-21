/*
  bgPNG.cpp

  http://www.forest.impress.co.jp/img/wf/docs/567/238/html/image2.jpg.html
  http://www.forest.impress.co.jp/docs/review/20121019_567238.html
*/

#include "bgPNG.h"

using namespace std;

QbgPNG::QbgPNG(QObject *parent) : QObject(parent)
{
}

void QbgPNG::conout(const QString &s)
{
  cout << qPrintable(s) << endl;
}

void QbgPNG::conoutb(const QByteArray &b)
{
  vector<uchar> v(b.size()); // filled by '\0' x n
  for(QByteArray::ConstIterator it = b.begin(); it != b.end(); it++)
    v.push_back(*it);
  for(vector<uchar>::iterator it = v.begin(); it != v.end(); it++)
    cout << setw(2) << setfill('0') << hex << right << (ushort)*it << ", ";
  cout << endl;
}

void QbgPNG::fin(void)
{
  emit done();
}

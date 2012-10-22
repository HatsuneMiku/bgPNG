/*
  bgPNG.cpp
*/

#include "bgPNG.h"

using namespace std;

bgPNG::bgPNG(int n, QObject *parent) : QObject(parent), num(n), stat(0)
{
}

void bgPNG::get(const QString &s)
{
  cout << qPrintable(s) << endl;
}

void bgPNG::getb(const QByteArray &b)
{
  vector<uchar> v(b.size()); // filled by '\0' x n
  for(QByteArray::ConstIterator it = b.begin(); it != b.end(); it++)
    v.push_back(*it);
  for(vector<uchar>::iterator it = v.begin(); it != v.end(); it++)
    cout << setw(2) << setfill('0') << hex << right << (ushort)*it << ", ";
  cout << endl;
}

void bgPNG::fin(void)
{
  stat = 1;
  emit done();
}

/*
  bgPNG.h
*/

#ifndef __BGPNG_H__
#define __BGPNG_H__

#include <QtCore>
#include <QtGui>
#include <QThread>
#include <vector>
#include <iomanip>
#include <iostream>

class bgPNG : public QWidget {
  Q_OBJECT
  Q_DISABLE_COPY(bgPNG)

public:
  bgPNG(QThread *thread, int n);
  virtual ~bgPNG();
  int getnum() { return num; }
  int getstat() { return stat; }
  QByteArray &getdat() { return dat; }
  void get(const QString &s);
  void getb(const QByteArray &b);

protected:
  void paintEvent(QPaintEvent *ev);

signals:
  void done();

private slots:
  void proc(void);

private:
  QThread *th;
  QString bgImgPath;
  int num;
  volatile int stat;
  QByteArray dat;
};

#endif // __BGPNG_H__

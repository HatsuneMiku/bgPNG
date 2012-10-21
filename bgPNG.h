/*
  bgPNG.h
*/

#ifndef __BGPNG_H__
#define __BGPNG_H__

#include <QtCore>
// #include <QtGui>
#include <vector>
#include <iomanip>
#include <iostream>

class QbgPNG : public QObject {
  Q_OBJECT
public:
  QbgPNG(QObject *parent=0);
  void conout(const QString &s);
  void conoutb(const QByteArray &b);
signals:
  void done();
private slots:
  void fin(void);
private:
  QString bgImgPath;
};

#endif

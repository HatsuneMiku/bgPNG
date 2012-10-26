/*
  ChaserWidget.h
*/

#ifndef __CHASERWIDGET_H__
#define __CHASERWIDGET_H__

#include <QtGui>
#include <QWidget>
#include <vector>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>

class ChaserWidget : public QWidget {
  Q_OBJECT

public:
  ChaserWidget(const QString &name,
    QWidget *parent=0, Qt::WindowFlags flags=0);
  virtual ~ChaserWidget(){}

protected:
  void drawXORrect(ulong w);
  int cmpWindowName(char *buf);
  void mouseMoveEvent(QMouseEvent *ev);
  void mousePressEvent(QMouseEvent *ev);
  void mouseReleaseEvent(QMouseEvent *ev);
  void paintEvent(QPaintEvent *ev);

signals:
  void clear();
  void handle(const QString &txt);

public slots:
  void setPixmap(const QPixmap &pm);

private:
  QPixmap pixmap;
  QString self_name;
  std::vector<std::string> exclude;
  ulong hwnd, prev_window;
};

#endif // __CHASERWIDGET_H__

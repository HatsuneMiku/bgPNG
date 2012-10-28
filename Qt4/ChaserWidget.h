/*
  ChaserWidget.h
*/

#ifndef __CHASERWIDGET_H__
#define __CHASERWIDGET_H__

#include <QtGui>
#include <QWidget>
#include <vector>
#include <string>

class ChaserWidget : public QWidget {
  Q_OBJECT

public:
  ChaserWidget(const QString &name,
    QWidget *parent=0, Qt::WindowFlags flags=0);
  virtual ~ChaserWidget() {}
  const QString &getScls(void) { return scls; }
  const QString &getSwnd(void) { return swnd; }

protected:
  void drawXORrect(ulong w);
  int cmpWindowName(const char *buf);
  void mouseMoveEvent(QMouseEvent *ev);
  void mousePressEvent(QMouseEvent *ev);
  void mouseReleaseEvent(QMouseEvent *ev);
  void paintEvent(QPaintEvent *ev);

signals:
  void clear();
  void hover(ulong hwnd);
  void dropped(ulong hwnd);

public slots:
  void setPixmap(const QPixmap &pm);

private:
  QPixmap pixmap;
  QString self_name;
  std::vector<std::string> exclude;
  ulong hwnd, prev_window;
  QString scls, swnd;
  bool dragging;
};

#endif // __CHASERWIDGET_H__

/*
  ChaseThread.h
*/

#ifndef __CHASETHREAD_H__
#define __CHASETHREAD_H__

#include <QtCore>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>

class ChaseThread : public QThread {
  Q_OBJECT
  Q_DISABLE_COPY(ChaseThread)

public:
  ChaseThread(QObject *parent=0);
  void stop();

signals:
  //

private slots:
  void run();
  void active();

private:
  volatile bool stopped;
  mutable QMutex mutex;
  QWaitCondition cond;
};

#endif // __CHASETHREAD_H__

/*
  ChaseThread.h
*/

#ifndef __CHASETHREAD_H__
#define __CHASETHREAD_H__

#include <QtCore>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QTimer>

class ChaseThread : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(ChaseThread)

public:
  ChaseThread(QThread *thread);

signals:
  void proc();

public slots:
  void stop();

private slots:
  void chase();
  void started();
  void active();

private:
  QTimer timer;
  QThread *th;
  mutable QMutex mutex;
  QWaitCondition cond;
};

#endif // __CHASETHREAD_H__

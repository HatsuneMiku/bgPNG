/*
  ChaseThread.cpp

  Must not inherit QThread and must not use moveToThread(this).
  http://blog.qt.digia.com/2010/06/17/youre-doing-it-wrong/
  http://blog.qt.digia.com/2006/12/04/threading-without-the-headache/
  http://vivi.dyndns.org/tech/Qt/QThread.html
*/

#include "ChaseThread.h"

ChaseThread::ChaseThread(QThread *thread) : QObject(), // fource set parent = 0
  timer(this), th(thread)
{
  qDebug("[ChaseThread created: %08x]", (uint)th->currentThreadId());
  this->moveToThread(th);
  connect(th, SIGNAL(started()), this, SLOT(started()));
}

ChaseThread::~ChaseThread()
{
  qDebug("[ChaseThread deleted: %08x]", (uint)th->currentThreadId());
}

void ChaseThread::stop()
{
  timer.stop();
  qDebug("[ChaseThread timer stop: %08x]", (uint)th->currentThreadId());
}

void ChaseThread::started()
{
  qDebug("[ChaseThread timer start: %08x]", (uint)th->currentThreadId());
  connect(&timer, SIGNAL(timeout()), this, SLOT(chase()));
  timer.start(50); // QTimer can only be used with threads started with QThread
}

void ChaseThread::chase()
{
  static int i = 0;
  if(++i > 200){
    qDebug("[ChaseThread chase: %08x]", (uint)th->currentThreadId());
    i = 0;
  }
  // 今のところ二重に呼ばれる訳ではない
  {
    QMutexLocker locker(&mutex);
    emit proc();
  }
}

void ChaseThread::active()
{
  cond.wakeOne();
  qDebug("actived");
}

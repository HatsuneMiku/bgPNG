/*
  ChaseThread.cpp
*/

#include "ChaseThread.h"

ChaseThread::ChaseThread() : QThread(), timer(this) // fource set parent = 0
{
  //qDebug("[ChaseThread before moveToThread: %08x]", (uint)currentThreadId());
  moveToThread(this);
  //qDebug("[ChaseThread after moveToThread: %08x]", (uint)currentThreadId());
}

void ChaseThread::stop()
{
  timer.stop();
  //qDebug("[ChaseThread timer stop: %08x]", (uint)currentThreadId());
}

void ChaseThread::chase()
{
  // 今のところ二重に呼ばれる訳ではない
  {
    QMutexLocker locker(&mutex);
    emit proc();
  }
}

void ChaseThread::run()
{
  //qDebug("[ChaseThread run(in): %08x]", (uint)currentThreadId());
  //qDebug("[ChaseThread timer start: %08x]", (uint)currentThreadId());
  connect(&timer, SIGNAL(timeout()), this, SLOT(chase()));
  timer.start(50); // QTimer can only be used with threads started with QThread
  QThread::run(); // exec(); to catch signals (must call moveToThread() before)
  //qDebug("[ChaseThread run(out): %08x]", (uint)currentThreadId());
}

void ChaseThread::active()
{
  cond.wakeOne();
  qDebug("actived");
}

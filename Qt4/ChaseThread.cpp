/*
  ChaseThread.cpp
*/

#include "ChaseThread.h"

ChaseThread::ChaseThread() : QThread(), stopped(FALSE) // fource set parent = 0
{
  qDebug("[ChaseThread before moveToThread: %08x]", (uint)currentThreadId());
  moveToThread(this);
  qDebug("[ChaseThread after moveToThread: %08x]", (uint)currentThreadId());
}

void ChaseThread::stop()
{
  qDebug("[ChaseThread stop: %08x]", (uint)currentThreadId());
  stopped = TRUE;
}

void ChaseThread::run()
{
  qDebug("[ChaseThread run(in): %08x]", (uint)currentThreadId());
  while(!stopped){
    // 今のところ二重に呼ばれる訳ではない
    {
      QMutexLocker locker(&mutex);
      emit proc();
      exec(); // event loop to catch signals (must call moveToThread() before)
    }
    usleep(100000);
  }
  qDebug("[ChaseThread run(out): %08x]", (uint)currentThreadId());
  stopped = FALSE;
}

void ChaseThread::active()
{
  cond.wakeOne();
  qDebug("actived");
}

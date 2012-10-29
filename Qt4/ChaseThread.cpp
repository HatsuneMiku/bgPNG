/*
  ChaseThread.cpp
*/

#include "ChaseThread.h"

ChaseThread::ChaseThread() : QThread(), stopped(FALSE) // fource set parent = 0
{
  moveToThread(this);
}

void ChaseThread::stop()
{
  stopped = TRUE;
}

void ChaseThread::run()
{
  while(!stopped){
    // 今のところ二重に呼ばれる訳ではない
    {
      QMutexLocker locker(&mutex);
      emit proc();
      exec(); // event loop to catch signals (must call moveToThread() before)
    }
    usleep(100000);
  }
  stopped = FALSE;
}

void ChaseThread::active()
{
  cond.wakeOne();
  qDebug("actived");
}

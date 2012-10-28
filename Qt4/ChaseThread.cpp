/*
  ChaseThread.cpp
*/

#include "ChaseThread.h"

ChaseThread::ChaseThread(QObject *parent) : QThread(parent), stopped(FALSE)
{
  // moveToThread(this);
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
    }
    usleep(100000);
  }
  stopped = FALSE;
  // exec();
  // quit();
}

void ChaseThread::active()
{
  cond.wakeOne();
  qDebug("actived");
}

/*
  ChaseThread.cpp
*/

#include "MainWindow.h" // MainWindow * にアクセスするので必要
// #include "ChaseThread.h" // 上で読まれるのでこちらは省略

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
      ((MainWindow *)parent())->proc();
    }
    usleep(100000);
  }
  stopped = FALSE;
  // exec();
}

void ChaseThread::active()
{
  cond.wakeOne();
  qDebug("actived");
}

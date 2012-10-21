/*
  main.cpp
*/

#include "bgPNG.h"

int main(int ac, char **av)
{
  QCoreApplication app(ac, av);
  QbgPNG bp;
  QObject::connect(&bp, SIGNAL(done()), &app, SLOT(quit()));
  QString txt(QObject::trUtf8("日本語表示UTF8"));
  bp.conout(txt);
  QTextCodec *jp = QTextCodec::codecForName("UTF-8");
  bp.conoutb(jp->fromUnicode(txt));
  QTimer::singleShot(0, &app, SLOT(quit())); // called after (exec)
  return app.exec();
}

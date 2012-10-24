/*
  main.cpp
*/

#include "MainWindow.h"

using namespace std;

int main(int ac, char **av)
{
  // 起動インスタンスをひとつにするには QSingle(Core)Application を使う option
  QApplication app(ac, av);
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
  QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));

/*
  QTranslator appTranslator;
  appTranslator.load(QLocale::system().name(), ":/translations");
  app.installTranslator(&appTranslator);

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
    qApp->applicationDirPath());
  app.installTranslator(&qtTranslator);
*/

#if 0
  cout << QTextCodec::codecForLocale()->name().constData() << endl;
  // QTextCodec::setCodecForLocale(QTextCodec::codecForName("System"));
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
  QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
  QString qs(QObject::trUtf8("abc日本語表示xyz"));
  wstring ws(qs.toStdWString());
  cout << "toStdWString: " << ws.c_str() << endl;
  string ls(qs.toLocal8Bit()); // QByteArray data() constData()
  cout << "toLocal8Bit: " << ls << endl;
  string as(qs.toAscii()); // QByteArray data() constData()
  cout << "toAscii: " << as << endl;
  string ss(qs.toStdString()); // 偶々動くかも知れないし動かないかも知れない
  cout << "toStdString: " << ss << endl;
  return 0;
#endif

  // TrayIcon を使うので DeleteOnClose も destroyed への接続も不要
  if(!QSystemTrayIcon::isSystemTrayAvailable()){
    QMessageBox::critical(0, QObject::trUtf8(APP_NAME),
      QObject::trUtf8("システムトレイがありません"));
      return 1;
  }
  QApplication::setQuitOnLastWindowClosed(false);

  QQueue<QString> quelst;
  MainWindow w(quelst);
  // w.setAttribute(Qt::WA_DeleteOnClose); // 下の connect と両方だと UAE 出る
  // connect(&w, SIGNAL(destroyed()), &app, SLOT(quit()));

  return app.exec();
}

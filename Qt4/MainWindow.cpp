/*
  MainWindow.cpp
*/

#include "MainWindow.h"

#include <windows.h>

using namespace std;

ChaserWidget::ChaserWidget(QWidget *parent, Qt::WindowFlags flags) :
  QWidget(parent, flags)
{
}

void ChaserWidget::paintEvent(QPaintEvent *ev)
{
  if(pixmap.isNull()) return;
  QPainter p(this);
  p.drawPixmap(0, 0, width(), height(),
    pixmap, 0, 0, pixmap.width(), pixmap.height());
/*
  double r = (double)pixmap.width() / (double)pixmap.height();
  int w = width(), h = height();
  p.drawPixmap(0, 0, r < 1.0 ? (int)(r * w) : w, r < 1.0 ? h : (int)(h / r),
    pixmap, 0, 0, pixmap.width(), pixmap.height());
*/
}

MainWindow::MainWindow(QQueue<QString> &q,
  QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags),
  hwnd(0), prev_window(0), cw(0), th(0), quelst(q),
  mChaseAction(0), mFileMenu(0), mViewMenu(0), mFileToolBar(0),
  mHANDLE(0), mText(0), mModel(0), mTree(0),
  mMinimizeAction(0), mMaximizeAction(0), mRestoreAction(0), mQuitAction(0),
  mTrayIcon(0), mTrayIconMenu(0)
{
  QIcon ico = QIcon(APP_ICON);
  QImage img(SAMPLE_IMG);
  if(!img.isNull()){
    QPixmap pixmap = QPixmap::fromImage(
      img.convertToFormat(QImage::Format_ARGB32));
    ico = QIcon(pixmap.scaled(32 * pixmap.width() / pixmap.height(), 32,
      Qt::KeepAspectRatio, Qt::FastTransformation));
  }
  setWindowIcon(ico);
  setWindowTitle(trUtf8(APP_NAME));
  resize(960, 480);

  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanupcode()));
  connect(this, SIGNAL(toBeAbort()), qApp, SLOT(quit()));

  home = QString(trUtf8("%1/.%2").arg(QDir::homePath()).arg(APP_NAME));
  qDebug("home: %s", home.toUtf8().constData());
  QString fimg(trUtf8("%1/%2").arg(home).arg(SAMPLE_NAME));
  if(!QDir().exists(home)){
    if(!QDir().mkdir(home)){
      QString msg(trUtf8("ホームディレクトリを作成出来ません\n%1").arg(home));
      QMessageBox::critical(this, trUtf8(APP_NAME), msg, QMessageBox::Cancel);
      emit toBeAbort();
    }else{
      if(!img.isNull()) img.save(fimg);
    }
  }

  createActions();
  createMenus();
  createToolBar();
  createStatusBar();
  createDockWindows();
  createTrayIcon();
  mTrayIcon->setIcon(ico);
  mTrayIcon->setToolTip(trUtf8(APP_NAME));

  mModel = new QDirModel;
  mModel->setReadOnly(true);
  // ここで sort すると起動が極端に遅くなる(固まる)のでスキップ
  // (特にネットワークドライブがツリーに含まれていると危険＝起動後でも固まる)
  // mModel->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);
  mTree = new QTreeView;
  mTree->setModel(mModel);
  mTree->header()->setStretchLastSection(true);
  mTree->header()->setSortIndicator(0, Qt::AscendingOrder);
  mTree->header()->setSortIndicatorShown(true);
  mTree->header()->setClickable(true);
  // mTree->setRootIndex(mModel->index(home));
  // QModelIndex idx = mModel->index(QDir::currentPath());
  QModelIndex idx = mModel->index(img.isNull() ? home : fimg);
  mTree->setCurrentIndex(idx);
  mTree->expand(idx);
  mTree->scrollTo(idx);
  mTree->resizeColumnToContents(0);
  setCentralWidget(mTree);

  loadLayout();
  mTrayIcon->show();
  show();

  connect(mTree, SIGNAL(activated(const QModelIndex &)),
    this, SLOT(treeActivated(const QModelIndex &)));
  treeActivated(idx);

  QString fname(trUtf8("%1/%2").arg(home).arg(APP_DATA));
  db = QSqlDatabase::addDatabase(APP_DB);
  db.setDatabaseName(fname);
  if(!QFileInfo(fname).exists()){
    if(!db.open()){
      QString msg(trUtf8("データベースを初期化出来ません\n%1").arg(fname));
      QMessageBox::critical(this, trUtf8(APP_NAME), msg, QMessageBox::Cancel);
    }else{
      QString msg(trUtf8("データベースが無いため初期化します\n%1").arg(fname));
      QMessageBox::warning(this, trUtf8(APP_NAME), msg);
      QSqlQuery q;
      q.exec("drop table if exists testtable;");
      q.exec("create table testtable (id integer primary key,"
        " c1 varchar(255), c2 varchar(255), c3 integer);");
      db.close();
    }
  }

  th = new ChaseThread(this);
  th->start();
}

void MainWindow::proc()
{
  // 再入禁止?
  if(quelst.isEmpty()) return;
  if(!db.open()){
    QString msg(trUtf8("データベースを開けません"));
    QMessageBox::critical(this, trUtf8(APP_NAME), msg, QMessageBox::Cancel);
  }else{
    db.transaction();
    QSqlQuery q;
    while(!quelst.isEmpty()){
      QString link(quelst.dequeue());
      QString v(trUtf8("'%1', '%2', %3").arg(link).arg(link).arg(0));
      // sqlite は複数の values を連結出来ないらしい
      q.exec(trUtf8("insert into testtable (c1, c2, c3) values (%1);").arg(v));
    }
    q.clear();
    if(!db.commit()) db.rollback();
    db.close();
  }
}

void MainWindow::saveLayout()
{
  QString fname(trUtf8("%1/%2").arg(home).arg(APP_CONF));
  QFile file(fname);
  if(!file.open(QFile::WriteOnly)){
    QString msg(trUtf8("レイアウトファイルをオープン出来ません(W) %1\n%2")
      .arg(fname).arg(file.errorString()));
    QMessageBox::warning(this, trUtf8(APP_NAME), msg);
    return;
  }
  QByteArray geo = saveGeometry();
  QByteArray lay = saveState();
  bool ok = file.putChar((uchar)geo.size());
  if(ok) ok = file.write(geo) == geo.size();
  if(ok) ok = file.write(lay) == lay.size();
  if(!ok){
    QString msg(trUtf8("レイアウトファイルの書き込みに失敗しました %1\n%2")
      .arg(fname).arg(file.errorString()));
    QMessageBox::warning(this, trUtf8(APP_NAME), msg);
    return;
  }
}

void MainWindow::loadLayout()
{
  QString fname(trUtf8("%1/%2").arg(home).arg(APP_CONF));
  if(!QFileInfo(fname).exists()) return;
  QFile file(fname);
  if(!file.open(QFile::ReadOnly)){
    QString msg(trUtf8("レイアウトファイルをオープン出来ません(R) %1\n%2")
      .arg(fname).arg(file.errorString()));
    QMessageBox::warning(this, trUtf8(APP_NAME), msg);
    return;
  }
  uchar geo_size;
  QByteArray geo;
  QByteArray lay;
  bool ok = file.getChar((char *)&geo_size);
  if(ok){
    geo = file.read(geo_size);
    ok = geo.size() == geo_size;
  }
  if(ok){
    lay = file.readAll();
    ok = lay.size() > 0;
  }
  if(ok) ok = restoreGeometry(geo);
  if(ok) ok = restoreState(lay);
  if(!ok){
    QString msg(trUtf8("レイアウトファイルの読み出しに失敗しました %1")
      .arg(fname));
    QMessageBox::warning(this, trUtf8(APP_NAME), msg);
    return;
  }
}

void MainWindow::closeEvent(QCloseEvent *ce)
{
  if(mTrayIcon->isVisible()){
    hide();
    ce->ignore();
  }
}

void MainWindow::drawXORrect(ulong w)
{
  RECT r;
  BOOL result = GetClientRect((HWND)w, &r);
  if(result){
    HDC dc = GetDC((HWND)w);
    HPEN pen = (HPEN)CreatePen(PS_SOLID, 8, RGB(255, 0, 0));
    HPEN op = (HPEN)SelectObject(dc, pen);
    // HBRUSH br = (HBRUSH)CreateHatchBrush(HS_BDIAGONAL, RGB(0, 0, 0));
    HBRUSH br = (HBRUSH)CreateSolidBrush(RGB(15, 15, 15)); // xor
    HBRUSH ob = (HBRUSH)SelectObject(dc, br);
    int om = SetBkMode(dc, TRANSPARENT);
    int orop = SetROP2(dc, R2_XORPEN);
    Rectangle(dc, r.left, r.top, r.right, r.bottom);
    SetROP2(dc, orop);
    SetBkMode(dc, om);
    SelectObject(dc, ob);
    DeleteObject(br);
    SelectObject(dc, op);
    DeleteObject(pen);
    ReleaseDC((HWND)w, dc);
  }
}

int MainWindow::cmpWindowName(char *buf)
{
  static char *exclude[] = {APP_NAME, "msime", "Program Manager"};
  for(size_t i = 0; i < sizeof(exclude) / sizeof(exclude[0]); i++)
    if(!strncmp(buf, exclude[i], strlen(exclude[i]))) return 0;
  return 1;
}

void MainWindow::mouseMoveEvent(QMouseEvent *ev)
{
  QPoint p = ev->globalPos();
  // cout << p.x() << ", " << p.y() << endl;
  HWND w = GetTopWindow(GetDesktopWindow());
  while(w){
    if(IsWindowVisible(w)){
      RECT r;
      BOOL result = GetWindowRect(w, &r);
      if(result
      && QRect(r.left, r.top, r.right - r.left, r.bottom - r.top).contains(p)){
        if(w == (HWND)prev_window) break;
        if(prev_window) drawXORrect(prev_window);
        char buf[1024];
        if(GetWindowTextA(w, buf, sizeof(buf))){
          if(!cmpWindowName(buf)){
            prev_window = hwnd = 0;
            mHANDLE->clear();
            break;
          }
          drawXORrect(prev_window = hwnd = (ulong)w);
          ostringstream oss;
          oss << setw(8) << setfill('0') << hex << right << hwnd;
          oss << " w[" << buf << "]";
          char cls[1024];
          if(GetClassNameA(w, cls, sizeof(cls))){
            oss << " c[" << cls << "]";
          }
          mHANDLE->setText(QString(oss.str().c_str()));
        }
        break;
      }
    }
    w = GetWindow(w, GW_HWNDNEXT);
  }
}

void MainWindow::mousePressEvent(QMouseEvent *ev)
{
  // if(ev->button() == Qt::RightButton) releaseMouse();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *ev)
{
  if(prev_window){
    drawXORrect(prev_window);
    prev_window = 0;
  }
}

void MainWindow::createActions()
{
  mChaseAction = new QAction(QIcon(":/qrc/icon_chase"),
    trUtf8("追尾(&G)"), this);
  mChaseAction->setShortcut(tr("Ctrl+G"));
  mChaseAction->setStatusTip(trUtf8("Chase window HANDLE"));
  connect(mChaseAction, SIGNAL(triggered()), this, SLOT(chase()));

  mMinimizeAction = new QAction(QIcon(":/qrc/icon_minimize"),
    trUtf8("最小化(&N)"), this);
  mMinimizeAction->setShortcut(tr("Ctrl+N"));
  mMinimizeAction->setStatusTip(trUtf8("Minimize window"));
  connect(mMinimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

  mMaximizeAction = new QAction(QIcon(":/qrc/icon_maximize"),
    trUtf8("最大化(&X)"), this);
  mMaximizeAction->setShortcut(tr("Ctrl+X"));
  mMaximizeAction->setStatusTip(trUtf8("Maximize window"));
  connect(mMaximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

  mRestoreAction = new QAction(QIcon(":/qrc/icon_restore"),
    trUtf8("復元(&R)"), this);
  mRestoreAction->setShortcut(tr("Ctrl+R"));
  mRestoreAction->setStatusTip(trUtf8("Restore window"));
  connect(mRestoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

  mQuitAction = new QAction(QIcon(":/qrc/icon_exit"),
    trUtf8("終了(&Q)"), this);
  mQuitAction->setShortcut(tr("Ctrl+Q"));
  mQuitAction->setStatusTip(trUtf8("Quit the application"));
  connect(mQuitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void MainWindow::createMenus()
{
  mFileMenu = menuBar()->addMenu(trUtf8("ファイル(&F)"));
  mFileMenu->addAction(mChaseAction);
  mFileMenu->addSeparator();
  mFileMenu->addAction(mQuitAction);
  mViewMenu = menuBar()->addMenu(trUtf8("表示(&V)"));
}

void MainWindow::createToolBar()
{
  mFileToolBar = addToolBar(trUtf8("ファイル"));
  mFileToolBar->setObjectName(tr("File"));
  mFileToolBar->addAction(mChaseAction);
  mFileToolBar->addSeparator();
  mFileToolBar->addAction(mQuitAction);
}

void MainWindow::createStatusBar()
{
  statusBar()->showMessage(trUtf8("完了"));
}

void MainWindow::createDockWindows()
{
  QDockWidget *dockT1 = new QDockWidget(trUtf8("HANDLE"), this);
  dockT1->setObjectName(tr("HANDLE"));
  dockT1->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
  QWidget *wT1 = new QWidget();
  QHBoxLayout *hbT1 = new QHBoxLayout();
  mHANDLE = new QLineEdit(trUtf8("drag&drop image to target window"), dockT1);
  hbT1->addWidget(mHANDLE);
  QPushButton *btnDl = new QPushButton(trUtf8("追尾(&G)"), dockT1);
  connect(btnDl, SIGNAL(clicked()), this, SLOT(chase()));
  hbT1->addWidget(btnDl);
  wT1->setLayout(hbT1);
  dockT1->setWidget(wT1);
  addDockWidget(Qt::TopDockWidgetArea, dockT1);
  mViewMenu->addAction(dockT1->toggleViewAction());

  QDockWidget *dockL1 = new QDockWidget(trUtf8("データ"), this);
  dockL1->setObjectName(tr("Data"));
  dockL1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  QWidget *wL1 = new QWidget();
  QVBoxLayout *vbL1 = new QVBoxLayout();
  mText = new QTextEdit(trUtf8("データ"), dockL1);
  vbL1->addWidget(mText);
  cw = new ChaserWidget(dockL1);
  cw->setMinimumSize(QSize(160, 320));
  vbL1->addWidget(cw);
  wL1->setLayout(vbL1);
  dockL1->setWidget(wL1);
  addDockWidget(Qt::LeftDockWidgetArea, dockL1);
  mViewMenu->addAction(dockL1->toggleViewAction());

  QDockWidget *dockR1 = new QDockWidget(trUtf8("操作"), this);
  dockR1->setObjectName(tr("Buttons"));
  dockR1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  QWidget *wR1 = new QWidget();
  QVBoxLayout *vbR1 = new QVBoxLayout();
  QPushButton *btnTray = new QPushButton(trUtf8("トレイへ(&T)"), dockR1);
  connect(btnTray, SIGNAL(clicked()), this, SLOT(hide()));
  vbR1->addWidget(btnTray);
  QPushButton *btnQuit = new QPushButton(trUtf8("終了(&Q)"), dockR1);
  connect(btnQuit, SIGNAL(clicked()), qApp, SLOT(quit()));
  vbR1->addWidget(btnQuit);
  wR1->setLayout(vbR1);
  dockR1->setWidget(wR1);
  addDockWidget(Qt::RightDockWidgetArea, dockR1);
  mViewMenu->addAction(dockR1->toggleViewAction());
}

void MainWindow::createTrayIcon()
{
  mTrayIconMenu = new QMenu(this);
  mTrayIconMenu->addAction(mMinimizeAction);
  mTrayIconMenu->addAction(mMaximizeAction);
  mTrayIconMenu->addAction(mRestoreAction);
  mTrayIconMenu->addSeparator();
  mTrayIconMenu->addAction(mQuitAction);

  mTrayIcon = new QSystemTrayIcon(this);
  mTrayIcon->setContextMenu(mTrayIconMenu);
  connect(mTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
    this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch(reason){
  case QSystemTrayIcon::Trigger:
  case QSystemTrayIcon::DoubleClick:
    ;
    break;
  case QSystemTrayIcon::MiddleClick:
    ;
    break;
  default:
    ;
  }
}

void MainWindow::treeActivated(const QModelIndex &idx)
{
  if(mModel->isDir(idx)) return;
  QString path(mModel->filePath(idx));
  if(QFileInfo(path).suffix().toLower() != "png") return;
  mText->setPlainText(path);
  QImage img(path);
  if(img.isNull()){
    qDebug("PNG is null: %s", path.toUtf8().constData());
  }else{
    QImage img2 = img.convertToFormat(QImage::Format_ARGB32);
    for(int y = 0; y < img2.height(); y++){
      QRgb *row = (QRgb *)img2.scanLine(y);
      for(int x = 0; x < img2.width(); x++){
        ((uchar *)&row[x])[3] = 0x30;
      }
    }
    cw->pixmap = QPixmap::fromImage(img2);
    cw->setUpdatesEnabled(true);
    cw->update(); // cw->repaint();
  }
}

void MainWindow::chase()
{
  // grabMouse(QCursor(QIcon(APP_ICON).pixmap(32, 32)));

  QString handle(mHANDLE->text());

  qDebug("HANDLE: %s", mHANDLE->text().toUtf8().constData());
  bgPNG *bp123 = new bgPNG(123);
  connect(bp123, SIGNAL(done()), this, SLOT(fin()));
  bp123->get(handle);

  qDebug("HANDLE: %s", mHANDLE->text().toUtf8().constData());
  bgPNG *bp456 = new bgPNG(456);
  connect(bp456, SIGNAL(done()), this, SLOT(fin()));
  QTextCodec *jp = QTextCodec::codecForName("utf-8");
  bp456->getb(jp->fromUnicode(handle));
  // delete jp;
}

void MainWindow::fin()
{
  bgPNG *bp = qobject_cast<bgPNG *>(sender());
  qDebug("num: %d", bp->getnum());

  if(bp->getstat() == 0){
    // エラー時はとりあえず utf-8 でデコード
    QTextCodec *jp = QTextCodec::codecForName("utf-8");
    QString txt = jp->toUnicode(bp->getdat());
    qDebug("error: %s", txt.toUtf8().constData());
    mText->setPlainText(txt);
  }else{
    // 正常時も utf-8 決め打ち
    QTextCodec *jp = QTextCodec::codecForName("utf-8");
    QString txt = jp->toUnicode(bp->getdat());
    qDebug("result: %s", txt.toUtf8().constData());
    mText->setPlainText(txt);

    QStringList lst = txt.split(QChar('\n'));
    QRegExp re(QString("ttp:[^\\s\"'<>]+"), Qt::CaseInsensitive);
    for(QStringList::Iterator i = lst.begin(); i != lst.end(); ++i){
      // if((*i).contains("error")) continue;
      int p = 0;
      while((p = re.indexIn(*i, p)) >= 0){
        quelst.enqueue((*i).mid(p, re.matchedLength()));
        p += re.matchedLength();
      }
    }
  }
  delete bp;
}

void MainWindow::cleanupcode()
{
  qDebug("running clean up code...");
  th->stop();
  while(!th->isFinished()){}
  saveLayout();
  qDebug("application is cleaned up.");
}

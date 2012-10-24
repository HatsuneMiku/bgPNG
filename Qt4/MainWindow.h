/*
  MainWindow.h
*/

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QtGui>
#include <QApplication>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QtSql>
#include <QFileInfo>
#include <QDirModel>
#include <QTreeView>
#include <QLineEdit>
#include <QTextEdit>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstring>

#include "bgPNG.h"
#include "ChaseThread.h"

#define SAMPLE_NAME "claudia.png"
#define SAMPLE_IMG ":/qrc/img_sample"

#define APP_NAME "bgPNG"
#define APP_ICON ":/qrc/icon_bgPNG"
#define APP_CONF APP_NAME".qtd"
#define APP_DATA APP_NAME".sl3"
#define APP_DB "QSQLITE"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QQueue<QString> &q, QWidget *parent=0, Qt::WindowFlags flags=0);
  virtual ~MainWindow(){}
  void proc();

protected:
  void saveLayout();
  void loadLayout();
  void closeEvent(QCloseEvent *ce);

  void drawXORrect(ulong w);
  int cmpWindowName(char *buf);
  void mouseMoveEvent(QMouseEvent *ev);
  void mousePressEvent(QMouseEvent *ev);
  void mouseReleaseEvent(QMouseEvent *ev);
  void paintEvent(QPaintEvent *ev);

private:
  void createActions();
  void createMenus();
  void createToolBar();
  void createStatusBar();
  void createDockWindows();
  void createTrayIcon();

signals:
  void toBeAbort();

public slots:
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void treeActivated(const QModelIndex &idx);
  void chase();
  void fin();
  void cleanupcode();

private:
  QPixmap pixmap;
  ulong hwnd, prev_window;
  ChaseThread *th;
  QQueue<QString> &quelst;
  QSqlDatabase db;
  QString home;

  QAction *mChaseAction;
  QMenu *mFileMenu;
  QMenu *mViewMenu;
  QToolBar *mFileToolBar;
  QLineEdit *mHANDLE;
  QTextEdit *mText;
  QWidget *mWidget;
  QDirModel *mModel;
  QTreeView *mTree;

  QAction *mMinimizeAction;
  QAction *mMaximizeAction;
  QAction *mRestoreAction;
  QAction *mQuitAction;
  QSystemTrayIcon *mTrayIcon;
  QMenu *mTrayIconMenu;
};

#endif // __MAINWINDOW_H__

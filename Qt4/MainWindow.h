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
#include <iostream>
#include <cstring>

#include "bgPNG.h"
#include "ChaseThread.h"

#define APP_NAME "bgPNG"
#define APP_ICON ":/qrc/icon_bgPNG"
#define APP_CONF APP_NAME".qtd"
#define APP_DATA APP_NAME".sl3"
#define APP_DB "QSQLITE"
#define SAMPLE_IMG "../resource/claudia.png"

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
  void chase();
  void fin();
  void cleanupcode();

private:
  ulong prev_window;
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

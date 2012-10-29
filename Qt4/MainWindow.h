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
#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QLineEdit>
#include <QTextEdit>

#include "bgPNG.h"
#include "ChaseThread.h"
#include "ChaserWidget.h"

#define SAMPLE_NAME "claudia.png"
#define SAMPLE_IMG ":/qrc/img_sample"
#define SAMPLE_NAME_JP "窓辺.png"
#define SAMPLE_IMG_JP ":/qrc/img_sample_jp"

#define APP_NAME "bgPNG"
#define APP_ICON ":/qrc/icon_bgPNG"
#define APP_CONF APP_NAME".qtd"
#define APP_DATA APP_NAME".sl3"
#define APP_DB "QSQLITE"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QQueue<QString> &q, QWidget *parent=0, Qt::WindowFlags flags=0);
  virtual ~MainWindow();

protected:
  void saveLayout();
  void loadLayout();
  void closeEvent(QCloseEvent *ce);

private:
  void createActions();
  void createMenus();
  void createToolBar();
  void createStatusBar();
  void createDockWindows();
  void createTrayIcon();

signals:
  void toBeAbort();
  void stop();
  void quit();

public slots:
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void treeActivated(const QModelIndex &idx);
  void listActivated(const QModelIndex &idx);
  QImage customRGBA(const QImage &img);
  void hover(ulong hwnd);
  void chase(ulong hwnd=0);
  void proc();
  void fin();
  void cleanupcode();

private:
  ChaserWidget *cw;
  ChaseThread *ct;
  QThread *th;
  QQueue<QString> &quelst;
  QSqlDatabase db;
  QString home;

  QAction *mChaseAction;
  QMenu *mFileMenu;
  QMenu *mViewMenu;
  QToolBar *mFileToolBar;
  QLineEdit *mHANDLE;
  QTextEdit *mText;
  QFileSystemModel *mDirModel;
  QTreeView *mTree;
  QFileSystemModel *mFileModel;
  QListView *mList;

  QAction *mMinimizeAction;
  QAction *mMaximizeAction;
  QAction *mRestoreAction;
  QAction *mQuitAction;
  QSystemTrayIcon *mTrayIcon;
  QMenu *mTrayIconMenu;
};

#endif // __MAINWINDOW_H__

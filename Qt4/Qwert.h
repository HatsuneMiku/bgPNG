/*
  Qwert.h
*/

/*
  2010-10-11 by KernelCoder
  A Cross-platform Faster File System Model than QFileSystemModel
  http://kernelcoder.wordpress.com/2010/10/11/a-cross-platform-faster-file-system-model-than-qfilesystemmodel/

  Background:
   You know that,
   QDirModel is obsolete and Qt recommend to use QFileSystemModel.
   However, QFileSystemModel is not fast enough as windows explorer.
   Here I will create an explorer as fast as windows explorer.

  Model Related Classes:
   FileSystemItem: This class represent an item (dir/file/folder) in the model.

  4 Responses:
   on 2011-05-06 11:37 | Vitaly Volkov
    Hi!
    You have a compilation error on line 233 : return index(path);
    function index() should take 2 parameters.

   on 2011-05-06 11:38 | Vitaly Volkov
    In FileSystemModel class

   on 2011-09-01 14:22 | Simon
    Great work! For a network share with 20.000+ files,
    this is three times faster than QFileSystemModel.

   on 2011-10-14 10:09 | Dennis Lang
    Nice job.
    I made a few changes so I could use it as a drop-in replacement
    when attached to a QTreeView, such as having childCount(),
    force a call to populate. I also added support for the nameFilters.

    I donÅft know if it is any faster than QFileSystemModel,
    but it is still nice to see the code for a model.
    My biggest problem with QFileSystemModel is there is no easy way to tell it
    to skip removal drives or network shared drives.
    Your version suffers the same problem. It looks like there is no way
    to inventory the drives and tell if they are 'local' and not removable.
    The QT Mobil API has a SystemStorage class which does have the info,
    but not the PC API.
*/

#ifndef __QWERT_H__
#define __QWERT_H__

#include <QtCore>
#include <QtGui>
#include <QDateTime>
#include <QDir>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QSortFilterProxyModel>
#include <QTableView>

#define QWERT_SEPARATOR QString("/")

class Qwert : public QObject {
  Q_OBJECT

public:
  Qwert(){}
  virtual ~Qwert(){}
  static QStringList getLogicalDriveStrings(void);
  static QStringList getLocalDrives(void);
};

class QwertFileSystemItem { // This class represents a row in Model
public:
  QwertFileSystemItem(const QFileInfo &fileInfo, QwertFileSystemItem *parent=0)
    : mFileInfo(fileInfo), mParent(parent) {
    if(parent){
      parent->addChild(this);
      // NOTE: absoluteFilePath method of QFileInfo class cause
      // the file system query hence causes slower performance,
      // We are going to keep the absolutepath of an item.
      // Absolute path means the absolute path of parent
      // plus a separator plus filename of this item.
      // (for drives, there is no filename, so we used canonicalPath)
      mAbsFilePath = parent->parent() == 0 ? fileInfo.canonicalPath() :
        parent->absoluteFilePath() + QWERT_SEPARATOR + fileInfo.fileName();
    }else{ mAbsFilePath = ""; } // Path of root item which is not visible
  }
  virtual ~QwertFileSystemItem(){ qDeleteAll(mChildren); }
  QwertFileSystemItem *childAt(int position){
    return mChildren.value(position, 0);
  }
  int childCount() const { return mChildren.count(); }
  // returns the position of this son among his siblings
  int childNumber() const {
    return mParent ?
      mParent->mChildren.indexOf(const_cast<QwertFileSystemItem *>(this)) : 0;
  }
  QwertFileSystemItem *parent(){ return mParent; }
  QString absoluteFilePath() const { return mAbsFilePath; }
  // (for drives, there is no filename, so we used canonicalPath)
  QString fileName() const {
    return !mParent ? "" : (mParent->parent() == 0 ?
      mFileInfo.canonicalPath() : mFileInfo.fileName());
  }
  QFileInfo fileInfo() const { return mFileInfo; }
  void addChild(QwertFileSystemItem *child){
    if(!mChildren.contains(child)) mChildren.append(child);
  }
  // This is a recursive method which tries to match a path to a specifiq
  // QwertFileSystemItem item which has the path;
  // Here startIndex is the position of the separator
  QwertFileSystemItem *matchPath(const QStringList &path, int startIndex=0){
    foreach(QwertFileSystemItem *child, mChildren){
      QString match = path.at(startIndex);
      if(child->fileName() == match)
        if(startIndex + 1 == path.count()) return child;
        else  return child->matchPath(path, startIndex + 1);
    }
    return 0;
  }

private:
  const QFileInfo &mFileInfo;
  QwertFileSystemItem *mParent;
  QList<QwertFileSystemItem *> mChildren;
  QString mAbsFilePath;
};

class QwertFileSystemModel : public QAbstractItemModel {
  Q_OBJECT

public:
  enum Column { NAME, SIZE, TYPE, DATE, LASTCOLUMN };
  QwertFileSystemModel(QObject *parent) :
    QAbstractItemModel(parent), mIconFactory(new QFileIconProvider()){
    // Now it is time to fix the headers
    mHeaders << "Name" << "Size" << "Type" << "Date Modified";
    // Create the root item
    // [NOTE: Root item is not visible, but is the parent of all drives]
    mRootItem = new QwertFileSystemItem(QFileInfo(), 0);
    mCurrentPath = "";
    QFileInfoList drives = QDir::drives();
    foreach(QFileInfo drive, drives)
      new QwertFileSystemItem(drive, mRootItem);
  }
  virtual ~QwertFileSystemModel(){
    delete mRootItem;
    delete mIconFactory;
  }
  QVariant headerData(int section, Qt::Orientation orientation, int role) const
  {
    if(orientation == Qt::Horizontal)
      switch(role){
      // in case of DisplayRole, just return the header text
      case Qt::DisplayRole: return mHeaders.at(section);
      // in case of TextAlignmentRole, only SIZE column will be right align,
      // others will be left align
      case Qt::TextAlignmentRole:
        return int(SIZE) == section ? Qt::AlignRight : Qt::AlignLeft;
      }
    return QVariant();
  }
  Qt::ItemFlags flags(const QModelIndex &index) const {
    if(!index.isValid()) return 0;
    // Our model is read only.
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  int columnCount(const QModelIndex &parent) const { return LASTCOLUMN; } // 4
  int rowCount(const QModelIndex &parent) const {
    QwertFileSystemItem *parentItem = getItem(parent);
    return parentItem->childCount();
  }
  QVariant data(const QModelIndex &index, int role) const {
    // invalid work, invalid return
    if(!index.isValid()) return QVariant();
    // in case of TextAlignmentRole, only SIZE column will be right align,
    // others will be left align
    if(int(SIZE) == index.column() && Qt::TextAlignmentRole == role)
      return Qt::AlignRight;
    // At present,
    // I don't want pay attention other than DisplayRole & DecorationRole
    if(role != Qt::DisplayRole && role != Qt::DecorationRole)
      return QVariant();
    QwertFileSystemItem *item = getItem(index);
    if(!item) return QVariant();
    // if the role is for decoration & column is zero, we send the item's icon
    if(role == Qt::DecorationRole && index.column() == int(NAME))
      return mIconFactory->icon(item->fileInfo());
    QString data;
    Column col = Column(index.column());
    switch(col){
    case NAME: data = item->fileName(); break;
    case SIZE:
      data = item->fileInfo().isDir() ? "" : // we don't want to show zero.
        QString::number(item->fileInfo().size()); break;
    case TYPE: data = mIconFactory->type(item->fileInfo()); break;
    case DATE:
      data = item->fileInfo().lastModified().toString(Qt::LocalDate); break;
    default: data = ""; break;
    }
    return data;
  }
  QModelIndex index(int row, int column, const QModelIndex &parent) const {
    // As the NAME column is a tree,
    // we will only create index which parent is NAME column
    if(parent.isValid() && parent.column() != int(NAME)) return QModelIndex();
    QwertFileSystemItem *parentItem = getItem(parent);
    // if there is a parent index, we want to work
    if(parentItem){
      QwertFileSystemItem *childItem = parentItem->childAt(row);
      if(childItem) return createIndex(row, column, childItem);
    }
    return QModelIndex();
  }
  // *** must check that default value column=0 is correct ***
  QModelIndex index(const QString &path, int column=0) const {
    if(path.length() > 0){
      QwertFileSystemItem *item = mRootItem->matchPath(
        path.split(QWERT_SEPARATOR));
      if(item) return createIndex(item->childNumber(), column, item);
    }
    return QModelIndex();
  }
  QModelIndex parent(const QModelIndex &index) const {
    // invalid work, invalid return
    if(!index.isValid()) return QModelIndex();
    QwertFileSystemItem *childItem = getItem(index);
    // If there is no child, there is no index
    if(!childItem) return QModelIndex();
    QwertFileSystemItem *parentItem = childItem->parent();
    // if there is no parent or parent is invisible, there is no index
    if(!parentItem || parentItem == mRootItem) return QModelIndex();
    return createIndex(parentItem->childNumber(), NAME, parentItem);
  }
  bool isDir(const QModelIndex &index){
    QwertFileSystemItem *item = static_cast<QwertFileSystemItem*>(
      index.internalPointer());
    if(item && item != mRootItem) return item->fileInfo().isDir();
    return false;
  }
  // NOTE: absoluteFilePath method of QFileInfo class cause
  // the file system query hence causes slower performance,
  // We are going to keep the absolutepath of an item.
  // Absolute path means the absolute path of parent
  // plus a separator plus filename of this item.
  QString absolutePath(const QModelIndex &index){
    QwertFileSystemItem *item = static_cast<QwertFileSystemItem*>(
      index.internalPointer());
    if(item && item != mRootItem) return item->absoluteFilePath();
    return "";
  }
  QString currentPath() const { return mCurrentPath; }
  QModelIndex setCurrentPath(const QString &path){
    mCurrentPath = path;
    // find the file system item
    QwertFileSystemItem *item = mRootItem->matchPath(
      path.split(QWERT_SEPARATOR));
    // if there is a item and item's child is zero, we are going to find the
    // entries in the directory
    if(item && item != mRootItem && !item->childCount()) populateItem(item);
    return index(path);
  }

private:
  void populateItem(QwertFileSystemItem *item){
    QDir dir(item->absoluteFilePath());
    QFileInfoList all = dir.entryInfoList(
      QDir::AllEntries|QDir::NoDotAndDotDot);
    // loop through all the item and construct the childs
    foreach(QFileInfo one, all) new QwertFileSystemItem(one, item);
  }
  QwertFileSystemItem *getItem(const QModelIndex &index) const {
    // just return the internal pointer we set at creating index
    // if the index is valid
    if(index.isValid()){
      QwertFileSystemItem *item = static_cast<QwertFileSystemItem*>(
        index.internalPointer());
      if(item) return item;
    }
    return mRootItem;
  }

private:
  QwertFileSystemItem *mRootItem;
  QString mCurrentPath;
  QStringList mHeaders;
  QFileIconProvider *mIconFactory;
};

class QwertFileSystemSortProxyModel : public QSortFilterProxyModel {
  Q_OBJECT

public:
  QwertFileSystemSortProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent) {}

protected:
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const {
    QwertFileSystemModel* fsModel = dynamic_cast<QwertFileSystemModel*>(
      sourceModel());
    // To keep the the folder always on the up, we need to do the followings
    // while the compare indice are folder & file exclusively.
    if(fsModel)
      if((fsModel->isDir(left) && !fsModel->isDir(right)))
        return sortOrder() == Qt::AscendingOrder;
      else if((!fsModel->isDir(left) && fsModel->isDir(right)))
        return sortOrder() == Qt::DescendingOrder;
    return QSortFilterProxyModel::lessThan(left, right);
  }
};

class QwertDirBrowser : public QTableView {
  Q_OBJECT

public:
  QwertDirBrowser(QWidget *parent) : QTableView(parent) {
    grabKeyboard();
    /* IMPORTANT
      Construct the QFileSystemModel. This is relatively faster that QDirModel.
      ****** TODO ******
      However, QFileSystemModel is not fast enough as windows system explorer.
      We have to consider platform specifiq API in this case,
      like FindFirstFile or FindFirstFileEx with looping FindNextFile.
      In this case we have to completely make a View class
      to show contents on this.
      I have worked with this APIs at past, yet need some R&D's on this.
      In this case, there are challenges like finding the item's icon
      and item's text color.
    */
    // mFSModel = new QFileSystemModel(this);
    mFSModel = new QwertFileSystemModel(this);
    // This will populate the filesystem
    // mFSModel->setRootPath(QDir::currentPath());
    mSortModel = new QwertFileSystemSortProxyModel(this);
    mSortModel->setSourceModel(mFSModel);
    this->setShowGrid(false);
    this->verticalHeader()->hide();
    this->setModel(mSortModel);
    this->setSortingEnabled(true);
    /*
      Now, binds the necessary signals & corresponding slots.
      One thing ro remember is that, we are the setting the signal
      ConnectionType to AutoConnection (default). So if a signal is emitted
      in another thread, yet the slot will run on invoked objects thread.
    */
    // Binds the doubleClicked signal to handleDirItemDoubleClicked slot
    // of this class
    connect(this, SIGNAL(doubleClicked(QModelIndex)),
      SLOT(handleDirItemDoubleClicked(QModelIndex)));
  }

private slots:
  void handleDirItemDoubleClicked(const QModelIndex &index){
    if(mFSModel->isDir(mSortModel->mapToSource(index)))
      updateCurrentPath(index);
  }

protected:
  void keyPressEvent(QKeyEvent *e){
    // if bakcspace key is pressed,
    // view the dir browser to updates parent folder
    if(e->key() == Qt::Key_Backspace){
      // If parent model-index is valid, up one level;
      // Otherwise back to topmost level
      if(this->rootIndex().parent().isValid())
        updateCurrentPath(this->rootIndex().parent());
      else
        this->setRootIndex(
          mSortModel->mapFromSource(mFSModel->setCurrentPath("")));
    }
  }

private:
  void updateCurrentPath(const QModelIndex &index){
    if(index.isValid()){
      // update the model for current path
      mFSModel->setCurrentPath(
        mFSModel->absolutePath(mSortModel->mapToSource(index)));
      // update the view as necessarily
      this->setRootIndex(
        mSortModel->mapFromSource(mFSModel->index(mFSModel->currentPath())));
    }
  }

private:
  QwertFileSystemSortProxyModel *mSortModel;
  QwertFileSystemModel *mFSModel;
};

#endif // __QWERT_H__

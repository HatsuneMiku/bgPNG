/*
  Qwert.h
*/

#ifndef __QWERT_H__
#define __QWERT_H__

#include <QtCore>

class Qwert : public QObject {
  Q_OBJECT

public:
  Qwert(){}
  virtual ~Qwert(){}
  static QStringList getLogicalDriveStrings(void);
  static QStringList getLocalDrives(void);
};

#endif // __QWERT_H__

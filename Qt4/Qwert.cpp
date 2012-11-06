/*
  Qwert.cpp
*/

#include "Qwert.h"
#include <windows.h>
#include <cstring>

using namespace std;

QStringList Qwert::getLogicalDriveStrings(void)
{
  QStringList result;
  char szLogicalDrives[256];
  DWORD dwBuffSize = sizeof(szLogicalDrives) / sizeof(szLogicalDrives[0]);
  if(GetLogicalDriveStringsA(dwBuffSize, szLogicalDrives) > 0)
    for(char *p = szLogicalDrives; *p != '\0'; p += strlen(p) + 1) result << p;
  return result;
}

QStringList Qwert::getLocalDrives(void)
{
  QStringList result;
  foreach(QString s, getLogicalDriveStrings()){
    switch(GetDriveTypeA(s.toStdString().c_str())){
    case DRIVE_UNKNOWN: break;
    case DRIVE_NO_ROOT_DIR: break;
    case DRIVE_REMOVABLE: result << s; break;
    case DRIVE_FIXED: result << s; break;
    case DRIVE_REMOTE: break;
    case DRIVE_CDROM: result << s; break;
    case DRIVE_RAMDISK: result << s; break;
    default: break;
    }
  }
  return result;
}

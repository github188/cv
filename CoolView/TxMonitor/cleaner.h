#ifndef CLEANER_H
#define CLEANER_H

#include "baseloopjob.h"

#include <QString>
#include <memory>

#ifndef ULONGLONG
#define ULONGLONG unsigned __int64
#endif

class IRecordDB;

//���ڼ��ʣ����̿ռ䣬����ʱɾ�����ļ�
class Cleaner : public BaseLoopJob
{
  Q_OBJECT

public:
  Cleaner();
  ~Cleaner();

  void Init();

signals:
  void NotifyFreeDiskSpaceChangeSignal(quint64 space);

protected:
  void RunOnceAtStart(); // ����ʱִ��һ�ε�����
  void Run();

  ULONGLONG GetPathFreeSpace(QString path);

  // DeleteFiles
  void DeleteFiles(QString path, QString mark, QString exclude, 
    ULONGLONG size = 0, bool recursive = true);
  void SetFileDeletedFlag(QString file);
  void DeleteEmptyFolders();

private:

  ULONGLONG _lowSpaceThreshold;    // ��������ֵ(Byte),���ڴ�ֵ��ʼɾ���ļ�
  ULONGLONG _highSpaceThreshold;    // ����������ֵ(Byte),���ڴ�ֵ��ֹͣɾ���ļ�

  QString _recPath;
  QString _uploadPath;
  QString _uploadMark;
  QString _uploadDirMark;
  QString _appPath;
  
  std::shared_ptr<IRecordDB> db_;
};

#endif // CLEANER_H

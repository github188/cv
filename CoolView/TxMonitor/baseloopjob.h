#ifndef BASELOOPJOB_H
#define BASELOOPJOB_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include "log.h"

class BaseLoopJob : public QObject
{
  Q_OBJECT

public:
  BaseLoopJob(const char * name = NULL, const int msec = 10000, QObject *parent = NULL);
  ~BaseLoopJob();

  //���ش˷���ʵ�ֶ����ʼ��������������û���˷���
  virtual void Init();
  //��ʼѭ��������һ�㲻��Ҫ����
  virtual void Start();
  //ֹͣѭ��������һ�㲻��Ҫ����
  virtual void Stop();

protected:
  //���ش˷���ʵ��ÿ��ѭ����Ҫִ�еĹ���
  virtual void Run() = 0;

  // signals and slots
signals:
  void StartJobSignal();
  void StopJobSignal();

protected slots:
  //QTimerͨ���˷�����ʱ����Run��һ�㲻��Ҫ����
  virtual void Timeout();


  // members
protected:
  const char * _jobName;
  int _interval;

  QTimer  _timer;
  QThread _thread;

  ILogger *_logger;
  
};

#endif // BASELOOPJOB_H

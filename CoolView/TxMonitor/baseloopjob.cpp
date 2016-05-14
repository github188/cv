#include "baseloopjob.h"

#include "Log.h"

BaseLoopJob::BaseLoopJob(const char * name, const int msec, QObject *parent)
  : QObject(parent)
  , _jobName(name)
  , _interval(msec)
  , _logger(nullptr)
{
}

BaseLoopJob::~BaseLoopJob()
{

}

void BaseLoopJob::Timeout()
{
  LOG_PRINTF_EX(_logger, "Running %s in thread %d...", 
    _jobName, QThread::currentThreadId());
  Run();
}

void BaseLoopJob::Init()
{
  moveToThread(&_thread);

  _logger = CreateLogger();
  _logger->SetModuleName(_jobName);

  _timer.setInterval(_interval);
  _timer.moveToThread(&_thread);

  QObject::connect(&_timer, &QTimer::timeout, 
    this, &BaseLoopJob::Timeout);
  QObject::connect(&_thread, SIGNAL(started()), 
    &_timer, SLOT(start())); // ����ͨ���źŲ�������ʱ������Ϊ�����߳��е���Start����ʱ���������ڲ��߳�
  QObject::connect(&_thread, SIGNAL(finished()), 
    &_timer, SLOT(stop()));
}

void BaseLoopJob::Start()
{
  _thread.start(); // ���������߳�
  emit StartJobSignal();
}

void BaseLoopJob::Stop()
{
  emit StopJobSignal();
  _thread.quit();
}

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "BaseLoopJob.h"
#include "scheduler_types.h"


class WorkerService;
struct WeblibSettingInfo;

//�����������ڼ������״��������ͣ�Ϳ�����Ӧ������ȷ��TX��������
//Ŀǰֻ�������ϴ�����
class Scheduler : public BaseLoopJob
{
  Q_OBJECT

public:
  Scheduler();
  ~Scheduler();

  virtual void Init();

signals:
  //֪ͨ������������Ϣ
  void NotifyNetworkScheduleSignal(
    const NetworkInfo &info,
    quint64 weblib_speed_limit);

  // TODO:Weblib����������һ�������������
  void RequestResetWeblibUploader(
    const WeblibSettingInfo &setting);
  void RequestStartWeblibUploader();
  void RequestStopWeblibUploader();

protected:
  virtual void Run();

private:
  //ˢ����������Ϣ
  void RefreshAdapterInfo();
  //��ȡ�ӿ�����������Ϣ��info������ʹ���������Ҳ���������
  bool GetInterfaceInfo(NetworkInfo &info);
  //�������
  void ScheduleNetwork();

private:
  NetworkInfo _adapterInfo;
  WorkerService *_uploadWorkService; // ע��ָ��Ķ������������߳�
    // ���̵߳���Ӧ��ʹ���źŲۻ���
};

#endif // SCHEDULER_H

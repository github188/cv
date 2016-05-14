#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H

#include <QObject>

#include <memory>

class Cleaner;
class Scheduler;
//class Optimizer;
class TxWebServiceController;
class RecordDBController;
class TxMonitorService;

class ComponentManager : public QObject
{
  Q_OBJECT

public:
  ComponentManager(QObject *parent = nullptr);
  ~ComponentManager();

  void Initialize();

  //ע�⣺��Щ�������ص�������ָ���ڲ�����ָ�룬ʹ��ʱȷ���ⲿ��Ҫ������ʹ������ָ���й�Ŷ
  Cleaner *GetCleaner() { return cleaner_.get(); }
  Scheduler *GetScheduler() { return scheduler_.get(); }

private:
  std::shared_ptr<Cleaner> cleaner_;
  //std::shared_ptr<Optimizer> optimizer_;
  std::shared_ptr<Scheduler> scheduler_;

  std::shared_ptr<TxWebServiceController> vod_controller_;
  std::shared_ptr<RecordDBController> db_controller_;
  std::shared_ptr<TxMonitorService> tx_monitor_service_;
};

#endif // COMPONENT_MANAGER_H

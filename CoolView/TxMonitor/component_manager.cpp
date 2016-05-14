#include "component_manager.h"

#include "txmonitorservice.h"
#include "tx_web_service_controller.h"
#include "record_db_controller.h"
#include "optimizer.h"
#include "cleaner.h"
#include "scheduler.h"

#include "util\ini\CVIniConfig.h"
#include "util\ini\TxConfig.h"


ComponentManager::ComponentManager(QObject *parent)
  : QObject(parent)
{

}

ComponentManager::~ComponentManager()
{

}

void ComponentManager::Initialize()
{
  bool is_model_tx = CVIniConfig::getInstance().IsModelTX();

  //db
  if (is_model_tx) {
    db_controller_.reset(new RecordDBController);
  }

  //dbus service
  tx_monitor_service_.reset(new TxMonitorService);
  tx_monitor_service_->Initialize(db_controller_);

  //���ǵ�TX�������⣬�������ļ��ṹ�Ż���
  /*Optimizer *optimizer = new Optimizer();
  optimizer->Init();
  optimizer->Start();
  _optimizer = optimizer;*/

  //��ʼ������ά������
  Cleaner *cleaner = new Cleaner();
  cleaner->Init();
  cleaner->Start();
  cleaner_.reset(cleaner);

  if (!is_model_tx) {
    //��TX��������������
    return;
  }

  //��ʼ���ƴ洢�ϴ����ٵ�����
  Scheduler *scheduler = new Scheduler();
  scheduler->Init();
  scheduler->Start();
  scheduler_.reset(scheduler);

  //vod webservice���
  vod_controller_.reset(new TxWebServiceController);
  vod_controller_->Initialize();
}

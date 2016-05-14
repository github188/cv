#include "tx_web_service_controller.h"

#include "tx_web_service_wrapper.h"
#include "util/ini/TxConfig.h"
#include "log.h"

TxWebServiceController::TxWebServiceController(QObject *parent)
  : QObject(parent)
{
  logger_ = CreateLogger();
  logger_->SetModuleName("soap");
}

TxWebServiceController::~TxWebServiceController()
{
  if (service_) {
    service_->StopService();
    service_.reset();
  }
  delete logger_; //ȷ��û�еĳ�Աʹ��logger��
}

void TxWebServiceController::Initialize()
{
  int port = 0;
  port = CTxConfig::getInstance().GetSoapListeningPort();
  if (0 == port) {
    LOG_ERROR_EX(logger_, "invalid port=%d", port);
    return;
  }
  LOG_INFO_EX(logger_, "start service from thread[%x]", QThread::currentThreadId());

  service_.reset(new TxWebServiceWrapper(logger_));
  service_->moveToThread(&service_thread_); //�ڵ����߳��й����������������߳�

  //��ͬ�̶߳������Ҫͨ���źŲ�
  connect(this, &TxWebServiceController::NotifyStartServiceSignal,
    service_.get(), &TxWebServiceWrapper::HandleStartServiceSlot);

  service_thread_.start();
  emit NotifyStartServiceSignal(port);
}

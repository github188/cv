#include "txmonitorservice.h"

#include <QtDBus/QDBusConnection>

#include <string>

#include "util\ini\CVIniConfig.h"
#include "util\ini\TxConfig.h"
#include "dbus\txMonitor\service\TxMonitorAdaptor.h"
#include "record_db_controller.h"
#include "Util.h"
#include "Ini.h"
#include "Log.h"


TxMonitorService::TxMonitorService()
  : config_(nullptr)
{
  new TxMonitorAdaptor(this);
}

TxMonitorService::~TxMonitorService()
{
}


void TxMonitorService::Initialize(
  std::shared_ptr<RecordDBController> db_controller)
{
  db_controller_ = db_controller;
  logger_.reset(CreateLogger());
  logger_->SetModuleName("dbus");

  // dbus
  QDBusConnection connection = QDBusConnection::sessionBus();
  bool ret = connection.registerService(TX_MONITOR_SERVICE_NAME);
  if (!ret) {
    LOG_ERROR_EX(logger_, "register service failed: %s", 
      qPrintable(connection.lastError().message()));
  }
  ret = connection.registerObject(TX_MONITOR_SERVICE_OBJECT_PATH ,this);
  if (!ret) {
    LOG_ERROR_EX(logger_, "register object failed: %s", 
      qPrintable(connection.lastError().message()));
  }

  //config
  config_ = &(CVIniConfig::getInstance());

  rec_path_ = QString::fromLocal8Bit(CTxConfig::getInstance().GetRecPath());
  upload_path_ = QString::fromLocal8Bit(CTxConfig::getInstance().GetUploadPath());
}


void TxMonitorService::ReportRecStat( int stat_type, const QByteArray &data_garray )
{
  if (!config_->IsModelTX() || !db_controller_) {
    return;
  }

  if (stat_type == RecStat_FileClose) {
    RecordCompleteParam param;
    param = DBusParamFromByteArray<RecordCompleteParam>(data_garray);

    //��¼������ļ��ƶ������ϴ�Ŀ¼
    //ͬһ�������ļ��ƶ�ֻ���޸Ĵ��������������ƶ��ļ����ݣ��ʼ�������Ӱ��IO
    QString new_path = param.view.file;
    new_path.replace(rec_path_, upload_path_);
    MakeParentDir(qPrintable(new_path)); //�����ļ��У����·�������������ƶ��ļ���ʧ��
    if (MoveFile(qPrintable(param.view.file), qPrintable(new_path))) {
      //����·����ֻ�洢���·������������Ժ��¼���Ŀ¼��Ҳ��Ӱ�����ݿ⣩
      param.view.file = GetRelativePath(upload_path_, new_path);
    } else {
      //�ƶ�ʧ��ʱ���޸ĵ�ǰ�ľ���·�������㶨λ�ļ�
      LOG_ERROR_EX(logger_, "move file failed:\n\tfrom: %s\n\tto: %s",
        qPrintable(param.view.file), qPrintable(new_path));
    }
    db_controller_->HandleRecordComplete(param);
  }
}


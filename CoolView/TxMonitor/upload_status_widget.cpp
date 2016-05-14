#include "upload_status_widget.h"
#include "WeblibUploader/WeblibUIComponont.h"
#include "WeblibUploader/workerservice.h"
#include "dbus/txMonitor/common/TxMonitorServiceCommon.h"
#include "record_sqlite_db.h"
#include "util/ini/TxConfig.h"
#include "log.h"


UploadStatusWidget::UploadStatusWidget(QWidget *parent)
    : QWidget(parent)
    , weblib_upload_widget_(nullptr)
    , status_msg_label_(nullptr)
    , upload_worker_service_(nullptr)
{
  ui.setupUi(this);
}

UploadStatusWidget::~UploadStatusWidget()
{

}

void UploadStatusWidget::Initialize()
{
  weblib_upload_widget_ = WeblibUIComponont::getUploadStatusWidget();
  if (weblib_upload_widget_) {
    ui.layout->addWidget(weblib_upload_widget_);
  }

  status_msg_label_ = new QLabel(QString::fromLocal8Bit("����"));
  status_msg_label_->setWordWrap(false);
  ui.layout->addWidget(status_msg_label_);

  logger_.reset(CreateLogger());
  logger_->SetModuleName("weblib");

  db_.reset(new RecordSqliteDB);

  upload_worker_service_ = &WorkerService::getInstance();
  connect(upload_worker_service_,
    &WorkerService::signalShowErrorInfo,
    this,
    &UploadStatusWidget::HandleShowErrorMessageSlot);
  connect(upload_worker_service_,
    &WorkerService::signalItemStateChanged,
    this,
    &UploadStatusWidget::HandleItemStateChangedSlot);

  file_uploaded_mark_ = CTxConfig::getInstance().GetUploadMark();
}

void UploadStatusWidget::HandleShowErrorMessageSlot( 
  const QString &info )
{
  QString infoInOneLine = info;
  infoInOneLine.replace("\n", " ");
  status_msg_label_->setText(infoInOneLine);
}

void UploadStatusWidget::HandleItemStateChangedSlot( 
  ItemOperate oper, const UploadItem &item )
{
  //TODO:���������ϴ����ֻ�ܱ���Uploading״̬������������ʵʲôҲ�����ˣ����������ǰ����Ч��
  //���⣬������filename�Ǿ���·����·���ָ���Ϊ/������\���Ժ����Ҫ�������Ҫע��
  if (db_ && item.status == Done) {
    IRecordDB::ViewEntityPointer view = db_->SelectViewByFilePath(item.filename);
    if (!view) {
      LOG_ERROR_EX(logger_, "set uploaded status failed, record not exist: %s",
        item.filename);
      return;
    }
    //�ϴ�����ϴ��ļ�����һ����׺���ı����ļ���������Ҫ�������ݿ�
    QString new_path = item.filename;
    int pos = new_path.lastIndexOf(".");
    if (pos != -1) {
      new_path = new_path.left(pos) + file_uploaded_mark_ + new_path.mid(pos);
    }
    view->file = new_path;
    view->status |= RecFileStat_Uploaded;
    if (db_->UpdateView(view) != 0) {
      LOG_ERROR_EX(logger_, "set uploaded status failed, update db error");
    }
  } 
  else if (item.status == Failed || item.status == Error) {
    LOG_INFO_EX(logger_, "weblib upload not complete, status=%d, file=%s",
      item.status, qPrintable(item.filename));
  }
}

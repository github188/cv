#include "monitor_window.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QTextBrowser>
#include <QScrollBar>
#include <QDateTime>
#include <QFileInfo>

#include "util\ini\CVIniConfig.h"
#include "util\ini\TxConfig.h"
#include "Util.h"
#include "component_manager.h"
#include "cleaner.h"
#include "scheduler.h"

#include "upload_status_widget.h"
#include "WeblibUploader/WeblibUIComponont.h"
#include "WeblibUploader/workerservice.h"
#include "db_browser_widget.h"
#include "file_browser_widget.h"


MonitorWindow::MonitorWindow(QWidget *parent)
    : QWidget(parent)
    , upload_status_widget_(nullptr)
    , upload_history_widget_(nullptr)
    , upload_log_widget_(nullptr)
{
  ui.setupUi(this);
}

MonitorWindow::~MonitorWindow()
{

}

void MonitorWindow::Initialize(ComponentManager *component_manager)
{
  if (!CVIniConfig::getInstance().IsModelTX()) {
    return;
  }

  //��ʼ��������ʽ�ʹ�С
  setWindowFlags(Qt::FramelessWindowHint);
  QRect rect = QApplication::desktop()->availableGeometry(0);
  setGeometry(rect.x() + rect.width() / 2, rect.y(), 
    rect.width() / 2, rect.height() / 2);
  ui.tabWidget->setTabPosition(QTabWidget::West);

  //����Widget
  InitialUploadTab();
  InitialLocalStorageTab();

  //�����źŲ�
  connect(
    component_manager->GetCleaner(), 
    &Cleaner::NotifyFreeDiskSpaceChangeSignal,
    this, 
    &MonitorWindow::HandleFreeDiskSpaceChangeNotifySlot);
  connect(
    component_manager->GetScheduler(), 
    &Scheduler::NotifyNetworkScheduleSignal,
    this, 
    &MonitorWindow::HandleNetworkScheduleNotifySlot);

  //����״̬��
  ui.globalStatusLabel->setText(
    QString::fromLocal8Bit("�����ռ��豸��Ϣ..."));

  show();

  //����
  library_path_ = GetAbsolutePath(
    QString::fromLocal8Bit(CTxConfig::getInstance().GetUploadPath()));
}

void MonitorWindow::InitialUploadTab()
{
  upload_tab_widget_ = new QTabWidget;
  ui.tabWidget->addTab(upload_tab_widget_, 
    QString::fromLocal8Bit("Weblib�ϴ�"));

  // upload status tab widget
  upload_status_widget_ = new UploadStatusWidget();
  upload_status_widget_->Initialize();
  upload_tab_widget_->addTab(upload_status_widget_, 
    QString::fromLocal8Bit("�ϴ�״̬"));

  // upload history widget
  upload_history_widget_ = WeblibUIComponont::getUploadHistoryWidget();
  if (upload_history_widget_) {
    upload_tab_widget_->addTab(upload_history_widget_, 
      QString::fromLocal8Bit("�ϴ���ʷ"));
  }

  // error message history
  // ��Ϊֻ��һ���ؼ�����ֱ�Ӽ��ˣ�û�н�ui�ļ�
  upload_log_widget_ = new QTextBrowser();
  upload_tab_widget_->addTab(upload_log_widget_, 
    QString::fromLocal8Bit("�ϴ���־"));

  // ��������
  connect(&WorkerService::getInstance(),
    &WorkerService::signalShowErrorInfo,
    this,
    &MonitorWindow::HandleWeblibErrorMessageSlot);
}

void MonitorWindow::InitialLocalStorageTab()
{
  local_storage_tab_widget_ = new QTabWidget;
  ui.tabWidget->addTab(local_storage_tab_widget_, 
    QString::fromLocal8Bit("���ش洢"));

  db_browser_widget_ = new DBBrowserWidget;
  db_browser_widget_->Initialize();
  local_storage_tab_widget_->addTab(db_browser_widget_,
    QString::fromLocal8Bit("��¼����"));

  file_browser_widget_ = new FileBrowserWidget;
  file_browser_widget_->Initialize();
  local_storage_tab_widget_->addTab(file_browser_widget_,
    QString::fromLocal8Bit("����洢"));

  connect(db_browser_widget_, &DBBrowserWidget::NotifyViewSelectedSignal,
    this, &MonitorWindow::HandleViewSelectedSlot);
}

void MonitorWindow::HandleWeblibErrorMessageSlot( const QString &msg )
{
  QString time_string = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
  upload_log_widget_->append(time_string);
  upload_log_widget_->append(msg);
  upload_log_widget_->append("");

  // ����������
  QScrollBar *sb = upload_log_widget_->verticalScrollBar();
  sb->setValue(sb->maximum());
}

void MonitorWindow::HandleNetworkScheduleNotifySlot( 
  const NetworkInfo &info, quint64 weblib_speed_limit )
{
  //ע�����紫������Ϊ1000���ƶ���1024
  network_schedule_msg_ = QString::fromLocal8Bit(
    "���У�%2MBps/%3MBps ���У�%4MBps/%5MBps "
    "Weblib���٣�%6MBps")
    //.arg(info.adapter_description)
    .arg(QString::number(info.in_speed_in_bytes / 1000000.0, 'f', 2))
    .arg(QString::number(info.receive_bandwidth / 1000000.0 / 8, 'f', 1))
    .arg(QString::number(info.out_speed_in_bytes / 1000000.0, 'f', 2))
    .arg(QString::number(info.transmit_bandwidth / 1000000.0 / 8, 'f', 1))
    .arg(QString::number(weblib_speed_limit / 1000000.0, 'f', 2));
  UpdateGlobalStatusMsg();
}

void MonitorWindow::HandleFreeDiskSpaceChangeNotifySlot( quint64 space )
{
  disk_space_msg_ = QString::fromLocal8Bit("���ؿ��ã�%1GB")
    .arg(QString::number((double)space/(1LL << 30), 'f', 2));
  UpdateGlobalStatusMsg();
}

void MonitorWindow::UpdateGlobalStatusMsg()
{
  QString msg = network_schedule_msg_;
  msg += " ";
  msg += disk_space_msg_;
  ui.globalStatusLabel->setText(msg);
}

void MonitorWindow::HandleViewSelectedSlot( 
  int vid, const QString &file )
{
  QString path = file;
  if (file.indexOf(":") == -1) {
    path = library_path_ + file;
  }
  QFileInfo file_info(path);
  if (file_info.exists() && file_info.isFile()) {
    local_storage_tab_widget_->setCurrentWidget(file_browser_widget_);
    file_browser_widget_->ExpandToPath(path);
  }
}

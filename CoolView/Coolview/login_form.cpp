#include "login_form.h"

#include <cassert>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <QtNetwork/QNetworkInterface>
#include <QMessageBox>
#include <QCloseEvent>

#include <util/NetworkUtil.h>
#include <util/ProcessManager.h>
#include <util/GetFileVersion.h>
#include <log/Log.h>
#include <util/desktop/screen_helper.h>

#include <dbus/sipwrapper/client/CvSipWrapperIf.h>
#include <dbus/sipwrapper/common/SipWrapperCommonService.h>
#include <DeviceManager/DeviceManager.h>

#include "profile/RunningProfile.h"
#include "config/RunningConfig.h"
#include "util/ini/CVIniConfig.h"

#include "ip_setting_dialog.h"
//#include "WeblibConfigDialog.h"
#include "devicelist_widget.h"

#include "session_controller_interface.h"
#include "system_controller_interface.h"
#include "controller_manager_interface.h"

LoginForm::LoginForm(QWidget *parent/* = nullptr*/)
    : QWidget(parent), 
      device_list_widget_(nullptr),
      update_screen_timer_(parent) {
  ui_.setupUi(this);
  has_initialized_ = false;
  ip_config_dialog_ = nullptr;
  controller_manager_ = nullptr;

  WSADATA wsa_data;
  int rc = WSAStartup(MAKEWORD(2,2), &wsa_data);

  SetGeometryInPrimaryScreen();
}

LoginForm::~LoginForm() {
   WSACleanup();

   if (device_list_widget_) {
     device_list_widget_->close();
   }
}

void LoginForm::SetGeometryInPrimaryScreen()
{
  QRect rect = ScreenHelper::Instance()->GetAvailableGeometry(-1);
  int half_width = rect.width() / 2;
  int half_height = rect.height() / 2;
  int offset_x = rect.x() + half_width;
  int offset_y = rect.y() + half_height;
  move(offset_x + (half_width - width()) / 2, 
       offset_y + (half_height - height()) / 2);
}

void LoginForm::Initialize( IControllerManager *controller_manager) {
  assert(has_initialized_ == false);
  assert(controller_manager != nullptr);

  controller_manager_ = controller_manager;
  // login and logout function
  ISessionController *session_controller = 
    controller_manager->GetSessionController();
  assert(session_controller != nullptr);
  connect(this, SIGNAL(LoginRequestSignal()),
          session_controller, SLOT(HandleLoginRequestSlot()));
  connect(
    session_controller, 
    SIGNAL(NotifyLoginErrorSignal(ISessionController::LoginError)),
    this,
    SLOT(HandleLoginErrorNotifySlot(ISessionController::LoginError)));

  connect(
    session_controller, 
    SIGNAL(NotifySessionStateChangedSignal(ISessionController::SessionState)),
    this
    , SLOT(HandleSessionStateChangedNotifySlot(ISessionController::SessionState)));

  ISystemController *system_controller =
    controller_manager->GetSystemController();
  assert(system_controller != nullptr);
  connect(this, &LoginForm::RestartSystemSignal, 
          system_controller, &ISystemController::HandleRestartSystemRequestSlot);
  connect(this, &LoginForm::ShutDownSystemSignal,
          system_controller, &ISystemController::HandleShutDownSystemRequestSlot);
  connect(this, &LoginForm::QuitSytemSignal,
          system_controller, &ISystemController::HandleQuitSystemRequestSlot);

  connect(&update_screen_timer_, &QTimer::timeout,
          this, &LoginForm::UpdateScreenSlot);
  update_screen_timer_.start(2000);

  connect(ScreenHelper::Instance(), &ScreenHelper::PrimaryScreenChanged,
          this, &LoginForm::HandlePrimaryScreenChangedNotifySlot);

  // ��ʼ��������¼�
  SetupGui();

  // ���ܻ����Զ���¼�������Ҫ�����ж�������ʼ����Ž������ö�ȡ
  LoadConfigSlot();

  has_initialized_ = true;
}

void LoginForm::SetupGui() {
  
  //�����ö���ȥ��������gmlan20150714
  setWindowFlags( Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint );

  //��ȡ������Ч���������д����¼�һ����gmlan20150714
  setMouseTracking(true);


  // ���ؽ����¼�����
  connect(ui_.loginButton, SIGNAL(clicked()), this, SLOT(TryToLoginSlot()));
  connect(ui_.cancelButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui_.deviceConfigButton, SIGNAL(clicked()), 
          this, SLOT(ShowDeviceSettingWidgetSlot()));
  connect(ui_.autoLoginCheckbox, SIGNAL(stateChanged(int)), 
          this, SLOT(AutoLoginCheckedSlot(int)));
  connect(ui_.toolButton, SIGNAL(clicked()),
          this, SLOT(ShowOrHideExtraSettingSlot()));
  connect(ui_.ipConfigButton, SIGNAL(clicked()),
          this, SLOT(ShowIpConfigDialogSlot()));
  connect(ui_.weblibConfigButton, SIGNAL(clicked()),
          this, SLOT(ShowWeblibConfigDialogSlot()));
  connect(ui_.refreshIPListButton, SIGNAL(clicked()),
          this, SLOT(LoadIPListSlot()));

  ui_.ipConfigButton->setVisible(true);
  ui_.deviceConfigButton->setVisible(true);
  ui_.weblibConfigButton->setVisible(false);
  ui_.toolButton->setVisible(false);
  //ui_.verticalLayout_2->setSizeConstraint(QLayout::SetFixedSize);


}

void LoginForm::LoadConfigSlot() {
  RunningProfile* profile = RunningProfile::getInstance();

  profile->LoadFile();

  LoadIPListSlot();

  // ��ȡ�û���¼����
  if ( profile->ukey_certify() || profile->remember_profile() ) {
    ui_.rememberCheckBox->setChecked(true);
    ui_.userNameLineEdit->setText( QString::fromStdString(profile->username()) );
    ui_.passwordLineEdit->setText( QString::fromStdString(profile->password()) );

    // realm & global Service
    ui_.realmComboBox->setCurrentText(profile->realm().c_str());
  } else {
    ui_.userNameLineEdit->setText("");
    ui_.passwordLineEdit->setText("");
    ui_.rememberCheckBox->setChecked(false);
  }
}


void LoginForm::TryAutoLogin() {
  assert(has_initialized_);

  RunningProfile* profile = RunningProfile::getInstance();
  if (profile->ukey_certify() || profile->auto_login()) {
    // ���ͨ����ukey��֤�������û�ѡ�����Զ���¼����ôֱ�ӵ�¼������
    ui_.autoLoginCheckbox->setChecked(true);

    if (CheckIPState() == false) {
      // �����ڿ��õ�IP��ַ
      QMessageBox::critical( this , 
        QString::fromLocal8Bit("IP����"),
        QString::fromLocal8Bit("û�п��õ�IP��ַ��"),
        QMessageBox::Ok);
      return;
    }

    TryToLoginSlot();
  }
}

bool LoginForm::CheckIPState() {
  if (ui_.IPListComboBox->count() > 0) {
    return true;
  } else {
    for(int i=0; i<3; ++i) {
      Sleep(3000);
      LoadIPListSlot();
      if (ui_.IPListComboBox->count())
        return true;
    }
    return false;
  }
}

void LoginForm::LoadIPListSlot() {
  QStringList ipList = GetHostIPList();
  ui_.IPListComboBox->clear();
  ui_.IPListComboBox->addItems(ipList);
  for(int i=0; i<ipList.size(); ++i) {
    QString ip = ipList.at(i);
    if (ip.contains('.')) {
      ui_.IPListComboBox->setCurrentIndex(ui_.IPListComboBox->findText(ip));
      break;
    }
  }

  RunningProfile* profile = RunningProfile::getInstance();
  if (profile->remember_profile()) {
    int index = ui_.IPListComboBox->findText(
      QString::fromStdString(profile->ip_address()));
    ui_.IPListComboBox->setCurrentIndex(index > 0 ? index : 0);
  }
}

void LoginForm::TryToLoginSlot() {
  assert(has_initialized_);

  if (ui_.userNameLineEdit->text().isEmpty() || 
    ui_.passwordLineEdit->text().isEmpty()) {
      ui_.warningLabel->setText(QString::fromLocal8Bit("�������ʺź����룡 "));
  } else {
    //�ȱ�������
    SaveConfigSlot();

    if (RunningConfig::Instance()->isEnableKinect()) {
      //���Ĭ��ʹ��kinect������kinect����
      startKinectDaemon();
    }

    //��վ�ʾ����
    ui_.warningLabel->setText("");

    emit LoginRequestSignal();
  }
}

void LoginForm::SaveConfigSlot() {
  RunningProfile* profile = RunningProfile::getInstance();

  profile->set_auto_login(ui_.autoLoginCheckbox->isChecked());
  profile->set_remember_profile(ui_.rememberCheckBox->isChecked());

  QString login = ui_.userNameLineEdit->text().trimmed().toLower();
  profile->set_username(login.toStdString());

  QString password = ui_.passwordLineEdit->text().trimmed();
  profile->set_password(password.toStdString());

  QString realm = ui_.realmComboBox->currentText();
  profile->set_realm(realm.toStdString());

  QString ipAddr = ui_.IPListComboBox->currentText();
  profile->set_ip_address(ipAddr.toStdString());

  QString mac = GetMacAddress(ipAddr);
  profile->set_mac_address(mac.toStdString());

  profile->SaveFile();

  RunningConfig* config = RunningConfig::Instance();

  config->saveConfig();
}

void LoginForm::HandleLoginErrorNotifySlot( 
    ISessionController::LoginError error) {
  QString warnning_msg;
  switch (error) {
  case ISessionController::kOk: 
    warnning_msg = QLatin1String(""); break;
  case ISessionController::kErrorOfTimeOut:
    warnning_msg = QString::fromLocal8Bit("��¼��ʱ"); break;
  case ISessionController::kErrorOfAuthenticationFailure:
    warnning_msg = QString::fromLocal8Bit("��֤ʧ��"); break;
  case ISessionController::kErrorOfConnectToServerFailure:
    warnning_msg = QString::fromLocal8Bit("�޷����ӵ�������"); break;
  case ISessionController::kErrorOfPermissionDenied:
    warnning_msg = QString::fromLocal8Bit("�ն�û��Ȩ��ʹ�ñ��˺ŵ�½");
  case ISessionController::kErrorOfAccessNetworkAdaptorFailure:
    // fall through
  case ISessionController::kErrorOfNetworkInterfaceNonoperational:
    warnning_msg = QString::fromLocal8Bit("�������ӳ������⣬���������豸��");
    break;
  case ISessionController::kErrorOfAlreadyLogin:
    warnning_msg = QString::fromLocal8Bit("���ն��ѵ�½");
    break;
  case ISessionController::kErrorOfUnknown:
    // fall through
  default:
    warnning_msg = QString::fromLocal8Bit("δ֪����"); break;
  }
  ui_.warningLabel->setText(warnning_msg);
  if (error != ISessionController::kOk) {
    show();
  }
}

void LoginForm::HandleSessionStateChangedNotifySlot(
    ISessionController::SessionState state) {
  switch (state) {
  case ISessionController::kIsLogining:
    ui_.warningLabel->setText(QString::fromLocal8Bit("��½��...")); break;
  case ISessionController::kIsOnline:
    ui_.warningLabel->setText(QString::fromLocal8Bit("��½�ɹ�")); 
    HandleLoginNofity();
    break;
  case ISessionController::kIsOffline:
    ui_.warningLabel->setText(QString::fromLocal8Bit("�˺��˳�"));
    HandleLogoutNofity();
    break;
  default:
    assert(false); break;
  }
}

void LoginForm::HandleLoginNofity() {
  hide();
}

void LoginForm::HandleLogoutNofity() {
  LoadConfigSlot();
  ui_.warningLabel->setText("");
}

void LoginForm::ShowOrHideExtraSettingSlot() {
  if (ui_.toolButton->arrowType() == Qt::RightArrow) {
    ui_.toolButton->setArrowType(Qt::DownArrow);
    ui_.ipConfigButton->show();
    ui_.deviceConfigButton->show();
  } else {
    ui_.toolButton->setArrowType(Qt::RightArrow);
    ui_.ipConfigButton->hide();
    ui_.deviceConfigButton->hide();
  }
}

void LoginForm::AutoLoginCheckedSlot( int state) {
  if (state == Qt::Checked) {
    ui_.rememberCheckBox->setChecked(true);
  }
}

void LoginForm::ShowIpConfigDialogSlot() {
  if (ip_config_dialog_ == nullptr) {
    ip_config_dialog_ = new IPSettingDialog( this );
    connect(ip_config_dialog_, SIGNAL(ReloadConfigSignal()),
            this, SLOT(LoadConfigSlot()));
    connect(ip_config_dialog_, SIGNAL(RestartSystemSignal()),
            this, SIGNAL(RestartSystemSignal()));
  }
  ip_config_dialog_->show();
}

void LoginForm::ShowDeviceSettingWidgetSlot() {
  if (device_list_widget_) {
    if (!device_list_widget_->isVisible()) {
      device_list_widget_->ReadConfig();
    }
    device_list_widget_->show();
    device_list_widget_->raise();
    device_list_widget_->setFocus();
  }
}

void LoginForm::closeEvent( QCloseEvent * event ) {
  QString infoMsg;
#ifdef _DEBUG
  infoMsg = QString::fromLocal8Bit("ȷ���˳�ϵͳ��");
#else
  infoMsg = QString::fromLocal8Bit("ȷ���˳�ϵͳ���ػ���");
#endif
  QMessageBox::StandardButton reply = QMessageBox::warning(
    this, tr("CoolView"), infoMsg, QMessageBox::Yes | QMessageBox::No);
  if (reply != QMessageBox::Yes) {
    event->ignore();
    return;
  }
  
#ifdef _DEBUG
  emit QuitSytemSignal();
#else
  emit ShutDownSystemSignal();
#endif

  event->accept();
}

void LoginForm::UpdateScreenSlot() {
  ScreenHelper::Instance()->Update();
}

void LoginForm::HandlePrimaryScreenChangedNotifySlot() {
  SetGeometryInPrimaryScreen();
}

//5.2���ϵ�mousePressEvent�¼���д
void LoginForm::mousePressEvent(QMouseEvent *event)
{                                                                                                          
	if (event->button() == Qt::LeftButton) 
	{
		dragPosition = event->globalPos() - frameGeometry().topLeft();
		event->accept();
	}
}

void LoginForm::mouseMoveEvent(QMouseEvent *event)
{   
	if (event->buttons() & Qt::LeftButton)
	{
		move(event->globalPos() - dragPosition);
		event->accept();
	}
}

//���������ڱ���ͼ���߿�Բ��//gmlan20150714
void LoginForm::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QBrush brush;
	brush.setTextureImage(QImage(":/image/login_bg.jpg")); //����ͼƬ
	painter.setBrush(brush);
	painter.setPen(Qt::gray);  //�߿�ɫ
	painter.drawRoundedRect(QRect(0,0,this->width()-1,this->height()-1),0,0); //Բ��5����
}
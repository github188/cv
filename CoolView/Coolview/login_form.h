#ifndef LOGIN_FORM_H
#define LOGIN_FORM_H

#include <QWidget>

#include "ui_login_form.h"
#include "CoolviewCommon.h"

class MainControlWindow;
class IPSettingDialog;
class DeviceListWidget;

class IControllerManager;
class ISystemController;

#include "session_controller_interface.h"

class LoginForm : public QWidget {
  Q_OBJECT
public:
  LoginForm(QWidget *parent = nullptr);
  ~LoginForm();

  void Initialize(IControllerManager *controller_manager);
  void TryAutoLogin();
  void SetDeviceListWidget(DeviceListWidget *device_list_widget) {
    device_list_widget_ = device_list_widget;
  }

Q_SIGNALS:
  void LoginRequestSignal();
  void ShutDownSystemSignal();
  void RestartSystemSignal();
  void QuitSytemSignal();

private Q_SLOTS:
  // �����session controller��õ���ʾ�ź�
  void HandleLoginErrorNotifySlot(ISessionController::LoginError error);
  void HandleSessionStateChangedNotifySlot(ISessionController::SessionState state);

  void TryToLoginSlot();

  // ��ʾ���������ý���
  void ShowOrHideExtraSettingSlot();

  // �û����Զ���¼��ѡ������˲���
  void AutoLoginCheckedSlot(int state);

  // ������ҳ���ô���
  void ShowIpConfigDialogSlot();

  // �����豸���ý���
  void ShowDeviceSettingWidgetSlot();

  //ˢ��IP�б�
  void LoadIPListSlot();

  void LoadConfigSlot();

  void SaveConfigSlot();

  void UpdateScreenSlot();

  void HandlePrimaryScreenChangedNotifySlot();

private:
  void SetupGui();
  void HandleLoginNofity();
  void HandleLogoutNofity();
  void SetGeometryInPrimaryScreen();

protected:
  // ���عر��¼�
  void closeEvent(QCloseEvent * event) override;

  //��д����¼�gmlan20150714
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);

  //���������ڱ���ͼ���߿�Բ��//gmlan20150714
  void paintEvent(QPaintEvent *event);

private:
  // �趨�˹̶�IP���Զ���¼ʱ�����ܻ��������ʱIP��ַ��ȡʧ�ܵ��������ʱͨ��
  // ˯������������̶�IP��ʱ�����õ�����		by Ruan 2013/1/14
  bool CheckIPState();

private:
  Ui::LoginForm ui_;

  IControllerManager *controller_manager_;

  // IpConfig
  IPSettingDialog *ip_config_dialog_;

  // �豸����
  DeviceListWidget *device_list_widget_;

  QTimer update_screen_timer_;

  bool has_initialized_;

  //��д����¼�gmlan20150714
  QPoint dragPosition;
};

#endif // LOGIN_FORM_H

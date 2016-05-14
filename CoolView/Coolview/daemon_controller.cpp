#include "daemon_controller.h"

#include <windows.h>

#include "CoolviewCommon.h"

#include <dbus/channelDispatcher/client/ChannelDispatcherIf.h>
#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>

const char *procs[] = {
	DBUS_DAEMON_PROC_NAME,
	COOLVIEW_SIP_WRAPPER_RPOC_NAME,
	COOLVIEW_TELECONTROLLER_PROC_NAME,
	COOLVIEW_LOCAL_MONITOR_PROC_NAME,
	COOLVIEW_MONITOR_PROC_NAME,
	PROC_NETWORK_MEASURER_NAME,
	COOLVIEW_TX_MONITOR_NAME,
};


DaemonController::DaemonController(QObject *parent)
  : IDaemonController(parent)
{
  channel_dispatcher_proxy_ = nullptr;
  show_console_ = false;
	first_time_show_console_ = true;
}

DaemonController::~DaemonController()
{

}

void DaemonController::Initialize()
{
  // ��Ҫ�ڴ˳�ʼ��dbus proxy�����ܵ���CoolView��DBusͨ��ʧ��
  QString app_dir = QCoreApplication::applicationDirPath();
  QString debug_path = app_dir + "\\debug";
  if(QFile::exists(debug_path)) {
    show_console_ = true;
  } else {
    show_console_ = false;
  }
}

void DaemonController::StartDaemon()
{
	//TODO:Ӧ���ø��෽����дstartReleativeServices()
  startReleativeServices();
  // TODO: ��CvConferenceRoom.exe��ʧ�ź�ʱ�������������ʱ�취���
  //QThread::msleep(5000);
}

void DaemonController::StopDaemon()
{
	//TODO:Ӧ���ø��෽����дstopReleativeServices()
  stopReleativeServices();
}

void DaemonController::HandleShowConsoleSlot()
{
	if (first_time_show_console_) {
		channel_dispatcher_proxy_ = new ChannelDispatcherIf( 
			CHANNEL_DISPATCHER_SERVICE_NAME, 
			CHANNEL_DISPATCHER_OBJECT_PATH,
			QDBusConnection::sessionBus());

		//����MSDN��ShowWindow API��˵����Ӧ�ó����״ε���ShowWindowʱ������Ӧ����
		//��������ʱSTATUPINFO��ָ���Ĳ���һ�£��ʴ˴��Գ�ʼ״̬����һ�Ρ�
		//ʵ�ʲ��Է��֣���ִ����һ������һ�ε���ʱ�еĿ���̨��������ȷʵ����ȷ��
		ChangeConsoleShowStatus(show_console_);
		first_time_show_console_ = false;
	}
	show_console_ = !show_console_;
	ChangeConsoleShowStatus(show_console_);
}

void DaemonController::ChangeConsoleShowStatus( bool show )
{
	// channeldispatcher��ý��������
	if (channel_dispatcher_proxy_) {
		QByteArray recvByteArray;
		QDataStream out( &recvByteArray , QIODevice::WriteOnly );
		out << show;
		channel_dispatcher_proxy_->ModifyChannel(
			"", "", MMT_ShowConsoleWindow, recvByteArray);
	}

	// ���²���Ӧ����startReleativeServices�������Ľ���һ��
	const int flag = show ? SW_SHOW : SW_HIDE;
	HWND console_window = NULL;

	for (int i = 0; i < sizeof(procs)/sizeof(char*); ++i) {
		const char * window_title = procs[i];
		console_window = FindWindowA(NULL, window_title); // ǿ��ʹ�ö��ֽڰ汾
		if (NULL != console_window) {
			ShowWindow(console_window, flag);
			continue;
		}
		// �еĽ��̱���������·��
		const QString app_path = getProcessPath(procs[i]);
		console_window = FindWindowA(NULL, qPrintable(app_path)); // ǿ��ʹ�ö��ֽڰ汾
		if (NULL != console_window) {
			ShowWindow(console_window, flag);
		}
	}
}

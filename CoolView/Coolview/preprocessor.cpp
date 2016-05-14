#include "preprocessor.h"

#include <QThread>
#include <QWidget>
#include <QMessageBox>

#include "ui_pending_widget.h"
#include "util/NetworkUtil.h"
#include "launcher.h"
#include "config/RunningConfig.h"

Preprocessor::Preprocessor(QObject *parent)
	: QObject(parent)
{
	work_thread_ = nullptr;
    pending_widget_ = nullptr;
  ip_valid_ = false;
  video_valid_ = false;
}

Preprocessor::~Preprocessor()
{
	if (work_thread_) {
		work_thread_->deleteLater();
	}
}

void Preprocessor::Initialize(Launcher *launcher)
{
	work_thread_ = new QThread();
	launcher_ = launcher;
	moveToThread(work_thread_); // ͨ���۵ĵ��ý������߳���ִ��
	connect(this, &Preprocessor::StartedSignal,
		this, &Preprocessor::Preprocess);
	connect(this, &Preprocessor::FinishedSignal,
		this, &Preprocessor::Launch);
}

void Preprocessor::Start()
{
	// �����߳���ʾGUI
	ShowPendingWidget();
    //RunningConfigҪ����UI�����ȡ��Ļ���ã����������߳������У����������һ����ȷ�����ѳ�ʼ��
    RunningConfig::Instance();
	// �����źţ������߳��п�ʼ��������
	if (work_thread_) {
		work_thread_->start();
		emit StartedSignal();
	}
}

void Preprocessor::Preprocess()
{
	CheckIP();
    CheckVideoDevice();
	//�ƶ������̣߳��Ա����Launcher��ط���
	moveToThread(QApplication::instance()->thread());
	work_thread_->quit();
	emit FinishedSignal();
    pending_widget_ = nullptr;//FinishedSignal���ӵ����ڵ����ٲ�
}

void Preprocessor::Launch()
{
  //�����봦����������Ϣ
  //�����߳��в���ʹ��GUI�����ڴ˼��д���
  if (!ip_valid_) {
    QMessageBox::critical(nullptr, 
      QString::fromLocal8Bit("IP����"),
      QString::fromLocal8Bit("�޷���ȡ��ЧIP��ַ�������������ã�������ϵͳ��"));
  }
  if (!video_valid_) {
    QMessageBox::critical(nullptr, 
      QString::fromLocal8Bit("�豸����"),
      QString::fromLocal8Bit("������Ƶ�����豸�޷�����⵽��������Ƶ���ã�������ϵͳ��"));
  }

  //����Laucher
	if (launcher_) {
		launcher_->Initialize();
		launcher_->Start();
	}
}

void Preprocessor::ShowPendingWidget()
{
	if (!work_thread_) {
		return;
	}
	
    pending_widget_ = new QWidget();
	Ui::PendingWidget *pending_widget_ui = new Ui::PendingWidget();
	pending_widget_ui->setupUi(pending_widget_);
	pending_widget_->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	pending_widget_->show();
	// ���߳�ֹͣʱ�Զ����ٴ���
	connect(this, &Preprocessor::FinishedSignal,
        pending_widget_, &QObject::deleteLater);
    connect(this, &Preprocessor::ShowMsgSignal,
        pending_widget_ui->msgLabel, &QLabel::setText);
}

/**
 * LoginForm�е�CheckIPState()Ҳ�Ǽ��IP��Ϊ�λ�Ҫ�ڴ�Ԥ�����أ�
 * ��Ϊ���û����ЧIP��DBus�޷�����������DBus���������ڻ��IP֮��
 * ��Ҫ�ȴ�IP�����������DBus
 */
void Preprocessor::CheckIP()
{
  emit ShowMsgSignal(QString::fromLocal8Bit("���ڼ��IP��ַ..."));
	//���IP��ַ
  //���µȴ�max_pending_time/1000��
  const unsigned int sleep_interval = 100; //ms
  const unsigned int max_pending_time = 15000; //ms
  unsigned int pending_time = 0;

	while (pending_time < max_pending_time) {
		QStringList ip_list = GetHostIPList();
		for (auto ip : ip_list) {
			//TODO:����ֻ������IPv4��δ����IPv6
			if (!ip.isEmpty() && 
				  !ip.startsWith("0.") &&
					!ip.startsWith("169.")) {
						ip_valid_ = true;
						break;
			}
		}
		if (ip_valid_) {
			break;
		}
    pending_time += sleep_interval;
		QThread::msleep(sleep_interval); // �ȴ�100ms����
	}
}

void Preprocessor::CheckVideoDevice()
{
    emit ShowMsgSignal(QString::fromLocal8Bit("���ڼ����Ƶ�豸..."));
    const unsigned int sleep_interval = 1000; //ms
    const unsigned int max_pending_time = 15000; //ms
    unsigned int pending_time = 0;

    while (pending_time < max_pending_time) {
        RunningConfig::Instance()->loadVideoConfig();//ÿ�μ��ǰ��ȡһ������
        video_valid_ = RunningConfig::Instance()->IsConfiguredVideoDeviceValid();
        if (video_valid_) {
            break;
        }
        pending_time += sleep_interval;
        QThread::msleep(sleep_interval); // �ȴ�100ms����
    }
}

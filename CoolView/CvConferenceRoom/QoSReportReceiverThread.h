#pragma once
#include <QObject>
#include <QTimer>
#include "util/report/RtpStat.h"
#include "util/report/RecordStat.h"

class UdpReceiver;
class QtConfrenceVideoWindow;

// TODO:Ӧ�ù���CoolView�ı������ģ��
// ����ʹ��MoveToThread�����ƶ����߳���

class QoSReportReceiverThread :public QObject
{
  Q_OBJECT

public:
	QoSReportReceiverThread();
	~QoSReportReceiverThread();

  void Initialize(const QString&bindIP , const int port);

	//�����ȴ���������
	void Start();

	//ֹͣ�����߳�
	void Stop()
	{	
		_terminate = true;
	}


Q_SIGNALS:
  void SendUDPQoSReportSignal(const RtpStatItem &item);
  void SendUDPRecReportSignal(const RecStatItem &item);
  void StartSignal();

protected slots:
	void run();

private:
	UdpReceiver*	_udpServer;
	bool	_terminate;
};

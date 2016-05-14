#pragma once
#include "ui_RtcpInfoWidget.h"
#include <Windows.h>
#include <RtpStat.h>
class QtWengoPhone;
class CWengoPhone;

class QtRtcpInfoWidget:public QWidget
{
	friend class QtConferenceRoomWidget;

	Q_OBJECT
public:
	QtRtcpInfoWidget(QtWengoPhone & qtWengoPhone);
	~QtRtcpInfoWidget(void);

    void displayTranStat(const TranStat & transtat );

	//ʹ�� signal/slot ����ʵ���̰߳�ȫ

Q_SIGNALS:
	void displayTranStatSinal(const TranStat * pTranstat );

private Q_SLOTS:
	void displayTranStatSlot(const TranStat * pTranstat );

	void showRtcpMessage( bool show );
	
private:
	void clearStatText();		//���ͳ���ı�

	void tranStatRecvEventHandler( const TranStat & transtat );

	Ui::RtcpInfoWidget *_ui;
	QtWengoPhone & _qtWengoPhone;
	CWengoPhone & _cWengoPhone;
	TranStat _latestTranStat;
};

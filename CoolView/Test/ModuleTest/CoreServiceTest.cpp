#include "CoreServiceTest.h"
#include <dbus/core/common/CvCoreServiceCommon.h>
#include <dbus/core/client/CvCoreIf.h>
#include <QtDBus/QtDBus>
#include <iostream>
#include <Windows.h>
using namespace std;
void CoreServiceTest()
{
	CvCoreIf coreProxy( CVCORE_SERVICE_NAME,CVCORE_SERVICE_OBJECT_PATH,
		 QDBusConnection::sessionBus());
	
	QString UserSip = "scut_t_xzh2@sip.ccnl.scut.edu.cn";
	int ScreenIndex = 0;
	int Seet = 1;
	QByteArray SubData;
	QDataStream SubOut(&SubData, QIODevice::WriteOnly );
	SubOut << UserSip << ScreenIndex << Seet;

	/*
	cout << "���Խ�������Ƶ֮�������ͷ�ý����" << endl;
	//��������Ƶ
	coreProxy.TeleCommand(CoreAction_StartRecvStreamedMedia,SubData);


	//ȡ����������Ƶ
	coreProxy.TeleCommand(CoreAction_StopRecvStreamedMedia,SubData);

	getchar();
	*/

	//��������Ƶ��
	coreProxy.TeleCommand(CoreAction_StartRecvStreamedMedia,SubData);

	getchar();

	const int initialTimes = 5;
	int tryTimes = initialTimes;
	while(true)
	{
		while(tryTimes>0 )
		{
			cout << "�����ͷ�ý����֮�������������Ƶ" <<endl;

			coreProxy.TeleCommand(CoreAction_StopRecvStreamedMedia,SubData);
			coreProxy.TeleCommand(CoreAction_StartRecvStreamedMedia,SubData);
			tryTimes--;

			Sleep(100);
		}

		tryTimes = initialTimes*2;
		getchar();
	}
}
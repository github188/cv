#include <QtCore/QCoreApplication>
#include <log/Log.h>
#include "CvScreenMediaService.h"
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	//����log�ļ���
	SetLogComponent("screen_recv" );

	LOG_DEBUG( "����ScreenMedia[%s]" , RECV_SCREEN_MEDIA_ID );
	//ע�����DBUS
	CvScreenMediaService service( RECV_SCREEN_MEDIA_ID );
	//CvScreenMediaService service( "testrecv" );			//������

	return a.exec();
}

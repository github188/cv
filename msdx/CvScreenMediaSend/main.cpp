#include <QtWidgets/QApplication>
#include <log/Log.h>
#include "CvScreenMediaService.h"
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//����log�ļ���
	SetLogComponent("screen_send" );

	LOG_DEBUG( "����ScreenMedia[%s]" ,SEND_SCREEN_MEDIA_ID );
	//ע�����DBUS
	CvScreenMediaService service( SEND_SCREEN_MEDIA_ID );
	//CvScreenMediaService service( "testsend" );			//������

	return a.exec();
}

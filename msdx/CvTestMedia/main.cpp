#include <QtCore/QCoreApplication>
#include "CvTestMediaService.h"
#include <dbus/channel/type/testMedia/common/testMediaServiceCommon.h>
#include <log/Log.h>
#include <Windows.h>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	char* mediaID = NULL;
	if( argc>=2 )
		mediaID = argv[1];
	else
		mediaID = VIDEO_PREVIEW_MEDIA_ID;//DEVICE_TEST_MEDIA_ID; 

	//����console������Ŀ����Ҫ����
	char consoleTitle[_MAX_PATH]={0};
	strcpy( consoleTitle , TEST_MEDIA_PROC_NAME );
	strcat( consoleTitle , "_");
	strcat( consoleTitle , mediaID );

	SetConsoleTitleA( consoleTitle );

	//����log�ļ���
	SetLogComponent(mediaID );

	LOG_DEBUG("����TestMedia[%s]" , mediaID );

	//ע�����DBUS
	QString sMediaID = QString(mediaID);
	CvTestMedia service( sMediaID );

	
	
	

	int result = a.exec();
	qDebug("result:%d" , result );

	return result;
}

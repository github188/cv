#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include "qtconferenceroom.h"
#include "ConferenceRoomService.h"
#include "MediaWindowManager.h"
#include <log/Log.h>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	strcpy(Log::_logFileName , "CvConferenceRoom");
	QTranslator qtTralator;
	qtTralator.load(":/CvConferenceRoom/Resources/qt_zh_CN.qm");
	a.installTranslator( &qtTralator );

	QFont font  = a.font();
	font.setPointSize(9);
  //font.setStyleStrategy (QFont::PreferAntialias);
	a.setFont(font);

	//���ò����֧��ͼƬ��ʽ
	QString appDir = QCoreApplication::applicationDirPath();
	appDir.replace("/" , "\\" );
	QString pluginsDir = appDir + "\\plugins";
	a.addLibraryPath( pluginsDir );

  a.setAutoSipEnabled(true);
  QApplication::setQuitOnLastWindowClosed(false); //�����������أ���һ���÷�ֹQMessageBox�ر�ʱ�����˳�

	MediaWindowManager::getInstance();

	ConferenceRoomService::getInstance();

	return a.exec();
}
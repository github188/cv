#pragma once

#include <Qtcore/QString>
#include <QtCore/QDataStream>
#include <QRect>

#define		SCREEN_MEDIA_SERVICE_NAME			"com.dcampus.coolview.channel.type.screenMedia"
#define		SCREEN_MEDIA_SERVICE_OBJECT_PATH	"/com/dcampus/coolview/channel/type/screenMedia"

#define RECV_SCREEN_MEDIA_PROC_NAME	"CvScreenMediaRecv"
#define SEND_SCREEN_MEDIA_PROC_NAME "CvScreenMediaSend"
#define SEND_SCREEN_MEDIA_ID	"ScreenSend"			//������Ļ����mediaID
#define RECV_SCREEN_MEDIA_ID	"ScreenRecv"			//������Ļ����mediaID


inline QString getScreenMediaServiceName( const QString& mediaID )
{
	return QString(SCREEN_MEDIA_SERVICE_NAME)+ QString(".") + mediaID;
}

inline QString getScreenMediaObjectPath( const QString& mediaID )
{
	return QString(SCREEN_MEDIA_SERVICE_OBJECT_PATH)+ QString("/") + mediaID;
}


enum ScreenMediaActionID
{
	ScreenMediaAction_InitialRecvMedia,
	ScreenMediaAction_InitialSendMedia,
	ScreenMediaAction_AddSendDst,			//������ý���������Ŀ���ַ
	ScreenMediaAction_RemoveSendDst,		//������ý�������Ƴ�Ŀ���ַ
	ScreenMediaAction_ShowConsoleWindow,
};

class ScreenMediaInfo
{
public:

	///������Ϣ
	QString		_ipAddress;
	int			_port;

	///������λ����Ϣ
	QRect		_screenWnd;		//��������Ĵ��ھ���
	friend QDataStream& operator>>(QDataStream& in, ScreenMediaInfo& data)
	{
		in >> data._ipAddress >> data._port >> data._screenWnd;
		return in;
	}
	friend QDataStream& operator<<(QDataStream& out, const ScreenMediaInfo& data)
	{
		out << data._ipAddress << data._port << data._screenWnd;
		return out;
	}
};
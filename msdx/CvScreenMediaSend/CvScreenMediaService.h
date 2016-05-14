#ifndef CV_SCREEN_MEDIA_SERVICE_H
#define CV_SCREEN_MEDIA_SERVICE_H

#include <QtCore/QtCore>


#include <dbus/channel/type/screenMedia/common/ScreenMediaServiceCommon.h>
#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>

typedef unsigned char BYTE;


class ScreenDist;
typedef QList<ScreenDist*>		ScreenDistList;

class ScreenSnapping;
class ChannelDispatcherIf;

class CvScreenMediaService:public QObject
{
	friend class CvScreenMediaService;
	Q_OBJECT
public: // PROPERTIES
	CvScreenMediaService( QString mediaID );
	~CvScreenMediaService( );

public Q_SLOTS: // METHODS
	void Destroy();
	QByteArray GetMediaInfo(int info_type);
	void Pause();
	void Run();
	void SetMediaInfo(int action_id, const QByteArray &input_garray);
	void Stop();
Q_SIGNALS: // SIGNALS
	void RunningStateChanged(const QString &media_id, int current_state);

private Q_SLOTS:
	int sendImage();

private:
	
	int createSendMedia( const ScreenMediaInfo& info );


	int addSendMedia( const QString& dstIP );
	int removeSendMedia( const QString& dstIP );

	int captureScreen();

	

	
	MediaState				_currentState;
	MediaDirection			_mediaDirection;
	QString					_mediaID;
	QRect					_captureWindow;		//��Ҫ���н����Ĵ���

	int						_port;				//��Ļ��ռ�õĶ˿�

	ScreenDistList			_sendDistList;

	ScreenSnapping*			_snapper;

	QTimer*					_sendImageTimer;

	ChannelDispatcherIf*	_channelDispatcherProxy;

	///ͬ�������ڷ���ͼƬ�Ĺ����в�����ӻ�ɾ����ַ����֤IP�б��һ����
	QMutex					_mutext;
};


#endif
#ifndef CV_SCREEN_MEDIA_SERVICE_H
#define CV_SCREEN_MEDIA_SERVICE_H

#include <QtCore/QtCore>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>

#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>
#include <dbus/channel/type/screenMedia/common/ScreenMediaServiceCommon.h>

typedef unsigned char BYTE;

class ScreenSrc;
class ScreenSnapping;
class ChannelDispatcherIf;

class CvScreenMediaService:public QObject
{
	friend class ScreenSrc;
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


private:
	
	int createRecvMedia( const ScreenMediaInfo& info );
	/**
	 * @brief ��_captureWindow����ʾ���յ���ͼƬ��
	 */
	void displayScreen();
	
	MediaState				_currentState;
	MediaDirection			_mediaDirection;
	QString					_mediaID;
	QRect					_captureWindow;		//��Ҫ���н����Ĵ���
	QString					_ip;				//��Ļ���ļ���IP
	int						_port;				//��Ļ��ռ�õĶ˿�
	ScreenSrc*				_screenSrc;
	///��Ž��յ���ͼƬ����
	BYTE*					_imageBuffer;			//ͼƬ������
	unsigned int			_imageBufferMaxLength;	//ͼƬ�������Ĵ�С
	unsigned int			_imageLength;			//ʵ��ͼƬ�ĳ���


	//������ʾ��ͼƬ����
	QString					_displayImageFileName;

	//ChannelDispatcher proxy
	ChannelDispatcherIf*	_channelDispatcherProxy;

	///ͬ������
	QMutex					_mutext;
};


#endif
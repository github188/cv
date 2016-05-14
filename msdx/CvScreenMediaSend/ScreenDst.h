#ifndef SCREEN_DST_H
#define SCREEN_DST_H
#include <QtCore/QtCore>
#include <QtNetwork/QTcpSocket>
///��Ļ���ķ���Ŀ��
class ScreenDist : public QObject
{
	Q_OBJECT
public:
	ScreenDist( QString ip , int port);
	~ScreenDist();

	void resetDst( const QString&ip ,const int port );

	//�ͷ�����
	void release();

private Q_SLOTS:
	void dealError(QAbstractSocket::SocketError error);

public:
	QString		_ip;			//Ŀ��IP
	int			_port;			//Ŀ��˿�
	bool		_isBusy;		//�Ƿ�����ʹ��
	QTcpSocket	_socket;		//ͨ���õ�socket
};

#endif